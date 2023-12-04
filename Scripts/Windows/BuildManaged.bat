@echo off
echo Building Coral.Managed (Configuration: %1)

cd ../../

echo:
echo [BUILD]: Running build step 1
premake5 vs2022 --file=premake5-managed.lua
dotnet build .\CoralManaged.sln --nologo -c %1 --property:WarningLevel=0

echo:
echo [BUILD]: Running source generator...
.\Build\%1\Coral.Generator.exe .\Build\%1\Coral.Managed.dll --cs-source-dir .\Coral.Managed\Source --cpp-source-dir .\Coral.Native\Source\Coral

echo:
echo [BUILD]: Running build step 2
premake5 vs2022 --file=premake5-managed.lua
premake5 vs2022 --file=premake5-native.lua
dotnet build .\CoralManaged.sln --nologo -c %1 --property:WarningLevel=0

cd .\Scripts\Windows\