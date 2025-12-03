project "Coral.Managed"
    filter { "not action:vs*", "not system:windows" }
        kind "StaticLib"
		-- Mach-y AR requires a non-empty file list for archive creation
		files { "Source/Dummy.cpp" }

    filter { "action:vs* or system:windows" }
        language "C#"
        dotnetframework "net9.0"
        kind "SharedLib"
        clr "Unsafe"
        targetdir("../Build/%{cfg.buildcfg}")
        objdir("../Intermediates/%{cfg.buildcfg}")
        dependson { "Coral.Generator" }

		--nuget {
		--	"Microsoft.Build:17.12.6",
		--	"Microsoft.Build.Framework:17.12.6",
		--	"Microsoft.Build.Runtime:17.12.6",
		--	"Microsoft.Build.Utilities.Core:17.12.6",
		--	"Microsoft.Build.Tasks.Core:17.12.6"
		--}

		-- Don't specify architecture here. (see https://github.com/premake/premake-core/issues/1758)

		vsprops {
			AppendTargetFrameworkToOutputPath = "false",
			Nullable = "enable",
			CopyLocalLockFileAssemblies = "true",
			EnableDynamicLoading = "true",
		}

        disablewarnings {
            "CS8500"
        }

        files {
            "Source/**.cs"
        }
