# Fork patches

Changes this fork makes to upstream Coral (StudioCherno/Coral), and why, so each can be carried
forward when the fork is updated, or dropped once upstream has an equivalent. The record starts
with the changes of 2026-09-14; it is not a complete inventory of earlier ones.

## Memory-loaded assemblies (2026-09-14)

`Coral.Native/Source/Coral/Assembly.cpp`: `AssemblyLoadContext::LoadAssemblyFromMemory` matches
file loading for owner context and assembly-local type lookup. It sets the owner context ID,
reserves the owned local type vector before publishing its element pointers, and populates both
local name and ID maps. Each local entry retains its actual managed type ID; names are not resolved
through the process-wide cache.

Without this, a byte-loaded assembly reports a successful load and populates the legacy global
list, while `ManagedAssembly::GetLocalType` always returns null. Local lookup is what keeps two live
load contexts from selecting each other's classes by name, and later internal-call registration
uses the owner context. Reapply this when updating the fork unless upstream supplies equivalent
local ownership for both loading paths.

## Collectible context cache isolation (2026-09-14)

`Coral.Managed/Source/ManagedObject.cs` keys invocation lookup by the actual managed `Type`,
binding flags, method name and complete native parameter signature. Same-named classes from
different assemblies or load contexts cannot share a `MethodInfo`. Method equality checks arity
before indexing parameter arrays.

`AssemblyLoader.cs`, `TypeInterface.cs` and `ReflectionCacheContext.cs` remove only cache entries
that reference the quiesced, unloading context. All reflection categories are included, including
constructors and events. Constructed framework generics, arrays/pointers/function pointers and
reflected member signatures are traced through their component types. Cached attributes retain
their declaring member as ownership information: a framework attribute can still belong to a
collectible context's assembly. Debug `GCHandle` tracking uses actual assembly identity instead of
an assembly-name hash, so unloading a peer cannot free the surviving context's object handles.
These are owner-controlled unload semantics; unloading concurrently with active calls into that
same context remains unsupported.

`UniqueList.cs` assigns monotonic positive native handles to reference-identity entries (and their
optional declaration owners), with synchronized insertion/removal and no reuse after removal or
clear. Runtime object hash codes are dictionary hashes only, never native identity. Exhausting the
signed 32-bit handle space raises a managed error instead of wrapping. Native return-value
templates in `Type.hpp`, `ManagedObject.hpp` and `Attribute.hpp` initialize their storage so a
refused managed lookup does not expose uninitialized native bytes. A default value is not an
invocation-success signal: failures are reported through the managed exception and log channel.

The CMake build globs managed sources; a Premake build must regenerate its project after a source
such as `ReflectionCacheContext.cs` is added. No generated project file is a maintained patch
target.

## .NET discovery (2026-11-05)

`cmake/Dotnet.cmake` requires the .NET major in `CORAL_DOTNET_MAJOR_VERSION` (currently 10, the
`TargetFramework` of `Coral.Managed/*.csproj`) or newer, and selects more strictly than upstream:
- **Runtime:** only `Microsoft.NETCore.App` lines count. Upstream took the first listed runtime of
  any framework, usually ASP.NET Core.
- **Host pack:** the newest pack of a supported major in the selected SDK's dotnet root, skipping
  musl packs. Upstream required a pack whose version equalled that runtime's, which fails when a
  runtime is newer than the SDK's own pack.
- **nethost:** a cached `NETHOST` from another pack, such as the previous major's after a move to a
  new one, is looked up again in the selected pack, so the library matches its headers. Upstream's
  `find_library` kept the cached one.

## Coral.Managed built with dotnet build everywhere (2026-11-11)

`cmake/CMakeLists.txt` builds `Coral.Managed` with `dotnet build` on every generator. Upstream added
the `.csproj` to Visual Studio solutions with `include_external_msproject`, which Visual Studio
2022's MSBuild 17.14 builds with the newest SDK it can load; the .NET 10 SDK needs MSBuild 18, so
that fell back to .NET 9's and failed to target `net10.0`.

## Default load context entry points (2026-11-05)

`HostInstance::LoadAssemblyIntoDefaultContext(path)` and
`HostInstance::GetDefaultContextFunction(typeName, methodName)` wrap hostfxr's
`hdt_load_assembly` and `hdt_get_function_pointer` (with `UNMANAGEDCALLERSONLY_METHOD`), fetched
lazily from the same hostfxr context Coral initializes. Coral.Managed itself stays in hostfxr's
isolated component context, so an assembly loaded this way must not reference Coral.Managed's
types: it would resolve a second copy with a different identity. A host can use them to hand
native code a table of `[UnmanagedCallersOnly]` pointers instead of calling through Coral's
reflection invoke. Errors go through the message callback and return false or nullptr; both may
be called before `Initialize`, in which case they only fail.

## Shared load contexts and unload checks (2026-11-07)

`Coral.Managed/Source/AssemblyLoader.cs` gains shared contexts: non-collectible contexts whose
assemblies every other context resolves by simple name (`CreateSharedAssemblyLoadContext`, native
`HostInstance::CreateSharedAssemblyLoadContext`). `ResolveAssembly` returns a shared context's copy
right after the requesting context's own cache, before Coral's own context or any folder is
probed. A library that many collectible contexts reference, such as a host API that every script
context uses, is then loaded once, and every context sees the same types.

- A shared context is never unloaded: managed `UnloadContext` refuses it, and so does native
  `HostInstance::UnloadAssemblyLoadContext`, which leaves its assemblies listed.
- Creating one under a name that is already shared returns it, and loading an assembly it already
  holds returns that copy, so a later `HostInstance` in the same process gets the same context
  back.
- `UnloadContext` keeps a `WeakReference` to each unloaded context. `QueryContextCollected` (native
  `HostInstance::IsAssemblyLoadContextCollected`) reads it after a collection, so a host can tell
  whether something still holds an unloaded context's assemblies.
  `AssemblyLoadContext::GetContextId` gives native code the id to ask with. Ids are name hashes, so
  creating a context forgets an unloaded one of the same name.
