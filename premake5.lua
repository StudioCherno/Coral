workspace "Coral"
    configurations { "Debug", "Release" }

    targetdir "%{wks.location}/Build/%{cfg.buildcfg}"
	objdir "%{wks.location}/Intermediates/%{cfg.buildcfg}"

    startproject "Example.Native"

	defines {
		"_CRT_SECURE_NO_WARNINGS"
	}

include "Coral.Native"
include "Coral.Managed"

group "Tests"
	include "Tests/Testing.Native"
	include "Tests/Testing.Managed"
group ""

group "Example"
	include "Example/Example.Native"
	include "Example/Example.Managed"
group ""
