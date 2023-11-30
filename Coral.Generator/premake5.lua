include "../Premake/CSExtensions.lua"

project "Coral.Generator"
    language "C#"
    dotnetframework "net8.0"
    kind "ConsoleApp"
	clr "Unsafe"
	targetdir("../Build/%{cfg.buildcfg}")
	objdir("../Intermediates/%{cfg.buildcfg}")

    -- Don't specify architecture here. (see https://github.com/premake/premake-core/issues/1758)

    propertytags {
        { "AppendTargetFrameworkToOutputPath", "false" },
        { "Nullable", "enable" },
    }

    disablewarnings {
        "CS8500"
    }

	nuget {
		"ICSharpCode.Decompiler:8.2.0.7535"
	}

    files {
        "Source/**.cs"
    }
