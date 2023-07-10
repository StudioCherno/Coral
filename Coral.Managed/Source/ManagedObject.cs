using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.InteropServices;
using Coral.Managed.Interop;

namespace Coral.Managed;

internal static class ManagedObject
{
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
		{ ManagedType.Bool, InValue => Marshal.PtrToStructure<sbyte>(InValue) > 0 },
		{ ManagedType.Pointer, InValue => InValue }
	};
	
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
		try
		{
			var createInfo = Marshal.PtrToStructure<ObjectCreateInfo>(InCreateInfo);
			var type = TypeHelper.FindType(createInfo.TypeName);

			if (type == null)
			{
				Console.WriteLine($"[Coral.Managed]: Unknown type name '{createInfo.TypeName}'");
				return IntPtr.Zero;
			}

			object result;

			if (createInfo.Parameters != IntPtr.Zero && createInfo.ParameterTypes != IntPtr.Zero && createInfo.Length != 0)
			{
				object[] constructParameters = new object[createInfo.Length];

				for (int i = 0; i < createInfo.Length; i++)
				{
					var paramType = (ManagedType)Marshal.ReadInt32(createInfo.ParameterTypes, i * Marshal.SizeOf<int>());
					constructParameters[i] = s_MarshalFunctions[paramType](Marshal.ReadIntPtr(createInfo.Parameters, i * Marshal.SizeOf<nint>()));
				}

				result = TypeHelper.CreateInstance(type, constructParameters);
			}
			else
			{
				result = TypeHelper.CreateInstance(type);
			}

			var handle = GCHandle.Alloc(result, !createInfo.IsWeakRef ? GCHandleType.Pinned : GCHandleType.Weak);
			return GCHandle.ToIntPtr(handle);
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
			return IntPtr.Zero;
		}
	}

	[UnmanagedCallersOnly]
	public static void DestroyObject(IntPtr InObjectHandle)
	{
		try
		{
			GCHandle.FromIntPtr(InObjectHandle).Free();
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
		}
	}

	[UnmanagedCallersOnly]
	private static void InvokeMethod(IntPtr InObjectHandle, UnmanagedString InMethodName, IntPtr InParameterTypes, IntPtr InParameterValues, int InLength)
	{
		try
		{
			var target = GCHandle.FromIntPtr(InObjectHandle).Target;
			var targetType = target.GetType();

			MethodInfo methodInfo = null;
			foreach (var mi in targetType.GetMethods())
			{
				if (mi.Name != InMethodName || mi.GetParameters().Length != InLength)
					continue;
				
				methodInfo = mi;
				break;
			}

			object[] methodParameters = null;

			if (InParameterTypes != IntPtr.Zero && InParameterValues != IntPtr.Zero && InLength > 0)
			{
				methodParameters = new object[InLength];
				
				for (int i = 0; i < InLength; i++)
				{
					var paramType = (ManagedType)Marshal.ReadInt32(InParameterTypes, i * Marshal.SizeOf<int>());
					methodParameters[i] = s_MarshalFunctions[paramType](Marshal.ReadIntPtr(InParameterValues, i * Marshal.SizeOf<nint>()));
				}
			}
			
			methodInfo.Invoke(target, methodParameters);
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
		}
	}

	private static void CopyToUnmanaged<T>(object InValue, IntPtr InResultStorage, ulong InResultSize)
	{
		var value = (T)InValue;
		unsafe
		{
			Buffer.MemoryCopy(&value, InResultStorage.ToPointer(), InResultSize, InResultSize);
		}
	}
	
	[UnmanagedCallersOnly]
	private static void InvokeMethodRet(IntPtr InObjectHandle, UnmanagedString InMethodName, IntPtr InParameterTypes, IntPtr InParameterValues, int InLength, IntPtr InResultStorage, ulong InResultSize, ManagedType InResultType)
	{
		try
		{
			var target = GCHandle.FromIntPtr(InObjectHandle).Target;
			var targetType = target.GetType();

			MethodInfo methodInfo = null;
			foreach (var mi in targetType.GetMethods())
			{
				if (mi.Name != InMethodName || mi.GetParameters().Length != InLength)
					continue;
				
				methodInfo = mi;
				break;
			}

			if (methodInfo == null)
				throw new MissingMethodException($"Couldn't find method called {InMethodName}");

			object[] methodParameters = null;

			if (InParameterTypes != IntPtr.Zero && InParameterValues != IntPtr.Zero && InLength > 0)
			{
				methodParameters = new object[InLength];
				
				for (int i = 0; i < InLength; i++)
				{
					var paramType = (ManagedType)Marshal.ReadInt32(InParameterTypes, i * Marshal.SizeOf<int>());
					methodParameters[i] = s_MarshalFunctions[paramType](Marshal.ReadIntPtr(InParameterValues, i * Marshal.SizeOf<nint>()));
				}
			}
			
			object value = methodInfo.Invoke(target, methodParameters);

			switch (InResultType)
			{
			case ManagedType.SByte:
			{
				CopyToUnmanaged<sbyte>(value, InResultStorage, InResultSize);
				break;
			}
			case ManagedType.Byte:
			{
				CopyToUnmanaged<byte>(value, InResultStorage, InResultSize);
				break;
			}
			case ManagedType.Short:
			{
				CopyToUnmanaged<short>(value, InResultStorage, InResultSize);
				break;
			}
			case ManagedType.UShort:
			{
				CopyToUnmanaged<ushort>(value, InResultStorage, InResultSize);
				break;
			}
			case ManagedType.Int:
			{
				CopyToUnmanaged<int>(value, InResultStorage, InResultSize);
				break;
			}
			case ManagedType.UInt:
			{
				CopyToUnmanaged<uint>(value, InResultStorage, InResultSize);
				break;
			}
			case ManagedType.Long:
			{
				CopyToUnmanaged<long>(value, InResultStorage, InResultSize);
				break;
			}
			case ManagedType.ULong:
			{
				CopyToUnmanaged<ulong>(value, InResultStorage, InResultSize);
				break;
			}
			case ManagedType.Float:
			{
				CopyToUnmanaged<float>(value, InResultStorage, InResultSize);
				break;
			}
			case ManagedType.Double:
			{
				CopyToUnmanaged<double>(value, InResultStorage, InResultSize);
				break;
			}
			case ManagedType.Bool:
			{
				CopyToUnmanaged<bool>(value, InResultStorage, InResultSize);
				break;
			}
			case ManagedType.Pointer:
			{
				CopyToUnmanaged<IntPtr>(value, InResultStorage, InResultSize);
				break;
			}
			}
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
		}
	}
}