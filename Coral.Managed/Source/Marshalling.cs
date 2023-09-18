using Coral.Managed.Interop;

using System;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Coral.Managed;

public enum ManagedDataType
{
	SByte, Byte,
	Short, UShort,
	Int, UInt,
	Long, ULong,

	Float, Double,

	Bool,
	String,
	
	Struct
}

public readonly struct ManagedType
{
	public readonly ManagedDataType Type;
	public readonly bool IsPointer;
}

public static class Marshalling
{

	public static object? MarshalPointer(IntPtr InValue, ManagedType InManagedType)
	{
		if (InManagedType.IsPointer)
		{
			return InValue;
		}
		
		return InManagedType.Type switch
		{
			ManagedDataType.SByte => Marshal.PtrToStructure<sbyte>(InValue),
			ManagedDataType.Byte => Marshal.PtrToStructure<byte>(InValue),
			ManagedDataType.Short => Marshal.PtrToStructure<short>(InValue),
			ManagedDataType.UShort => Marshal.PtrToStructure<ushort>(InValue),
			ManagedDataType.Int => Marshal.PtrToStructure<int>(InValue),
			ManagedDataType.UInt => Marshal.PtrToStructure<uint>(InValue),
			ManagedDataType.Long => Marshal.PtrToStructure<long>(InValue),
			ManagedDataType.ULong => Marshal.PtrToStructure<ulong>(InValue),
			ManagedDataType.Float => Marshal.PtrToStructure<float>(InValue),
			ManagedDataType.Double => Marshal.PtrToStructure<double>(InValue),
			ManagedDataType.Bool => Marshal.PtrToStructure<sbyte>(InValue) > 0,
			ManagedDataType.String => Marshal.PtrToStructure<UnmanagedString>(InValue),
			ManagedDataType.Struct => InValue,
			_ => null
		};
	}

	struct ArrayContainer
	{
		public IntPtr Data;
		public int Length;
	};

	public static object? MarshalArray(IntPtr InArray, Type? InElementType)
	{
		if (InElementType == null)
			return null;

		var arrayContainer = MarshalPointer<ArrayContainer>(InArray);
		var elements = Array.CreateInstance(InElementType, arrayContainer.Length);
		int elementSize = Marshal.SizeOf(InElementType);

		unsafe
		{
			for (int i = 0; i < arrayContainer.Length; i++)
			{
				IntPtr source = (IntPtr)(((byte*)arrayContainer.Data.ToPointer()) + (i * elementSize));
				elements.SetValue(Marshal.PtrToStructure(source, InElementType), i);
			}
		}

		return elements;
	}

	public static void CopyArrayToBuffer(IntPtr InBuffer, Array? InArray, Type? InElementType)
	{
		if (InArray == null || InElementType == null)
			return;

		var elementSize = Marshal.SizeOf(InElementType);
		int byteLength = InArray.Length * elementSize;
		var mem = Marshal.AllocHGlobal(byteLength);

		int offset = 0;

		foreach (var elem in InArray)
		{
			var elementHandle = GCHandle.Alloc(elem, GCHandleType.Pinned);

			unsafe
			{
				Buffer.MemoryCopy(elementHandle.AddrOfPinnedObject().ToPointer(), ((byte*)mem.ToPointer()) + offset, elementSize, elementSize);
			}

			offset += elementSize;

			elementHandle.Free();
		}

		ArrayContainer container = new()
		{
			Data = mem,
			Length = InArray.Length
		};

		var handle = GCHandle.Alloc(container, GCHandleType.Pinned);
		
		unsafe
		{
			Buffer.MemoryCopy(handle.AddrOfPinnedObject().ToPointer(), InBuffer.ToPointer(), Marshal.SizeOf<ArrayContainer>(), Marshal.SizeOf<ArrayContainer>());
		}

		handle.Free();
	}

	public static object? MarshalPointer(IntPtr InValue, Type InType)
	{
		if (InType.IsPointer || InType == typeof(IntPtr))
			return InValue;

		// NOTE(Peter): Marshal.PtrToStructure<bool> doesn't seem to work
		//				instead we have to read it as a single byte and check the raw value
		if (InType == typeof(bool))
			return Marshal.PtrToStructure<byte>(InValue) > 0;

		if (InType == typeof(string))
			return Marshal.PtrToStringAuto(InValue);

		if (InType.IsSZArray)
			return MarshalArray(InValue, InType.GetElementType());


		if (InType.IsGenericType)
		{
			if (InType == typeof(NativeArray<>).MakeGenericType(InType.GetGenericArguments().First()))
			{
				var elements = Marshal.ReadIntPtr(InValue, 0);
				var elementCount = Marshal.ReadInt32(InValue, Marshal.SizeOf<IntPtr>());
				var genericType = typeof(NativeArray<>).MakeGenericType(InType.GetGenericArguments().First());
				return TypeHelper.CreateInstance(genericType, elements, elementCount);
			}
		}

		return Marshal.PtrToStructure(InValue, InType);	
	}
	public static T? MarshalPointer<T>(IntPtr InValue) => Marshal.PtrToStructure<T>(InValue);

	public static IntPtr[] NativeArrayToIntPtrArray(IntPtr InNativeArray, int InLength)
	{
		try
		{
			if (InNativeArray == IntPtr.Zero || InLength == 0)
				return Array.Empty<IntPtr>();

			IntPtr[] result = new IntPtr[InLength];

			for (int i = 0; i < InLength; i++)
				result[i] = Marshal.ReadIntPtr(InNativeArray, i * Marshal.SizeOf<nint>());

			return result;
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
			return Array.Empty<IntPtr>();
		}
	}

	public static object?[]? MarshalParameterArray(IntPtr InNativeArray, int InLength, MethodBase? InMethodInfo)
	{
		if (InMethodInfo == null)
			return null;

		if (InNativeArray == IntPtr.Zero || InLength == 0)
			return null;

		var parameterInfos = InMethodInfo.GetParameters();
		var parameterPointers = NativeArrayToIntPtrArray(InNativeArray, InLength);
		var result = new object?[parameterPointers.Length];

		for (int i = 0; i < parameterPointers.Length; i++)
			result[i] = MarshalPointer(parameterPointers[i], parameterInfos[i].ParameterType);

		return result;
	}
	
}