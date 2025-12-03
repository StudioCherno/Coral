include "../../Premake/DebuggerTypeExtension.lua"

project "Example.Native"
    language "C++"
    cppdialect "C++17"
    kind "ConsoleApp"
    staticruntime "Off"
    debuggertype "NativeWithManagedCore"

    files {
        "Source/**.cpp",
        "Source/**.hpp",
    }

    externalincludedirs { "../../Coral.Native/Include/" }

	links { "Coral.Native" }

    filter { "configurations:Debug" }
        runtime "Debug"
		symbols "On"
        defines { "CORAL_EXAMPLE_DEBUG" }
	filter {}

    filter { "configurations:Release" }
        runtime "Release"
		symbols "Off"
        defines { "CORAL_EXAMPLE_RELEASE" }
	filter {}
