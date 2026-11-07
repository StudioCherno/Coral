using Coral.Managed.Interop;

using System;
using System.Collections.Generic;
using System.IO;
using System.IO.MemoryMappedFiles;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Loader;

namespace Coral.Managed;

using static ManagedHost;

public enum AssemblyLoadStatus
{
	Success, FileNotFound, FileLoadFailure, InvalidFilePath, InvalidAssembly, UnknownError
}

public static class AssemblyLoader
{
	// NOTE(Emily): Visible to `TypeInterface.cs`.
	public static readonly Dictionary<int, AssemblyLoadContext?> s_AssemblyContexts = new();
	private static Dictionary<int, string[]> s_AlcDllPaths = new();

	private static readonly Dictionary<Type, AssemblyLoadStatus> s_AssemblyLoadErrorLookup = new();
	private static readonly Dictionary<int, Dictionary<int, Assembly>> s_AssemblyCache = new();
#if DEBUG
	private static readonly Dictionary<Assembly, List<GCHandle>> s_AllocatedHandles = new(ReferenceEqualityComparer.Instance);
#endif
	private static AssemblyLoadStatus s_LastLoadStatus = AssemblyLoadStatus.Success;

	private static readonly int CORAL_ALC_CACHE_ID = -1;
	private static readonly AssemblyLoadContext? s_CoralAssemblyLoadContext;

	// Non-collectible contexts whose assemblies every other context shares, such as a host API
	// library that every script context references. Resolved by name before a context probes its
	// own folders, so each is loaded once however many contexts reference it. Never unloaded.
	private static readonly List<AssemblyLoadContext> s_SharedContexts = new();
	private static readonly HashSet<int> s_SharedContextIds = new();
	// Each unloaded context, weakly, so a host can check it was freed. Keyed by context id, the
	// name's hash: creating a context of the same name forgets the entry.
	private static readonly Dictionary<int, WeakReference> s_UnloadedContexts = new();

	static AssemblyLoader()
	{
		s_AssemblyLoadErrorLookup.Add(typeof(BadImageFormatException), AssemblyLoadStatus.InvalidAssembly);
		s_AssemblyLoadErrorLookup.Add(typeof(FileNotFoundException), AssemblyLoadStatus.FileNotFound);
		s_AssemblyLoadErrorLookup.Add(typeof(FileLoadException), AssemblyLoadStatus.FileLoadFailure);
		s_AssemblyLoadErrorLookup.Add(typeof(ArgumentNullException), AssemblyLoadStatus.InvalidFilePath);
		s_AssemblyLoadErrorLookup.Add(typeof(ArgumentException), AssemblyLoadStatus.InvalidFilePath);

		s_CoralAssemblyLoadContext = AssemblyLoadContext.GetLoadContext(typeof(AssemblyLoader).Assembly);
		s_CoralAssemblyLoadContext!.Resolving += ResolveAssembly;

		s_AssemblyCache.Add(CORAL_ALC_CACHE_ID, new());

		CacheCoralAssemblies();
	}

	private static void CacheCoralAssemblies()
	{
		foreach (var assembly in s_CoralAssemblyLoadContext!.Assemblies)
		{
			int assemblyId = assembly.GetName().Name!.GetHashCode();
			s_AssemblyCache[CORAL_ALC_CACHE_ID].Add(assemblyId, assembly);
		}
	}

	internal static bool TryGetAssembly(int InAssemblyLoadContextId, int InAssemblyId, out Assembly? OutAssembly)
	{
		return s_AssemblyCache[InAssemblyLoadContextId].TryGetValue(InAssemblyId, out OutAssembly);
	}

	internal static Assembly? ResolveAssembly(AssemblyLoadContext? InAssemblyLoadContext, AssemblyName InAssemblyName)
	{
		try
		{
			if (InAssemblyName.Name == null) throw new ArgumentNullException("InAssemblyName");

			int assemblyId = InAssemblyName.Name.GetHashCode();

			if (InAssemblyLoadContext == null || InAssemblyLoadContext.Name == null)
			{
				// Search all the assemblies!
				// TODO(Emily): Mark all the non-ALC-specific APIs as deprecated.
				LogMessage($"[AssemblyLoader] Global ALC cache behaviour is deprecated", MessageLevel.Warning);

				foreach (var cache in s_AssemblyCache)
				{
					foreach (KeyValuePair<int, Assembly> entry in cache.Value)
					{
						if (InAssemblyName.Name == entry.Value.GetName().Name)
						{
							return entry.Value;
						}
					}
				}

				LogMessage($"[AssemblyLoader] Failed to resolve assembly {InAssemblyName.FullName} against global assembly cache", MessageLevel.Trace);
				return null;
			}

			int alcId = InAssemblyLoadContext.Name.GetHashCode();

			if (s_AssemblyCache[alcId].TryGetValue(assemblyId, out var cachedAssembly))
			{
				return cachedAssembly;
			}

			// A shared context's copy before anything is probed, so there is only ever one.
			foreach (var shared in s_SharedContexts)
			{
				foreach (var assembly in shared.Assemblies)
				{
					if (assembly.GetName().Name == InAssemblyName.Name)
						return assembly;
				}
			}

			if (s_CoralAssemblyLoadContext != null)
			{
				foreach (var assembly in s_CoralAssemblyLoadContext.Assemblies)
				{
					if (assembly.GetName().Name != InAssemblyName.Name)
						continue;

					// NOTE(Emily): Disabling this doesn't seem to cause any problems -- but marking it as an
					//              Unknown just in case.
					//s_AssemblyCache[CORAL_ALC_CACHE_ID].Add(assemblyId, assembly);
					return assembly;
				}
			}

			LogMessage($"[AssemblyLoader] Resolving uncached assembly: {InAssemblyName.FullName}", MessageLevel.Trace);

			foreach (var assembly in InAssemblyLoadContext.Assemblies)
			{
				if (assembly.GetName().Name != InAssemblyName.Name)
					continue;

				s_AssemblyCache[alcId].Add(assemblyId, assembly);
				return assembly;
			}
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
		}

		Assembly? resolved;
		var tryResolve = (string directory) =>
		{
			string assemblyPath = Path.Combine(directory, $"{InAssemblyName.Name}.dll");
			LogMessage($"[AssemblyLoader] Trying to find assembly in {assemblyPath}", MessageLevel.Trace);
			if (InAssemblyLoadContext != null && File.Exists(assemblyPath))
			{
				LogMessage($"[AssemblyLoader] Found assembly {InAssemblyName.FullName}", MessageLevel.Trace);
				return InAssemblyLoadContext.LoadFromAssemblyPath(Path.GetFullPath(assemblyPath));
			}

			return null;
		};

		if ((resolved = tryResolve(AppContext.BaseDirectory)) != null) return resolved;

		if (InAssemblyLoadContext != null && InAssemblyLoadContext.Name != null)
		{
			int alcId = InAssemblyLoadContext.Name.GetHashCode();
			foreach (var path in s_AlcDllPaths[alcId])
			{
				if ((resolved = tryResolve(path)) != null) return resolved;
			}
		}

		return null;
	}

	[UnmanagedCallersOnly]
	internal static int CreateAssemblyLoadContext(NativeString InName, NativeString InDllPath)
	{
		return CreateContext(InName, InDllPath, collectible: true);
	}

	// A context whose assemblies every other context shares (see s_SharedContexts).
	[UnmanagedCallersOnly]
	internal static int CreateSharedAssemblyLoadContext(NativeString InName, NativeString InDllPath)
	{
		try
		{
			return CreateContext(InName, InDllPath, collectible: false);
		}
		catch (Exception error)
		{
			try { HandleException(error); } catch { }
			return -1;
		}
	}

	private static int CreateContext(NativeString InName, NativeString InDllPath, bool collectible)
	{
		string? name = InName;

		if (name == null)
			return -1;

		// A shared context outlives the HostInstance that made it; a later one gets it back.
		int existingId = name.GetHashCode();
		if (!collectible && s_SharedContextIds.Contains(existingId))
			return existingId;

		var alc = new AssemblyLoadContext(name, collectible);
		alc.Resolving += ResolveAssembly;
		if (collectible)
			alc.Unloading += ctx => s_AssemblyCache.Remove(ctx.Name!.GetHashCode());

		int contextId = name.GetHashCode();
		// A new context under an unloaded one's name is live, so QueryContextCollected forgets it.
		s_UnloadedContexts.Remove(contextId);
		s_AssemblyContexts.Add(contextId, alc);
		s_AssemblyCache.Add(contextId, new());
		if (!collectible)
		{
			s_SharedContexts.Add(alc);
			s_SharedContextIds.Add(contextId);
		}

		var path = InDllPath.ToString();
		LogMessage($"Added ALC '{name}' with ID '{contextId}'", MessageLevel.Trace);
		s_AlcDllPaths.Add(contextId, (path ?? "").Split(':'));

		return contextId;
	}

	// 1 once an unloaded context has been freed, 0 while something still holds it, -1 for a
	// context that was never unloaded. Meaningful after a full collection.
	[UnmanagedCallersOnly]
	internal static int QueryContextCollected(int InContextId)
	{
		if (!s_UnloadedContexts.TryGetValue(InContextId, out var context))
			return -1;

		return context.IsAlive ? 0 : 1;
	}

	[UnmanagedCallersOnly]
	internal static void UnloadAssemblyLoadContext(int InContextId)
	{
		try { UnloadContext(InContextId); }
		catch (Exception error)
		{
			// Diagnostics can allocate too; no managed exception may cross the native boundary.
			try { HandleException(error); } catch { }
		}
	}

	private static void UnloadContext(int InContextId)
	{
		if (!s_AssemblyContexts.TryGetValue(InContextId, out var alc))
		{
			LogMessage($"Cannot unload AssemblyLoadContext '{InContextId}', it was either never loaded or already unloaded.", MessageLevel.Warning);
			return;
		}

		if (alc == null)
		{
			LogMessage($"AssemblyLoadContext '{InContextId}' was found in dictionary but was null. This is most likely a bug.", MessageLevel.Error);
			return;
		}

		if (s_SharedContextIds.Contains(InContextId))
		{
			LogMessage($"AssemblyLoadContext '{alc.Name}' is shared by every other context and can't be unloaded.", MessageLevel.Error);
			return;
		}

#if DEBUG
		foreach (var assembly in alc.Assemblies)
		{
			var assemblyName = assembly.GetName();

			if (!s_AllocatedHandles.TryGetValue(assembly, out var handles))
			{
				continue;
			}

			// If everything is working properly, then there should not be anything left kicking around in the handles list.
			// If you see messages here, it probably means you are mis-managing the lifetime of unmanaged resources.
			// Managed objects that wrap an unmanaged resource need to implement IDisposable, and be Dispose()'d properly.
			// Example:
			//    // SceneQueryHitInterop wraps an unmanaged resource. It needs to implement IDisposable
			//    using(SceneQueryHitInterop hit = new())
			//    {
			//        Physics.CastRay(ray, out hit);   // Calls into native code, populates the unmanaged resource into hit
			//
			//        // Do something with hit
			//
			//    } // hit is Dispose()'d here
			//
			foreach (var handle in handles)
			{
				LogMessage($"Found still-registered handle '{(handle.Target is null? "null" : handle.Target)}' from assembly '{assemblyName}'", MessageLevel.Warning);

				if (!handle.IsAllocated || handle.Target == null)
				{
					continue;
				}

				LogMessage($"Found unfreed object '{handle.Target}' from assembly '{assemblyName}'. Deallocating.", MessageLevel.Warning);
				handle.Free();
			}

			s_AllocatedHandles.Remove(assembly);
		}
#endif

		// Preserve every still-live peer's native reflection handles. Remove all
		// managed cache roots owned by (or constructed with types from) this context.
		ManagedObject.RemoveContext(alc);
		TypeInterface.RemoveContext(alc);

		s_AssemblyContexts.Remove(InContextId);
		s_AlcDllPaths.Remove(InContextId);
		s_UnloadedContexts[InContextId] = new WeakReference(alc);
		alc.Unload();
	}

	[UnmanagedCallersOnly]
	internal static int LoadAssembly(int InContextId, NativeString InAssemblyFilePath)
	{
		try
		{
			if (string.IsNullOrEmpty(InAssemblyFilePath))
			{
				s_LastLoadStatus = AssemblyLoadStatus.InvalidFilePath;
				return -1;
			}

			if (!File.Exists(InAssemblyFilePath))
			{
				LogMessage($"Failed to load assembly '{InAssemblyFilePath}', file not found.", MessageLevel.Error);
				s_LastLoadStatus = AssemblyLoadStatus.FileNotFound;
				return -1;
			}

			if (!s_AssemblyContexts.TryGetValue(InContextId, out var alc))
			{
				LogMessage($"Failed to load assembly '{InAssemblyFilePath}', couldn't find AssemblyLoadContext with id {InContextId}.", MessageLevel.Error);
				s_LastLoadStatus = AssemblyLoadStatus.UnknownError;
				return -1;
			}

			if (alc == null)
			{
				LogMessage($"Failed to load assembly '{InAssemblyFilePath}', AssemblyLoadContext with id {InContextId} was null.", MessageLevel.Error);
				s_LastLoadStatus = AssemblyLoadStatus.UnknownError;
				return -1;
			}

			Assembly? assembly = null;

			// A shared context holds one copy of each assembly; loading it again (from a later
			// HostInstance, say) returns that copy.
			if (s_SharedContextIds.Contains(InContextId))
			{
				string requested = Path.GetFileNameWithoutExtension(InAssemblyFilePath!);
				foreach (var loaded in alc.Assemblies)
				{
					if (loaded.GetName().Name != requested)
						continue;

					int loadedId = requested.GetHashCode();
					s_AssemblyCache[InContextId].TryAdd(loadedId, loaded);
					s_LastLoadStatus = AssemblyLoadStatus.Success;
					return loadedId;
				}
			}

			using (var file = MemoryMappedFile.CreateFromFile(InAssemblyFilePath!, FileMode.Open, null, 0, MemoryMappedFileAccess.Read))
			{
				using var stream = file.CreateViewStream(0, 0, MemoryMappedFileAccess.Read);
				assembly = alc.LoadFromStream(stream);
			}

			LogMessage($"Loading assembly '{InAssemblyFilePath}'", MessageLevel.Info);
			var assemblyName = assembly.GetName();
			int assemblyId = assemblyName.Name!.GetHashCode();
			s_AssemblyCache[InContextId].Add(assemblyId, assembly);
			s_LastLoadStatus = AssemblyLoadStatus.Success;
			return assemblyId;
		}
		catch (Exception ex)
		{
			if (!s_AssemblyLoadErrorLookup.TryGetValue(ex.GetType(), out s_LastLoadStatus))
				s_LastLoadStatus = AssemblyLoadStatus.UnknownError;
			HandleException(ex);
			return -1;
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe int LoadAssemblyFromMemory(int InContextId, byte* data, long dataLength)
	{
		try
		{
			if (!s_AssemblyContexts.TryGetValue(InContextId, out var alc))
			{
				LogMessage($"Failed to load assembly, couldn't find AssemblyLoadContext with id {InContextId}.", MessageLevel.Error);
				s_LastLoadStatus = AssemblyLoadStatus.UnknownError;
				return -1;
			}

			if (alc == null)
			{
				LogMessage($"Failed to load assembly, couldn't find AssemblyLoadContext with id {InContextId} was null.", MessageLevel.Error);
				s_LastLoadStatus = AssemblyLoadStatus.UnknownError;
				return -1;
			}

			Assembly? assembly = null;

			using (var stream = new UnmanagedMemoryStream(data, dataLength))
			{
				assembly = alc.LoadFromStream(stream);
			}

			LogMessage($"Loading assembly '{assembly.FullName}'", MessageLevel.Info);
			var assemblyName = assembly.GetName();
			int assemblyId = assemblyName.Name!.GetHashCode();
			s_AssemblyCache[InContextId].Add(assemblyId, assembly);
			s_LastLoadStatus = AssemblyLoadStatus.Success;
			return assemblyId;
		}
		catch (Exception ex)
		{
			if (!s_AssemblyLoadErrorLookup.TryGetValue(ex.GetType(), out s_LastLoadStatus))
				s_LastLoadStatus = AssemblyLoadStatus.UnknownError;
			HandleException(ex);
			return -1;
		}
	}

	[UnmanagedCallersOnly]
	internal static AssemblyLoadStatus GetLastLoadStatus() => s_LastLoadStatus;

	[UnmanagedCallersOnly]
	internal static NativeString GetAssemblyName(int InContextId, int InAssemblyId)
	{
		if (!s_AssemblyCache[InContextId].TryGetValue(InAssemblyId, out var assembly))
		{
			LogMessage($"Couldn't get assembly name for assembly '{InAssemblyId}', assembly not in dictionary.", MessageLevel.Error);
			return "";
		}

		var assemblyName = assembly.GetName();
		return assemblyName.Name;
	}

#if DEBUG
	// In DEBUG builds, we track all GCHandles that are allocated by the managed code,
	// so that we can check that they've all been freed when the assembly is unloaded.
	internal static void RegisterHandle(Assembly InAssembly, GCHandle InHandle)
	{

		if (!s_AllocatedHandles.TryGetValue(InAssembly, out var handles))
		{
			s_AllocatedHandles.Add(InAssembly, new List<GCHandle>());
			handles = s_AllocatedHandles[InAssembly];
		}

		handles.Add(InHandle);
	}

	internal static void DeregisterHandle(Assembly InAssembly, GCHandle InHandle)
	{
		var assemblyName = InAssembly.GetName();

		if (!s_AllocatedHandles.TryGetValue(InAssembly, out var handles))
		{
			return;
		}

		if (!InHandle.IsAllocated)
		{
			LogMessage($"AssemblyLoader de-registering an already freed object from assembly '{assemblyName}'", MessageLevel.Error);
		}

		handles.Remove(InHandle);
	}
#endif
}
