cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(Coral)

option(CORAL_EXAMPLE "Build the example programs" OFF)
option(CORAL_TESTING "Build the tests" ON)

include(ExternalProject)

find_program(DOTNET_EXE NAMES dotnet)
if(NOT DOTNET_EXE)
	message(FATAL_ERROR "Failed to find .NET!")
endif()
execute_process(
	COMMAND ${DOTNET_EXE} --version
	OUTPUT_VARIABLE DOTNET_SDKVER
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)
if(NOT DOTNET_SDKVER)
	message(FATAL_ERROR "Failed to check .NET version!")
endif()
if(DOTNET_SDKVER VERSION_LESS "9.0.0")
    message(FATAL_ERROR "No suitable .NET SDK found, .NET 9+ is required.")
endif()
message(STATUS "Found a suitable .NET SDK, version ${DOTNET_SDKVER}")

execute_process(
	COMMAND ${DOTNET_EXE} --list-runtimes
	OUTPUT_VARIABLE DOTNET_RUNTIMES
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)
string(REGEX MATCHALL "Microsoft\\.NETCore\\.App 9\\.[0-9]+\\.[0-9]+" DOTNET9_RUNTIMES "${DOTNET_RUNTIMES}")
if(NOT DOTNET9_RUNTIMES)
	message(FATAL_ERROR "No suitable .NET runtime found, .NET 9+ is required.")
endif()
list(LENGTH DOTNET9_RUNTIMES DOTNET9_COUNT)
math(EXPR LAST_INDEX "${DOTNET9_COUNT} - 1")
list(GET DOTNET9_RUNTIMES ${LAST_INDEX} DOTNET_RUNTIMEVER_STR)
string(REPLACE "Microsoft.NETCore.App " "" DOTNET_RUNTIMEVER "${DOTNET_RUNTIMEVER_STR}")
message(STATUS "Found a suitable .NET runtime, version ${DOTNET_RUNTIMEVER}")

set(CORAL_ROOT ${CMAKE_CURRENT_LIST_DIR}/..)

file(GLOB CORAL_MANAGED_SRC CONFIGURE_DEPENDS ${CORAL_ROOT}/Coral.Managed/Source/*.cs)
add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Coral.Managed.dll ${CMAKE_CURRENT_BINARY_DIR}/Coral.Managed.deps.json ${CMAKE_CURRENT_BINARY_DIR}/Coral.Managed.runtimeconfig.json
	DEPENDS ${CORAL_MANAGED_SRC} ${CORAL_ROOT}/Coral.Managed/Coral.Managed-Static.csproj
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building Coral managed code"
    VERBATIM
	COMMAND dotnet build ${CORAL_ROOT}/Coral.Managed/Coral.Managed-Static.csproj
			--artifacts-path ${CMAKE_CURRENT_BINARY_DIR}
			--output ${CMAKE_CURRENT_BINARY_DIR}
			--self-contained $<IF:$<CONFIG:Debug>,true,false>
)
add_custom_target(CoralManaged DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Coral.Managed.dll)

add_library(Coral STATIC)

add_dependencies(Coral CoralManaged)

set_target_properties(Coral PROPERTIES LINKER_LANGUAGE CXX)
target_compile_features(Coral PUBLIC cxx_std_20)

target_precompile_headers(Coral PRIVATE ${CORAL_ROOT}/Coral.Native/Source/CoralPCH.hpp)

file(GLOB CORAL_HDR ${CORAL_ROOT}/Coral.Native/Include/Coral/*.hpp)
target_sources(Coral PUBLIC ${CORAL_HDR})

file(GLOB CORAL_SRC ${CORAL_ROOT}/Coral.Native/Source/Coral/*.cpp ${CORAL_ROOT}/Coral.Native/Source/Coral/*.hpp)
target_sources(Coral PRIVATE ${CORAL_SRC})

target_include_directories(Coral PUBLIC ${CORAL_ROOT}/Coral.Native/Include)
target_include_directories(Coral PRIVATE ${CORAL_ROOT}/Coral.Native/Source)
target_include_directories(Coral PRIVATE ${CORAL_ROOT}/NetCore)

if(CORAL_EXAMPLE)
	set(EXAMPLE_ROOT ${CORAL_ROOT}/Example)
	set(EXAMPLE_BINDIR ${CMAKE_CURRENT_BINARY_DIR}/Example)

	file(GLOB EXAMPLE_MANAGED_SRC CONFIGURE_DEPENDS ${EXAMPLE_ROOT}/Example.Managed/Source/*.cs)
	add_custom_command(
		OUTPUT ${EXAMPLE_BINDIR}/Example.Managed.dll ${EXAMPLE_BINDIR}/Example.Managed.deps.json ${EXAMPLE_BINDIR}/Example.Managed.runtimeconfig.json
		DEPENDS ${EXAMPLE_MANAGED_SRC} ${EXAMPLE_ROOT}/Example.Managed/Example.Managed-Static.csproj CoralManaged
		WORKING_DIRECTORY ${EXAMPLE_ROOT}
		COMMENT "Building example managed code"
		VERBATIM
		COMMAND dotnet build ${EXAMPLE_ROOT}/Example.Managed/Example.Managed-Static.csproj
				--artifacts-path ${EXAMPLE_BINDIR}
				--output ${EXAMPLE_BINDIR}
				--self-contained $<IF:$<CONFIG:Debug>,true,false>
	)
	add_custom_target(ExampleManaged DEPENDS ${EXAMPLE_BINDIR}/Example.Managed.dll)

	add_executable(Example)
	set_target_properties(Example PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${EXAMPLE_BINDIR})

	add_dependencies(Example ExampleManaged)

	target_compile_features(Example PUBLIC cxx_std_20)
	target_link_libraries(Example Coral)

	file(GLOB EXAMPLE_SRC ${CORAL_ROOT}/Example/Example.Native/Source/*.cpp)
	target_sources(Example PRIVATE ${EXAMPLE_SRC})
endif()
if(CORAL_TESTING)
	set(TESTING_ROOT ${CORAL_ROOT}/Tests)
	set(TESTING_BINDIR ${CMAKE_CURRENT_BINARY_DIR}/Tests)

	file(GLOB TESTING_MANAGED_SRC CONFIGURE_DEPENDS ${TESTING_ROOT}/Testing.Managed/Source/*.cs)
	add_custom_command(
		OUTPUT ${TESTING_BINDIR}/Testing.Managed.dll ${TESTING_BINDIR}/Testing.Managed.deps.json ${TESTING_BINDIR}/Testing.Managed.runtimeconfig.json
		DEPENDS ${TESTING_MANAGED_SRC} ${TESTING_ROOT}/Testing.Managed/Testing.Managed-Static.csproj CoralManaged
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMENT "Building testing managed code"
		VERBATIM
		COMMAND dotnet build ${TESTING_ROOT}/Testing.Managed/Testing.Managed-Static.csproj
				--artifacts-path ${TESTING_BINDIR}
				--output ${TESTING_BINDIR}
				--self-contained $<IF:$<CONFIG:Debug>,true,false>
	)
	add_custom_target(TestingManaged DEPENDS ${TESTING_BINDIR}/Testing.Managed.dll)

	add_executable(Testing)
	set_target_properties(Testing PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TESTING_BINDIR})

	add_dependencies(Testing TestingManaged)

	target_compile_features(Testing PUBLIC cxx_std_20)
	target_link_libraries(Testing Coral)

	file(GLOB TESTING_SRC ${TESTING_ROOT}/Testing.Native/Source/*.cpp)
	target_sources(Testing PRIVATE ${TESTING_SRC})
endif()