# Coral

Coral is a C++ wrapper around the .NET HostFXR library, the purpose of Coral is to provide an interface similar to [Mono](https://www.mono-project.com/) when it comes to C++/C# interop, but in a more modern style, and using .NET Core instead of .NET Framework.

The goal of the API is to keep it as simple and flexible as possible, while remaning fast and (relatively) safe.

## Supported Platforms
* Windows x64 (VS2022)
* Linux x64 (Last tested on Ubuntu 22.04)

## Compiling
* Coral has been tested to compile with MSVC / CL (Visual Studio 2022) and Clang 16+
* It uses C++20 and depends on the .NET SDK being present on the system

### Building
Coral uses the [premake](https://premake.github.io/) meta-build system in order to generate build files for other build systems (e.g Visual Studio Soltuions, Makefiles, etc...)

You'll need to download premake from [https://premake.github.io/](https://premake.github.io/), after that open up a terminal and cd into the root directory of Coral, then run this command:

```
premake5 [action]
```

where action is one of the supported actions in premake: [https://premake.github.io/docs/Using-Premake#using-premake-to-generate-project-files](https://premake.github.io/docs/Using-Premake#using-premake-to-generate-project-files)

## License
Coral is licensed under the [MIT](./LICENSE) license
