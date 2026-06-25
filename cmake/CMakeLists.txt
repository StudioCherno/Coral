cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

project(Coral)

option(CORAL_EXAMPLE "Build the example programs" OFF)
option(CORAL_TESTING "Build the tests" ON)

include(ExternalProject)

include(${CMAKE_CURRENT_LIST_DIR}/Dotnet.cmake)
find_program(DOTNET NAMES dotnet REQUIRED)

set(CORAL_ROOT ${CMAKE_CURRENT_LIST_DIR}/..)

# Coral.Managed: the managed (.NET) half of Coral. On Visual Studio we add the .csproj to the
# solution as-is via include_external_msproject so VS shows it as a real C# project (sources,
# IntelliSense, NuGet). PLATFORM "Any CPU" maps the solution's x64 config onto the project, which
# VS otherwise refuses to build. Everywhere else there's no native C# support, so we just run
# dotnet build. Both branches define the Coral.Managed target, so dependencies behave the same.
set(CORAL_MANAGED_CSPROJ "${CORAL_ROOT}/Coral.Managed/Coral.Managed-Static.csproj")
if(CMAKE_GENERATOR MATCHES "Visual Studio")
	include_external_msproject(Coral.Managed "${CORAL_MANAGED_CSPROJ}" PLATFORM "Any CPU")
else()
	add_custom_target(Coral.Managed ALL
		COMMAND ${DOTNET} build "${CORAL_MANAGED_CSPROJ}"
			--configuration $<CONFIG>
			--output "${CORAL_ROOT}/Build/$<CONFIG>"
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMENT "Building Coral managed code"
		VERBATIM)
endif()

add_library(Coral.Native STATIC)

LinkDotnetNetHost(${DOTNET} Coral.Native)
add_dependencies(Coral.Native Coral.Managed)

set_target_properties(Coral.Native PROPERTIES LINKER_LANGUAGE CXX)
target_compile_features(Coral.Native PUBLIC cxx_std_17)

target_precompile_headers(Coral.Native PRIVATE ${CORAL_ROOT}/Coral.Native/Source/CoralPCH.hpp)

file(GLOB CORAL_HDR ${CORAL_ROOT}/Coral.Native/Include/Coral/*.hpp)
target_sources(Coral.Native PUBLIC ${CORAL_HDR})

file(GLOB CORAL_SRC ${CORAL_ROOT}/Coral.Native/Source/Coral/*.cpp ${CORAL_ROOT}/Coral.Native/Source/Coral/*.hpp)
target_sources(Coral.Native PRIVATE ${CORAL_SRC})

target_include_directories(Coral.Native PUBLIC ${CORAL_ROOT}/Coral.Native/Include)
target_include_directories(Coral.Native PRIVATE ${CORAL_ROOT}/Coral.Native/Source)

if(CORAL_EXAMPLE)
	set(EXAMPLE_ROOT ${CORAL_ROOT}/Example)
	set(EXAMPLE_BINDIR ${CMAKE_CURRENT_BINARY_DIR}/Example)

	file(GLOB EXAMPLE_MANAGED_SRC CONFIGURE_DEPENDS ${EXAMPLE_ROOT}/Example.Managed/Source/*.cs)
	add_custom_command(
		OUTPUT ${EXAMPLE_BINDIR}/Example.Managed.dll ${EXAMPLE_BINDIR}/Example.Managed.deps.json ${EXAMPLE_BINDIR}/Example.Managed.runtimeconfig.json
		DEPENDS ${EXAMPLE_MANAGED_SRC} ${EXAMPLE_ROOT}/Example.Managed/Example.Managed-Static.csproj Coral.Managed
		WORKING_DIRECTORY ${EXAMPLE_ROOT}
		COMMENT "Building example managed code"
		VERBATIM
		COMMAND ${DOTNET} build ${EXAMPLE_ROOT}/Example.Managed/Example.Managed-Static.csproj
				--artifacts-path ${EXAMPLE_BINDIR}
				--output ${EXAMPLE_BINDIR}
				--self-contained $<IF:$<CONFIG:Debug>,true,false>
	)
	add_custom_target(ExampleManaged DEPENDS ${EXAMPLE_BINDIR}/Example.Managed.dll)

	add_executable(Example.Native)
	set_target_properties(Example.Native PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${EXAMPLE_BINDIR})

	add_dependencies(Example.Native ExampleManaged)

	target_compile_features(Example.Native PUBLIC cxx_std_17)
	target_link_libraries(Example.Native Coral.Native)

	file(GLOB EXAMPLE_SRC ${CORAL_ROOT}/Example/Example.Native/Source/*.cpp)
	target_sources(Example.Native PRIVATE ${EXAMPLE_SRC})
endif()

if(CORAL_TESTING)
	set(TESTING_ROOT ${CORAL_ROOT}/Tests)
	set(TESTING_BINDIR ${CMAKE_CURRENT_BINARY_DIR}/Tests)

	file(GLOB TESTING_MANAGED_SRC CONFIGURE_DEPENDS ${TESTING_ROOT}/Testing.Managed/Source/*.cs)
	add_custom_command(
		OUTPUT ${TESTING_BINDIR}/Testing.Managed.dll ${TESTING_BINDIR}/Testing.Managed.deps.json ${TESTING_BINDIR}/Testing.Managed.runtimeconfig.json
		DEPENDS ${TESTING_MANAGED_SRC} ${TESTING_ROOT}/Testing.Managed/Testing.Managed-Static.csproj Coral.Managed
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMENT "Building testing managed code"
		VERBATIM
		COMMAND ${DOTNET} build ${TESTING_ROOT}/Testing.Managed/Testing.Managed-Static.csproj
				--artifacts-path ${TESTING_BINDIR}
				--output ${TESTING_BINDIR}
				--self-contained $<IF:$<CONFIG:Debug>,true,false>
	)
	add_custom_target(TestingManaged DEPENDS ${TESTING_BINDIR}/Testing.Managed.dll)

	add_executable(Testing.Native)
	set_target_properties(Testing.Native PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TESTING_BINDIR})

	add_dependencies(Testing.Native TestingManaged)

	target_compile_features(Testing.Native PUBLIC cxx_std_17)
	target_link_libraries(Testing.Native Coral.Native)

	file(GLOB TESTING_SRC ${TESTING_ROOT}/Testing.Native/Source/*.cpp)
	target_sources(Testing.Native PRIVATE ${TESTING_SRC})
endif()