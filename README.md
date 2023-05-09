# Coral
Coral is a C++ wrapper around the .NET CoreCLR library, the purpose of Coral is to provide an interface similar to Mono when it comes to C++/C# interop, but in a more modern style, and using .NET Core instead of .NET Framework.

The goal of the API is to keep it as simple and flexible as possible, while remaning fast and safe.

Currently the API is basically non-existent, but as I keep developing it I'll update the list below.

## Features
- [ ] Mono-like internal calls. Will either be implemented using [Marshal.GetDelegateForFunctionPointer](https://learn.microsoft.com/en-us/dotnet/api/system.runtime.interopservices.marshal.getdelegateforfunctionpointer?view=net-7.0) or the [calli](https://learn.microsoft.com/en-us/dotnet/api/system.reflection.emit.opcodes.calli?view=net-7.0) instruction
- [ ] Provide an easy to use system for unloading and reloading assemblies at runtime

This list isn't that extensive right now, but I'll keep updating it over time.

## Building
Coral uses the [premake](https://premake.github.io/) meta-build system in order to generate build files for other build systems (e.g Visual Studio Soltuions, Makefiles, etc...)

Before you can use premake on Coral you'll need to install the .NET SDK, and then tell Coral where it can find the HostFXR headers + libraries, this is done by setting an environment variable called `CORAL_DOTNET_PATH`.

On Windows this path should point to e.g `C:\Program Files\dotnet\packs\Microsoft.NETCore.App.Host.win-x64\7.0.4\runtimes\win-x64\native`.

You'll know that you've found the right directory when you see a file called `hostfxr.h` and a file called `nethost.h`.

You'll need to download premake from [https://premake.github.io/](https://premake.github.io/), after that open up a terminal and cd into the root directory of Coral, then run this command:
```
premake5 [action]
```
where action is one of the supported actions in premake: [https://premake.github.io/docs/Using-Premake#using-premake-to-generate-project-files](https://premake.github.io/docs/Using-Premake#using-premake-to-generate-project-files)

### Windows
For Windows I recommend building Coral with Visual Studio 2022, using either MSVC or clang-cl.

### Linux
For Linux I recommend using premakes `gmake2` generator to generate Makefiles, and then just running `make` in the root folder of Corals project.

## Supported Compilers
These are the compilers that I've tested Coral with:
- MSVC. Specifically the version that ships with version 17.5.3 of Visual Studio
- Clang

GCC currently isn't officially supported, but I'll make sure Coral compiles with GCC in the near future.

## Running the Testing project
Running the Testing project is realtively easy, you just have to make sure to copy the Coral.Managed.runtimeconfig.json file from the Coral.Managed folder to the build folder, e.g Build/Debug/Coral.Managed.runtimeconfig.json
