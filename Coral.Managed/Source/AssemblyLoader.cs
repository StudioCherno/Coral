using Coral.Managed.Interop;

using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Loader;

namespace Coral.Managed
{

	public enum AssemblyLoadStatus
	{
		Success, FileNotFound, FileLoadFailure, InvalidFilePath, InvalidAssembly, UnknownError
	}

	public static class AssemblyLoader
	{
		private static readonly Dictionary<Type, AssemblyLoadStatus> s_AssemblyLoadErrorLookup = new();
		private static readonly Dictionary<int, Assembly> s_AssemblyCache = new();
		private static AssemblyLoadStatus s_LastLoadStatus = AssemblyLoadStatus.Success;

		private static readonly AssemblyLoadContext s_CoralAssemblyLoadContext;
		private static readonly AssemblyLoadContext s_AppAssemblyLoadContext;

		static AssemblyLoader()
		{
			s_AssemblyLoadErrorLookup.Add(typeof(BadImageFormatException), AssemblyLoadStatus.InvalidAssembly);
			s_AssemblyLoadErrorLookup.Add(typeof(FileNotFoundException), AssemblyLoadStatus.FileNotFound);
			s_AssemblyLoadErrorLookup.Add(typeof(FileLoadException), AssemblyLoadStatus.FileLoadFailure);
			s_AssemblyLoadErrorLookup.Add(typeof(ArgumentNullException), AssemblyLoadStatus.InvalidFilePath);
			s_AssemblyLoadErrorLookup.Add(typeof(ArgumentException), AssemblyLoadStatus.InvalidFilePath);

			s_CoralAssemblyLoadContext = AssemblyLoadContext.GetLoadContext(typeof(AssemblyLoader).Assembly);
			s_CoralAssemblyLoadContext!.Resolving += ResolveAssembly;

			s_AppAssemblyLoadContext = new AssemblyLoadContext("AppAssemblyContext", true);
			s_AppAssemblyLoadContext.Resolving += ResolveAssembly;

			CacheCoralAssemblies();
		}

		private static void CacheCoralAssemblies()
		{
			foreach (var assembly in s_CoralAssemblyLoadContext.Assemblies)
			{
				int assemblyId = assembly.GetName().FullName.GetHashCode();
				s_AssemblyCache.Add(assemblyId, assembly);
			}
		}

		internal static Assembly ResolveAssembly(AssemblyLoadContext InAssemblyLoadContext, AssemblyName InAssemblyName)
		{
			int assemblyId = InAssemblyName.FullName.GetHashCode();
			
			if (s_AssemblyCache.TryGetValue(assemblyId, out var cachedAssembly))
			{
				return cachedAssembly;
			}

			foreach (var loadContext in AssemblyLoadContext.All)
			{
				foreach (var assembly in loadContext.Assemblies)
				{
					if (assembly.GetName().Name != InAssemblyName.Name)
						continue;
					
					s_AssemblyCache.Add(assemblyId, assembly);
					return assembly;
				}
			}

			return null;
		}

		[UnmanagedCallersOnly]
		public static int LoadAssembly(UnmanagedString InAssemblyFilePath)
		{
			if (InAssemblyFilePath.IsNull())
			{
				s_LastLoadStatus = AssemblyLoadStatus.InvalidFilePath;
				return -1;
			}

			if (!File.Exists(InAssemblyFilePath))
			{
				Console.WriteLine($"File {InAssemblyFilePath} not found!");
				s_LastLoadStatus = AssemblyLoadStatus.FileNotFound;
				return -1;
			}

			int assemblyId;

			try
			{
				var assembly = s_AppAssemblyLoadContext.LoadFromAssemblyPath(InAssemblyFilePath);
				var assemblyName = assembly.GetName();
				assemblyId = assemblyName.FullName.GetHashCode();
				s_AssemblyCache.Add(assemblyId, assembly);
			}
			catch (Exception ex)
			{
				s_LastLoadStatus = !s_AssemblyLoadErrorLookup.TryGetValue(ex.GetType(), out var error) ? AssemblyLoadStatus.UnknownError : error;
				return -1;
			}

			Console.WriteLine($"Loaded assembly {InAssemblyFilePath}");
			return assemblyId;
		}

		[UnmanagedCallersOnly]
		public static AssemblyLoadStatus GetLastLoadStatus() => s_LastLoadStatus;

	}
}
