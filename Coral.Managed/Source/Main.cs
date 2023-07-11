using System;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.Loader;
using Coral.Managed.Interop;

namespace Coral.Managed
{
	
	internal static class ManagedHost
	{
		private static unsafe delegate*<UnmanagedString, void> s_ExceptionCallback;

		[UnmanagedCallersOnly]
		private static void Initialize()
		{
			var assemblyLoadContexts = AssemblyLoadContext.All;
			
			foreach (var alc in assemblyLoadContexts)
			{
				Console.WriteLine($"Name: {alc.Name}");
				Console.WriteLine($"Assemblies: {alc.Assemblies.Count()}");
				foreach (var assembly in alc.Assemblies)
				{
					Console.WriteLine($"\tName: {assembly.FullName}");
				}
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe void SetExceptionCallback(delegate*<UnmanagedString, void> InCallback)
		{
			s_ExceptionCallback = InCallback;
		}

		internal static void HandleException(Exception InException)
		{
			unsafe
			{
				if (s_ExceptionCallback == null)
					return;

				var message = UnmanagedString.FromString(InException.ToString());
				s_ExceptionCallback(message);
				message.Free();
			}
		}

		
	}
}
