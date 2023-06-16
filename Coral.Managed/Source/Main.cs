using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.Loader;

using Coral.Interop;

namespace Coral
{

	public class ManagedHost
	{
		private enum ManagedType
		{
			SByte, Byte,
			Short, UShort,
			Int, UInt,
			Long, ULong,

			Float, Double,

			Bool,

			Pointer
		};

		private static readonly Dictionary<ManagedType, Func<IntPtr, object>> s_MarshalFunctions = new Dictionary<ManagedType, Func<nint, object>>()
		{
			{ ManagedType.SByte, (IntPtr InValue) => { return Marshal.PtrToStructure<sbyte>(InValue); } },
			{ ManagedType.Byte, (IntPtr InValue) => { return Marshal.PtrToStructure<byte>(InValue); } },
			{ ManagedType.Short, (IntPtr InValue) => { return Marshal.PtrToStructure<short>(InValue); } },
			{ ManagedType.UShort, (IntPtr InValue) => { return Marshal.PtrToStructure<ushort>(InValue); } },
			{ ManagedType.Int, (IntPtr InValue) => { return Marshal.PtrToStructure<int>(InValue); } },
			{ ManagedType.UInt, (IntPtr InValue) => { return Marshal.PtrToStructure<uint>(InValue); } },
			{ ManagedType.Long, (IntPtr InValue) => { return Marshal.PtrToStructure<long>(InValue); } },
			{ ManagedType.ULong, (IntPtr InValue) => { return Marshal.PtrToStructure<ulong>(InValue); } },
			{ ManagedType.Float, (IntPtr InValue) => { return Marshal.PtrToStructure<float>(InValue); } },
			{ ManagedType.Double, (IntPtr InValue) => { return Marshal.PtrToStructure<double>(InValue); } },
			{ ManagedType.Bool, (IntPtr InValue) => { return Marshal.PtrToStructure<bool>(InValue); } },
			{ ManagedType.Pointer, (IntPtr InValue) => { return InValue; } }
		};

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

		public class Test
		{
			public readonly int X;

			public Test(int x)
			{
				X = x;
			}
		}

		[StructLayout(LayoutKind.Sequential)]
		public struct ObjectCreateInfo
		{
			public UnmanagedString TypeName;
			public bool IsWeakRef;
			public IntPtr Parameters;
			public IntPtr ParameterTypes;
			public int Length;
		}

		[UnmanagedCallersOnly]
		public static IntPtr CreateObject(IntPtr InCreateInfo)
		{
			var createInfo = Marshal.PtrToStructure<ObjectCreateInfo>(InCreateInfo);
			var type = Type.GetType(createInfo.TypeName);

			if (type == null)
			{
				Console.WriteLine($"[Coral.Managed]: Unknown type name '{createInfo.TypeName}'");
				return IntPtr.Zero;
			}

			object result = null;

			if (createInfo.Parameters != IntPtr.Zero && createInfo.ParameterTypes != IntPtr.Zero && createInfo.Length != 0)
			{
				object[] constructParameters = new object[createInfo.Length];

				for (int i = 0; i < createInfo.Length; i++)
				{
					ManagedType paramType = (ManagedType)Marshal.ReadInt32(createInfo.ParameterTypes, i * Marshal.SizeOf<int>());
					constructParameters[i] = s_MarshalFunctions[paramType](Marshal.ReadIntPtr(createInfo.Parameters, i * Marshal.SizeOf<nint>()));
				}

				result = Activator.CreateInstance(type, constructParameters);
			}
			else
			{
				result = Activator.CreateInstance(type);
			}

			var handle = GCHandle.Alloc(result, !createInfo.IsWeakRef ? GCHandleType.Pinned : GCHandleType.Weak);
			return GCHandle.ToIntPtr(handle);
		}

		[UnmanagedCallersOnly]
		public static void DestroyObject(IntPtr InObjectHandle)
		{
			GCHandle.FromIntPtr(InObjectHandle).Free();
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
