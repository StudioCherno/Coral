workspace "CoralManaged"
    configurations { "Debug", "Release" }

    targetdir "%{wks.location}/Build/%{cfg.buildcfg}"
	objdir "%{wks.location}/Intermediates/%{cfg.buildcfg}"

include "Coral.Managed"

group "Tests"
	include "Tests/Testing.Managed"
group ""

group "Example"
	include "Example/Example.Managed"
group ""
