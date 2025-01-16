include "../../Premake/DebuggerTypeExtension.lua"

project "Testing.Native"
    language "C++"
    cppdialect "C++20"
    kind "ConsoleApp"
    staticruntime "Off"
    debuggertype "NativeWithManagedCore"
    architecture "x86_64"

    files {
        "Source/**.cpp",
        "Source/**.hpp",
    }

    externalincludedirs { "../../Coral.Native/Include/" }

	links { "Coral.Native", }

    filter { "configurations:Debug" }
        runtime "Debug"
		symbols "On"
        defines { "CORAL_TESTING_DEBUG" }
	filter {}

    filter { "configurations:Release" }
        runtime "Release"
		symbols "Off"
        defines { "CORAL_TESTING_RELEASE" }
	filter {}
