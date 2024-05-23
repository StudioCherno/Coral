include "../Premake/CSExtensions.lua"

project "Coral.Managed"
    filter { "not action:vs*", "not system:windows" }
        kind "StaticLib"
    filter { "action:vs* or system:windows" }
        language "C#"
        dotnetframework "net8.0"
        kind "SharedLib"
        clr "Unsafe"
        targetdir("../Build/%{cfg.buildcfg}")
        objdir("../Intermediates/%{cfg.buildcfg}")
        dependson { "Coral.Generator" }

        -- Don't specify architecture here. (see https://github.com/premake/premake-core/issues/1758)

        propertytags {
            { "AppendTargetFrameworkToOutputPath", "false" },
            { "Nullable", "enable" },
        }

        disablewarnings {
            "CS8500"
        }

        files {
            "Source/**.cs"
        }
