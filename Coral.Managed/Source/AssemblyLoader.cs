using Coral.Interop;

using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Loader;

namespace Coral
{

	public enum AssemblyLoadStatus
	{
		Success, FileNotFound, FileLoadFailure, InvalidFilePath, InvalidAssembly, UnknownError
	}

	public static class AssemblyLoader
	{
		private static Dictionary<Type, AssemblyLoadStatus> s_AssemblyLoadErrorLookup = new Dictionary<Type, AssemblyLoadStatus>();
		private static Dictionary<ushort, Assembly> s_LoadedAssemblies = new Dictionary<ushort, Assembly>();

		static AssemblyLoader()
		{
			s_AssemblyLoadErrorLookup.Add(typeof(BadImageFormatException), AssemblyLoadStatus.InvalidAssembly);
			s_AssemblyLoadErrorLookup.Add(typeof(FileNotFoundException), AssemblyLoadStatus.FileNotFound);
			s_AssemblyLoadErrorLookup.Add(typeof(FileLoadException), AssemblyLoadStatus.FileLoadFailure);
			s_AssemblyLoadErrorLookup.Add(typeof(ArgumentNullException), AssemblyLoadStatus.InvalidFilePath);
			s_AssemblyLoadErrorLookup.Add(typeof(ArgumentException), AssemblyLoadStatus.InvalidFilePath);
		}

		[UnmanagedCallersOnly]
		public static AssemblyLoadStatus LoadAssembly(ushort InAssemblyID, UnmanagedString InAssemblyFilePath)
		{
			if (InAssemblyFilePath == null)
				return AssemblyLoadStatus.InvalidFilePath;

			if (!File.Exists(InAssemblyFilePath))
			{
				Console.WriteLine($"File {InAssemblyFilePath} not found!");
				return AssemblyLoadStatus.FileNotFound;
			}

			Assembly assembly = null;

			try
			{
				var assm = Assembly.GetAssembly(typeof(AssemblyLoader));
				var alc = AssemblyLoadContext.GetLoadContext(assm);
				assembly = alc.LoadFromAssemblyPath(InAssemblyFilePath);
				s_LoadedAssemblies.Add(InAssemblyID, assembly);
			}
			catch (Exception ex)
			{
				var error = AssemblyLoadStatus.UnknownError;
				_ = s_AssemblyLoadErrorLookup.TryGetValue(ex.GetType(), out error);
				return error;
			}

			Console.WriteLine($"Loaded assembly {InAssemblyFilePath}");
			return AssemblyLoadStatus.Success;
		}

	}
}
