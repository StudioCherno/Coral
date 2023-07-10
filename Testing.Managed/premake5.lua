include "../Premake/CSExtensions.lua"

project "Testing.Managed"
    language "C#"
    dotnetframework "net7.0"
    kind "SharedLib"
	clr "Unsafe"
	
    -- Don't specify architecture here. (see https://github.com/premake/premake-core/issues/1758)

    propertytags { { "AppendTargetFrameworkToOutputPath", "false" } }

    files {
        "Source/**.cs"
    }
    
    links { "Coral.Managed" }
