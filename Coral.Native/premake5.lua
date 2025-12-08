include "../Premake/DebuggerTypeExtension.lua"

-- Thanks to https://stackoverflow.com/a/326715
function captureExec(cmd, raw)
    local f = assert(io.popen(cmd, 'r'))
    local s = assert(f:read('*a'))
    f:close()
    return s
end

function LinkNethost()
    local runtimes = captureExec('dotnet --list-runtimes')
    local versions = {}
    -- Thanks to https://gist.github.com/iwanbk/5479582
    for runtime in runtimes:gmatch("([^\n]*)\n?") do
        local version = string.match(runtime, "Microsoft%.NETCore%.App ([0-9]+%.[0-9]+%.[0-9]+)")
        table.insert(versions, version)
    end

    local sdks = captureExec('dotnet --list-sdks')
    for sdk in sdks:gmatch("([^\n]*)\n?") do
        if string.sub(sdk, 1, 1) == "9" then
            local arch_name = ""
            if os.target() == "macosx" then
                arch_name = "osx-"
            else
                arch_name = "arch-"
            end

            if os.hostarch() == "x86_64" then
                arch_name = arch_name .. "x64"
            elseif os.hostarch() == "ARM64" then
                arch_name = arch_name .. "arm64"
            end

            local base_path = string.match(sdk, "%[(.*)%]")
            base_path = base_path .. "/../packs/Microsoft.NETCore.App.Host." .. arch_name .. "/"

            local found = false
            for index, version in pairs(versions) do
                local base_path_ver = base_path .. version .. "/"
                if os.isdir(base_path_ver) then
                    base_path = base_path_ver .. "/runtimes/" .. arch_name .. "/native/"
                    base_path = os.realpath(base_path) .. "/"
                    print("Found .NET SDK path " .. base_path)
                    found = true
                end
            end
            if not found then break end

            externalincludedirs { base_path }
            libdirs { base_path }

            filter { "toolset:not msc" }
                linkoptions { base_path .. "libnethost.a" }
            filter { "toolset:msc" }
                links { "nethost" }
            filter {}
            return
        end
    end

    print("Failed to find .NET 9 SDK!")
    os.exit(1)
end

project "Coral.Native"
    language "C++"
    cppdialect "C++17"
    kind "StaticLib"
    staticruntime "Off"
    debuggertype "NativeWithManagedCore"

	dependson "Coral.Managed"

	targetdir("../Build/%{cfg.buildcfg}")
	objdir("../Intermediates/%{cfg.buildcfg}")

    pchheader "CoralPCH.hpp"
    pchsource "Source/CoralPCH.cpp"

    forceincludes { "CoralPCH.hpp" }

    filter { "action:xcode4" }
        pchheader "Source/CoralPCH.hpp"
    filter { }

    files {
        "Source/**.cpp",
        "Source/**.hpp",

        "Include/Coral/**.hpp",
    }

    includedirs { "Source/", "Include/" }

    LinkNethost()

    filter { "configurations:Debug" }
        runtime "Debug"
        symbols "On"
    filter { }

    filter { "configurations:Release" }
        runtime "Release"
        symbols "Off"
        optimize "On"
    filter { }

	filter { "system:windows" }
		defines { "CORAL_WINDOWS" }
    filter { }

	filter { "system:macosx" }
		defines { "CORAL_MACOSX" }
    filter { }
