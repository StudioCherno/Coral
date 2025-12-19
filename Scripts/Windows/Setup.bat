@echo off

cd ..\..\
premake5 vs2022 --file=premake5-native.lua --arch=x86_64
premake5 vs2022 --file=premake5-managed.lua
cd .\Scripts\Windows\
