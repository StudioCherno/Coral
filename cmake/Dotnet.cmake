# Links TARGET against nethost from the host pack of the .NET SDK that DOTNET runs.
#
# Coral.Managed targets net${CORAL_DOTNET_MAJOR_VERSION} (Coral.Managed/*.csproj), so configuring needs a
# runtime and an SDK of that major or newer; a newer major runs Coral.Managed through RollForward=Major.
function(LinkDotnetNetHost DOTNET TARGET)
	# Keep in step with the TargetFramework of Coral.Managed/*.csproj.
	set(CORAL_DOTNET_MAJOR_VERSION 10)

	execute_process(
		COMMAND ${DOTNET} --list-runtimes
		OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/dotnet-runtimes.txt
		COMMAND_ERROR_IS_FATAL ANY
	)

	# The listing also has ASP.NET Core and Windows Desktop runtimes; only the core runtime counts.
	file(STRINGS ${CMAKE_CURRENT_BINARY_DIR}/dotnet-runtimes.txt RUNTIMES)
	foreach(LINE ${RUNTIMES})
		if(LINE MATCHES "^Microsoft\\.NETCore\\.App (([0-9]+)\\.[0-9]+\\.[0-9]+)")
			if(CMAKE_MATCH_2 GREATER_EQUAL CORAL_DOTNET_MAJOR_VERSION)
				set(RUNTIME_VERSION "${CMAKE_MATCH_1}")
				break()
			endif()
		endif()
	endforeach()

	if(NOT RUNTIME_VERSION)
		message(FATAL_ERROR "No suitable .NET runtime found: .NET ${CORAL_DOTNET_MAJOR_VERSION} or newer is required.")
	endif()
	message(STATUS "Found .NET runtime version ${RUNTIME_VERSION}")

	execute_process(
		COMMAND ${DOTNET} --list-sdks
		OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/dotnet-sdks.txt
		COMMAND_ERROR_IS_FATAL ANY
	)

	# Lines read "<version>[-prerelease] [<dotnet root>/sdk]".
	file(STRINGS ${CMAKE_CURRENT_BINARY_DIR}/dotnet-sdks.txt SDKS)
	foreach(LINE ${SDKS})
		if(LINE MATCHES "^(([0-9]+)\\.[0-9]+\\.[0-9]+)[^ ]* [[](.+)[]]")
			if(CMAKE_MATCH_2 GREATER_EQUAL CORAL_DOTNET_MAJOR_VERSION)
				set(SDK_VERSION "${CMAKE_MATCH_1}")
				set(SDK_PATH "${CMAKE_MATCH_3}")
				break()
			endif()
		endif()
	endforeach()

	if(NOT SDK_PATH)
		message(FATAL_ERROR "No suitable .NET SDK found: the .NET ${CORAL_DOTNET_MAJOR_VERSION} SDK or newer is required.")
	endif()

	message(STATUS "Using .NET SDK ${SDK_VERSION} from ${SDK_PATH}")
	cmake_path(GET SDK_PATH PARENT_PATH SDK_PATH)

	if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "AMD64" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
		set(DOTNET_ARCH x64)
	elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i386" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
		set(DOTNET_ARCH x86)
	elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "ARM64" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm64")
		set(DOTNET_ARCH arm64)
	else()
		message(FATAL_ERROR "Unable to determine appropriate nethost architecture")
	endif()

	# An SDK installs the host pack of the runtime it shipped with, whatever runtimes are installed
	# separately, so take the newest pack of a supported major instead of one matching RUNTIME_VERSION.
	# nethost only locates hostfxr, which then selects the runtime. musl packs are for targets this
	# project never builds.
	file(GLOB HOST_PACK_CANDIDATES LIST_DIRECTORIES true "${SDK_PATH}/packs/Microsoft.NETCore.App.Host*-${DOTNET_ARCH}/*")
	foreach(CANDIDATE ${HOST_PACK_CANDIDATES})
		if(NOT IS_DIRECTORY "${CANDIDATE}" OR CANDIDATE MATCHES "-musl-")
			continue()
		endif()

		get_filename_component(CANDIDATE_VERSION "${CANDIDATE}" NAME)
		if(CANDIDATE_VERSION MATCHES "^([0-9]+)\\.[0-9]+\\.[0-9]+")
			if(CMAKE_MATCH_1 GREATER_EQUAL CORAL_DOTNET_MAJOR_VERSION)
				if(NOT HOST_PACK_VERSION OR CANDIDATE_VERSION VERSION_GREATER HOST_PACK_VERSION)
					set(HOST_PACK_VERSION "${CANDIDATE_VERSION}")
					set(HOST_PACK_DIR "${CANDIDATE}")
				endif()
			endif()
		endif()
	endforeach()

	if(NOT HOST_PACK_DIR)
		message(FATAL_ERROR "The .NET ${CORAL_DOTNET_MAJOR_VERSION} host pack for ${DOTNET_ARCH} was not found under "
			"${SDK_PATH}/packs; reinstall the .NET SDK.")
	endif()
	message(STATUS "Using .NET host pack ${HOST_PACK_VERSION} from ${HOST_PACK_DIR}")

	file(GLOB NATIVES "${HOST_PACK_DIR}/runtimes/*")

	target_include_directories(${TARGET} PUBLIC ${NATIVES}/native)

	block()
		if(NOT WINDOWS)
			list(REMOVE_ITEM CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_SHARED_LIBRARY_SUFFIX})
		endif()

		# find_library keeps a cached result, so a nethost from another pack (the previous major's,
		# once the SDK has moved on) is looked up again, to match the headers above.
		if(NETHOST)
			file(TO_CMAKE_PATH "${HOST_PACK_DIR}/" PACK_PREFIX)
			file(TO_CMAKE_PATH "${NETHOST}" CACHED_NETHOST)
			string(FIND "${CACHED_NETHOST}" "${PACK_PREFIX}" PACK_AT)
			if(NOT PACK_AT EQUAL 0)
				message(STATUS "Replacing nethost ${NETHOST}, which isn't from the selected host pack")
				unset(NETHOST CACHE)
			endif()
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
