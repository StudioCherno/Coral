#!/bin/bash

cd ../../

premake5 gmake2 --file=premake5-native.lua --arch=x86_64
premake5 vs2022 --file=premake5-managed.lua
make -j$(($(nproc) - 1))
dotnet build CoralManaged.sln

cd ./Scripts/Linux/
