using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.Loader;

using Coral.Managed.Interop;

namespace Coral.Managed
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

		private static readonly Dictionary<ManagedType, Func<IntPtr, object>> s_MarshalFunctions = new()
		{
			{ ManagedType.SByte, InValue => Marshal.PtrToStructure<sbyte>(InValue) },
			{ ManagedType.Byte, InValue => Marshal.PtrToStructure<byte>(InValue) },
			{ ManagedType.Short, InValue => Marshal.PtrToStructure<short>(InValue) },
			{ ManagedType.UShort, InValue => Marshal.PtrToStructure<ushort>(InValue) },
			{ ManagedType.Int, InValue => Marshal.PtrToStructure<int>(InValue) },
			{ ManagedType.UInt, InValue => Marshal.PtrToStructure<uint>(InValue) },
			{ ManagedType.Long, InValue => Marshal.PtrToStructure<long>(InValue) },
			{ ManagedType.ULong, InValue => Marshal.PtrToStructure<ulong>(InValue) },
			{ ManagedType.Float, InValue => Marshal.PtrToStructure<float>(InValue) },
			{ ManagedType.Double, InValue => Marshal.PtrToStructure<double>(InValue) },
			{ ManagedType.Bool, InValue => Marshal.PtrToStructure<bool>(InValue) },
			{ ManagedType.Pointer, InValue => InValue }
		};

		[UnmanagedCallersOnly]
		public static void Initialize()
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

		[StructLayout(LayoutKind.Sequential)]
		public readonly struct ObjectCreateInfo
		{
			public readonly UnmanagedString TypeName;
			public readonly bool IsWeakRef;
			public readonly IntPtr Parameters;
			public readonly IntPtr ParameterTypes;
			public readonly int Length;
		}

		[UnmanagedCallersOnly]
		public static IntPtr CreateObject(IntPtr InCreateInfo)
		{
			var createInfo = Marshal.PtrToStructure<ObjectCreateInfo>(InCreateInfo);
			var type = TypeHelper.FindType(createInfo.TypeName);

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
					var paramType = (ManagedType)Marshal.ReadInt32(createInfo.ParameterTypes, i * Marshal.SizeOf<int>());
					constructParameters[i] = s_MarshalFunctions[paramType](Marshal.ReadIntPtr(createInfo.Parameters, i * Marshal.SizeOf<nint>()));
				}

				try
				{
					result = TypeHelper.CreateInstance(type, constructParameters);
				}
				catch (Exception ex)
				{
					Console.WriteLine(ex.ToString());
				}
			}
			else
			{
				result = TypeHelper.CreateInstance(type);
			}

			var handle = GCHandle.Alloc(result, !createInfo.IsWeakRef ? GCHandleType.Pinned : GCHandleType.Weak);
			return GCHandle.ToIntPtr(handle);
		}

		[UnmanagedCallersOnly]
		public static void DestroyObject(IntPtr InObjectHandle)
		{
			GCHandle.FromIntPtr(InObjectHandle).Free();
		}
	}

}
