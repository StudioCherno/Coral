include "../Premake/CSExtensions.lua"

project "Coral.Managed"
    language "C#"
    dotnetframework "net7.0"
    kind "SharedLib"
	clr "Unsafe"
	targetdir("Build/%{cfg.buildcfg}-%{cfg.system}")
	objdir("Intermediates/%{cfg.buildcfg}-%{cfg.system}")

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
