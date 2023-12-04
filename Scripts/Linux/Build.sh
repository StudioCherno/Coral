#!/bin/bash

cd ../../

premake5 gmake2 --file=premake5-native.lua
premake5 vs2022 --file=premake5-managed.lua
make -j$(($(nproc) - 1))
dotnet build CoralManaged.sln

cd ./Scripts/Linux/
