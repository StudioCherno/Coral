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
		{
			result[i] = MarshalPointer(parameterPointers[i], parameterInfos[i].ParameterType);
		}

		return result;
	}
	
}