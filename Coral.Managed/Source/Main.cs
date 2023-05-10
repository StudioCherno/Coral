using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.Loader;

namespace Coral {

	public class ManagedHost
	{

		[StructLayout(LayoutKind.Sequential)]
		private struct DummyData
		{
			public float X;
			public IntPtr Str;
		}

		[UnmanagedCallersOnly]
		public static int Initialize(IntPtr InArguments)
		{
			var dummyData = Marshal.PtrToStructure<DummyData>(InArguments);
			Console.WriteLine($"X={dummyData.X}");
			Console.WriteLine($"Str={Marshal.PtrToStringAuto(dummyData.Str)}");

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

			//Console.WriteLine($"Hello! Arguments Size: {InArgumentsSize}");
			return 0;
		}

		[StructLayout(LayoutKind.Sequential)]
		public struct InternalCall
		{
			public IntPtr NamePtr;
			public IntPtr NativeFunctionPtr;

			public string Name => Marshal.PtrToStringAuto(NamePtr);
		}

		[StructLayout(LayoutKind.Sequential)]
		public unsafe struct InternalCallsList
		{
			public InternalCall** InternalCalls;
			public int NumInternalCalls;

			public List<InternalCall> GetInternalCalls()
			{
				var result = new List<InternalCall>();
				for (int i = 0; i < NumInternalCalls; i++)
					result.Add(*InternalCalls[i]);
				return result;
			}
		}

		public delegate void Dummy();

		[UnmanagedCallersOnly]
		public static void SetInternalCalls(InternalCallsList InList)
		{
			var internalCalls = InList.GetInternalCalls();
			Console.WriteLine(internalCalls.Count);
			foreach (var internalCall in internalCalls)
			{
				Console.WriteLine($"Name = {Marshal.PtrToStringAuto(internalCall.NamePtr)}");
				
				var del = Marshal.GetDelegateForFunctionPointer<Dummy>(internalCall.NativeFunctionPtr);
				del();
			}
		}
	}

}
