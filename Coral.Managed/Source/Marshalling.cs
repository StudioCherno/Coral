using System;
using System.Reflection;
using System.Runtime.InteropServices;
using Coral.Managed.Interop;

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

	public static object? MarshalArray(IntPtr InArray, Type InElementType)
	{
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

	public static void CopyArrayToBuffer(IntPtr InBuffer, Array InArray, Type InElementType)
	{
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

		return Marshal.PtrToStructure(InValue, InType);	
	}
	public static T? MarshalPointer<T>(IntPtr InValue) => Marshal.PtrToStructure<T>(InValue);

	public static object?[]? MarshalParameterArray(UnmanagedArray InParameterArray, MethodBase? InMethodInfo)
	{
		if (InMethodInfo == null)
			return null;

		if (InParameterArray.IsEmpty())
			return null;

		var parameterInfos = InMethodInfo.GetParameters();
		var parameterPointers = InParameterArray.ToIntPtrArray();
		var result = new object?[parameterPointers.Length];

		for (int i = 0; i < parameterPointers.Length; i++)
			result[i] = MarshalPointer(parameterPointers[i], parameterInfos[i].ParameterType);

		return result;
	}
	
}