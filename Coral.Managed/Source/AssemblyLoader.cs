﻿using Coral.Managed.Interop;

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
	private static readonly Dictionary<int, List<GCHandle>> s_AllocatedHandles = new();
#endif
	private static AssemblyLoadStatus s_LastLoadStatus = AssemblyLoadStatus.Success;

	private static readonly int CORAL_ALC_CACHE_ID = -1;
	private static readonly AssemblyLoadContext? s_CoralAssemblyLoadContext;

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
		string? name = InName;

		if (name == null)
			return -1;

		var alc = new AssemblyLoadContext(name, true);
		alc.Resolving += ResolveAssembly;
		alc.Unloading += ctx => s_AssemblyCache.Remove(ctx.Name!.GetHashCode());

		int contextId = name.GetHashCode();
		s_AssemblyContexts.Add(contextId, alc);
		s_AssemblyCache.Add(contextId, new());

		var path = InDllPath.ToString();
		LogMessage($"Added ALC '{name}' with ID '{contextId}'", MessageLevel.Trace);
		s_AlcDllPaths.Add(contextId, (path ?? "").Split(':'));

		return contextId;
	}

	[UnmanagedCallersOnly]
	internal static void UnloadAssemblyLoadContext(int InContextId)
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

#if DEBUG
		foreach (var assembly in alc.Assemblies)
		{
			var assemblyName = assembly.GetName();
			int assemblyId = assemblyName.Name!.GetHashCode();

			if (!s_AllocatedHandles.TryGetValue(assemblyId, out var handles))
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

			s_AllocatedHandles.Remove(assemblyId);
		}
#endif

		ManagedObject.s_CachedMethods.Clear();

		TypeInterface.s_CachedTypes.Clear();
		TypeInterface.s_CachedMethods.Clear();
		TypeInterface.s_CachedFields.Clear();
		TypeInterface.s_CachedProperties.Clear();
		TypeInterface.s_CachedAttributes.Clear();

		s_AssemblyContexts.Remove(InContextId);
		s_AlcDllPaths.Remove(InContextId);
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

			using (var file = MemoryMappedFile.CreateFromFile(InAssemblyFilePath!))
			{
				using var stream = file.CreateViewStream();
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
			s_AssemblyLoadErrorLookup.TryGetValue(ex.GetType(), out s_LastLoadStatus);
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
			s_AssemblyLoadErrorLookup.TryGetValue(ex.GetType(), out s_LastLoadStatus);
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
		var assemblyName = InAssembly.GetName();
		int assemblyId = assemblyName.Name!.GetHashCode();

		if (!s_AllocatedHandles.TryGetValue(assemblyId, out var handles))
		{
			s_AllocatedHandles.Add(assemblyId, new List<GCHandle>());
			handles = s_AllocatedHandles[assemblyId];
		}

		handles.Add(InHandle);
	}

	internal static void DeregisterHandle(Assembly InAssembly, GCHandle InHandle)
	{
		var assemblyName = InAssembly.GetName();
		int assemblyId = assemblyName.Name!.GetHashCode();

		if (!s_AllocatedHandles.TryGetValue(assemblyId, out var handles))
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
