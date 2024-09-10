workspace "Coral"
    configurations { "Debug", "Release" }

    targetdir "%{wks.location}/Build/%{cfg.buildcfg}"
	objdir "%{wks.location}/Intermediates/%{cfg.buildcfg}"

    startproject "Example.Native"

	defines {
		"_CRT_SECURE_NO_WARNINGS"
	}

include "Coral.Native"

group "Tests"
	include "Tests/Testing.Native"
group ""

group "Example"
	include "Example/Example.Native"
group ""
