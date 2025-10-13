#!/bin/zsh

cd ../../

premake5 gmake2 --cc=clang --file=premake5-native.lua
premake5 vs2022 --file=premake5-managed.lua
make -j$(($(sysctl -n hw.logicalcpu) - 1))
dotnet build CoralManaged.sln

cd ./Scripts/macOS/
