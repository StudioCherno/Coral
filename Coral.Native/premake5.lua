include "../Premake/DebuggerTypeExtension.lua"

project "Coral.Native"
    language "C++"
    cppdialect "C++20"
    kind "StaticLib"
    staticruntime "Off"
    debuggertype "NativeWithManagedCore"
    architecture "x86_64"

	dependson "Coral.Managed"

	targetdir("../Build/%{cfg.buildcfg}")
	objdir("../Intermediates/%{cfg.buildcfg}")

    pchheader "CoralPCH.hpp"
    pchsource "Source/CoralPCH.cpp"

    forceincludes { "CoralPCH.hpp" }

    filter { "action:xcode4" }
        pchheader "Source/CoralPCH.hpp"
    filter { }

    files {
        "Source/**.cpp",
        "Source/**.hpp",

        "Include/Coral/**.hpp",
    }

    includedirs { "Source/", "Include/" }
    externalincludedirs { "../NetCore/" }

    filter { "configurations:Debug" }
        runtime "Debug"
        symbols "On"
    filter { }

    filter { "configurations:Release" }
        runtime "Release"
        symbols "Off"
        optimize "On"
    filter { }

	filter { "system:windows" }
		defines { "CORAL_WINDOWS" }
    filter { }

	filter { "system:macosx" }
		defines { "CORAL_MACOSX" }
    filter { }
