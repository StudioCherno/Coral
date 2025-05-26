cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(Coral)

include(ExternalProject)

get_filename_component(CORAL_ROOT ${CMAKE_CURRENT_LIST_DIR}/.. REALPATH)
set(CORAL_LIB_ROOT ${CORAL_ROOT}/Coral.Native)

ExternalProject_Add(
		CoralManaged
		CONFIGURE_COMMAND ""
		INSTALL_COMMAND ""
		SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}
		BUILD_ALWAYS true
		BUILD_COMMAND
			dotnet build ${CMAKE_CURRENT_LIST_DIR}/../Coral.Managed/Coral.Managed-Static.csproj
			--artifacts-path ${CMAKE_CURRENT_BINARY_DIR}
			--output ${CMAKE_CURRENT_BINARY_DIR}
			--self-contained $<IF:$<CONFIG:Debug>,true,false>)

add_library(Coral STATIC)

add_dependencies(Coral CoralManaged)

set_target_properties(Coral PROPERTIES LINKER_LANGUAGE CXX)
target_compile_features(Coral PUBLIC cxx_std_20)

target_precompile_headers(Coral PRIVATE ${CORAL_LIB_ROOT}/Source/CoralPCH.hpp)

file(GLOB CORAL_HDR ${CORAL_LIB_ROOT}/Include/Coral/*.hpp)
target_sources(Coral PUBLIC ${CORAL_HDR})

file(GLOB CORAL_SRC ${CORAL_LIB_ROOT}/Source/Coral/*.cpp ${CORAL_LIB_ROOT}/Source/Coral/*.hpp)
target_sources(Coral PRIVATE ${CORAL_SRC})

target_include_directories(Coral PUBLIC ${CORAL_LIB_ROOT}/Include)
target_include_directories(Coral PRIVATE ${CORAL_LIB_ROOT}/Source)
target_include_directories(Coral PRIVATE ${CORAL_ROOT}/NetCore)
