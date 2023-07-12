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

		internal struct ReflectionType
		{
			public UnmanagedString FullName;
			public UnmanagedString Name;
			public UnmanagedString Namespace;
			public UnmanagedString BaseTypeName;
		}

		private static ReflectionType? BuildReflectionType(Type? InType)
		{
			if (InType == null)
				return null;
			
			var reflectionType = new ReflectionType
			{
				FullName = UnmanagedString.FromString(InType.FullName),
				Name = UnmanagedString.FromString(InType.Name),
				Namespace = UnmanagedString.FromString(InType.Namespace)
			};

			if (InType.BaseType != null)
			{
				reflectionType.BaseTypeName = UnmanagedString.FromString(InType.BaseType.FullName);
			}

			return reflectionType;
		}

		[UnmanagedCallersOnly]
		private static unsafe Bool32 GetReflectionType(UnmanagedString InTypeName, ReflectionType* OutReflectionType)
		{
			try
			{
				var type = TypeHelper.FindType(InTypeName);
				var reflectionType = BuildReflectionType(type);

				if (reflectionType == null)
					return false;

				*OutReflectionType = reflectionType.Value;
				return true;
			}
			catch (Exception ex)
			{
				HandleException(ex);
				return false;
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe Bool32 GetReflectionTypeFromObject(IntPtr InObjectHandle, ReflectionType* OutReflectionType)
		{
			try
			{
				var target = GCHandle.FromIntPtr(InObjectHandle).Target;
				var reflectionType = BuildReflectionType(target.GetType());

				if (reflectionType == null)
					return false;

				*OutReflectionType = reflectionType.Value;
				return true;
			}
			catch (Exception e)
			{
				HandleException(e);
				return false;
			}
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
