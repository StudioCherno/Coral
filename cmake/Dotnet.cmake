function(LinkDotnetNetHost DOTNET TARGET)
	execute_process(
		COMMAND ${DOTNET} --list-runtimes
		OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/dotnet-runtimes.txt
		COMMAND_ERROR_IS_FATAL ANY
	)

	file(STRINGS ${CMAKE_CURRENT_BINARY_DIR}/dotnet-runtimes.txt RUNTIMES)
	foreach(LINE ${RUNTIMES})
		string(REGEX MATCH "[0-9]+\.[0-9]+\.[0-9]+" VERSION "${LINE}")
		if(${VERSION} VERSION_GREATER "9.0.0")
			message(STATUS "Found .NET runtime version ${VERSION}")
			set(RUNTIME_VERSION ${VERSION})
			break()
		endif()
	endforeach()

	if(NOT RUNTIME_VERSION)
		message(FATAL_ERROR "No suitable .NET runtimes found, .NET 9+ is required.")
	endif()

	execute_process(
		COMMAND ${DOTNET} --list-sdks
		OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/dotnet-sdks.txt
		COMMAND_ERROR_IS_FATAL ANY
	)

	file(STRINGS ${CMAKE_CURRENT_BINARY_DIR}/dotnet-sdks.txt SDKS)
	foreach(LINE ${SDKS})
		string(REGEX MATCH "[0-9]+\.[0-9]+\.[0-9]+" SDK_VERSION "${LINE}")
		if(${SDK_VERSION} VERSION_GREATER "9.0.0")
			string(REGEX MATCH "[[](.+)[]]" _ "${LINE}")
			set(SDK_PATH "${CMAKE_MATCH_1}")
			break()
		endif()
	endforeach()

	if(NOT SDK_PATH)
		message(FATAL_ERROR "No suitable .NET SDKs found, .NET 9+ is required.")
	endif()

	message(STATUS "Using .NET SDK, ${SDK_VERSION} from ${SDK_PATH}")
	cmake_path(GET SDK_PATH PARENT_PATH SDK_PATH)
	file(GLOB PACKS "${SDK_PATH}/packs/Microsoft.NETCore.App.Host*")

	list(LENGTH PACKS PACKS_LENGTH)
	if(${PACKS_LENGTH})
		if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "AMD64" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
			set(DOTNET_ARCH x64)
		elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i386" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
			set(DOTNET_ARCH x86)
		elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "ARM64" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm64")
			set(DOTNET_ARCH arm64)
		else()
			message(FATAL_ERROR "Unable to determine appropriate nethost architecture")
		endif()

		file(GLOB PACKS "${SDK_PATH}/packs/Microsoft.NETCore.App.Host*-${DOTNET_ARCH}")
	endif()

	file(GLOB NATIVES "${PACKS}/${RUNTIME_VERSION}/runtimes/*")

	target_include_directories(${TARGET} PUBLIC ${NATIVES}/native)

	block()
		if(NOT WINDOWS)
			list(REMOVE_ITEM CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_SHARED_LIBRARY_SUFFIX})
		endif()
	
		find_library(
			NETHOST
			NAMES nethost
			PATHS ${NATIVES}/native
			NO_DEFAULT_PATH
			REQUIRED
		)

		target_link_libraries(${TARGET} PRIVATE ${NETHOST})
	endblock()
endfunction()
