local CoralDotNetPath = os.getenv("CORAL_DOTNET_PATH")

project "Coral.Native"
    language "C++"
    cppdialect "C++20"
    kind "StaticLib"

    -- Can't specify 64-bit architecture in the workspace level since VS 2022 (see https://github.com/premake/premake-core/issues/1758)
    architecture "x86_64"

    pchheader "CoralPCH.hpp"
    pchsource "Source/CoralPCH.cpp"

    forceincludes { "CoralPCH.hpp" }

    files {
        "Source/**.cpp",
        "Source/**.hpp",

        "Include/Coral/**.hpp",
    }

    includedirs { "Source/", "Include/Coral/" }
    externalincludedirs { CoralDotNetPath }
