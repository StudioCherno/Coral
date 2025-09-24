project "Example.Managed"
    language "C#"
    dotnetframework "net9.0"
    kind "SharedLib"
	clr "Unsafe"
	
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
