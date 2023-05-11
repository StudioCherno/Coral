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

		public delegate void Dummy();

		[StructLayout(LayoutKind.Sequential)]
		public struct UnmanagedArray
		{
			public IntPtr Ptr;
			public int Length;

			public T[] As<T>() where T : struct
			{
				T[] result = new T[Length];

				for (int i = 0; i < Length; i++)
				{
					IntPtr elementPtr = Marshal.ReadIntPtr(Ptr, i * Marshal.SizeOf<nint>());
					result[i] = Marshal.PtrToStructure<T>(elementPtr);
				}

				return result;
			}
		}

		[UnmanagedCallersOnly]
		public static void SetInternalCalls(UnmanagedArray InArr)
		{
			var internalCalls = InArr.As<InternalCall>();

			Console.WriteLine(internalCalls.Length);
			for (int i = 0; i < internalCalls.Length; i++)
			{
				Console.WriteLine($"Name = {Marshal.PtrToStringAuto(internalCalls[i].NamePtr)}");
				
				var del = Marshal.GetDelegateForFunctionPointer<Dummy>(internalCalls[i].NativeFunctionPtr);
				del();
			}
		}
	}

}
