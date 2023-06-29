workspace "Coral"
    configurations { "Debug", "Release" }

    targetdir "%{wks.location}/Build/%{cfg.buildcfg}"
	objdir "%{wks.location}/Intermediates/%{cfg.buildcfg}"

	filter { "configurations:Debug" }
		runtime "Debug"
		symbols "Full"
		optimize "Off"

	filter { "configurations:Release" }
		runtime "Release"
		symbols "Off"
		optimize "Full"

include "Coral.Native"
include "Coral.Managed"
include "Testing"
include "Testing.Managed"