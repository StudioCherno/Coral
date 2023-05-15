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

	public class AssemblyLoader
	{

		internal static Dictionary<Type, AssemblyLoadStatus> s_AssemblyLoadErrorLookup;

		static AssemblyLoader()
		{
			s_AssemblyLoadErrorLookup = new Dictionary<Type, AssemblyLoadStatus>();
			s_AssemblyLoadErrorLookup.Add(typeof(BadImageFormatException), AssemblyLoadStatus.InvalidAssembly);
			s_AssemblyLoadErrorLookup.Add(typeof(FileNotFoundException), AssemblyLoadStatus.FileNotFound);
			s_AssemblyLoadErrorLookup.Add(typeof(FileLoadException), AssemblyLoadStatus.FileLoadFailure);
			s_AssemblyLoadErrorLookup.Add(typeof(ArgumentNullException), AssemblyLoadStatus.InvalidFilePath);
			s_AssemblyLoadErrorLookup.Add(typeof(ArgumentException), AssemblyLoadStatus.InvalidFilePath);
		}

		[UnmanagedCallersOnly]
		public static AssemblyLoadStatus LoadAssembly(UnmanagedString InAssemblyFilePath)
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
				assembly = AssemblyLoadContext.Default.LoadFromAssemblyPath(InAssemblyFilePath);
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
