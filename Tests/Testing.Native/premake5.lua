local CoralDotNetPath = os.getenv("CORAL_DOTNET_PATH")

include "../../Premake/DebuggerTypeExtension.lua"

project "Testing.Native"
    language "C++"
    cppdialect "C++20"
    kind "ConsoleApp"
    staticruntime "Off"

    -- Can't specify 64-bit architecture in the workspace level since VS 2022 (see https://github.com/premake/premake-core/issues/1758)
    architecture "x86_64"

    debuggertype "NativeWithManagedCore"

    files {
        "Source/**.cpp",
        "Source/**.hpp",
    }

    externalincludedirs { "../../Coral.Native/Include/" }

    libdirs { "%{wks.location}/NetCore/7.0.7/" }

    links {
        "Coral.Native",

        "nethost",
        "libnethost",
    }

    postbuildcommands {
        '{ECHO} Copying "%{wks.location}/NetCore/7.0.7/nethost.dll" to "%{cfg.targetdir}"',
        '{COPYFILE} "%{wks.location}/NetCore/7.0.7/nethost.dll" "%{cfg.targetdir}"',
        '{COPYFILE} "%{wks.location}/Coral.Managed/Coral.Managed.runtimeconfig.json" "%{cfg.targetdir}"',
    }

    filter { "configurations:Debug" }
        runtime "Debug"
        defines { "CORAL_TESTING_DEBUG" }

    filter { "configurations:Release" }
        runtime "Release"
        defines { "CORAL_TESTING_RELEASE" }
