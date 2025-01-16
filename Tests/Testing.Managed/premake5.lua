project "Testing.Managed"
    language "C#"
    dotnetframework "net8.0"
    kind "SharedLib"
	clr "Unsafe"

	nuget { "Newtonsoft.Json:13.0.3" }
	
    -- Don't specify architecture here. (see https://github.com/premake/premake-core/issues/1758)

	vsprops {
		AppendTargetFrameworkToOutputPath = "false",
		Nullable = "enable",
		CopyLocalLockFileAssemblies = "true",
		EnableDynamicLoading = "true",
	}

    files {
        "Source/**.cs"
    }
    
    links { "Coral.Managed" }
