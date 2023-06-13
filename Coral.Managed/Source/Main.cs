using System;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.Loader;

using Coral.Interop;

namespace Coral
{

	public class ManagedHost
	{
		[UnmanagedCallersOnly]
		public static void Initialize()
		{
			var assemblyLoadContexts = AssemblyLoadContext.All;

			Console.WriteLine($"There are {assemblyLoadContexts.Count()} assembly load contexts.");

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

		[StructLayout(LayoutKind.Sequential)]
		public struct InternalCall
		{
			public IntPtr NamePtr;
			public IntPtr NativeFunctionPtr;

			public string Name => Marshal.PtrToStringAuto(NamePtr);
		}

		public delegate void Dummy();

		[UnmanagedCallersOnly]
		public static UnmanagedString GetString()
		{
			return UnmanagedString.FromString("Hello, World!");
		}

		[UnmanagedCallersOnly]
		public static void SetInternalCalls(UnmanagedArray InArr)
		{
			var internalCalls = InArr.ToArray<InternalCall>();

			Console.WriteLine(internalCalls.Length);
			for (int i = 0; i < internalCalls.Length; i++)
			{
				Console.WriteLine($"Name = {internalCalls[i].Name}");

				var delegateType = Type.GetType(internalCalls[i].Name);
				var del = Marshal.GetDelegateForFunctionPointer(internalCalls[i].NativeFunctionPtr, delegateType);
				del.DynamicInvoke();
			}
		}
	}

}
