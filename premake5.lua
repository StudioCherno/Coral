workspace "Coral"
    configurations { "Debug", "Release" }

    targetdir "%{wks.location}/Build/%{cfg.buildcfg}"
	objdir "%{wks.location}/Intermediates/%{cfg.buildcfg}"

    startproject "Testing"

include "Coral.Native"
include "Coral.Managed"
include "Testing"
include "Testing.Managed"