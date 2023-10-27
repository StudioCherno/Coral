using Coral.Managed.Interop;

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Coral.Managed;

using static ManagedHost;

internal enum ManagedType
{
	Unknown,

	SByte,
	Byte,
	Short,
	UShort,
	Int,
	UInt,
	Long,
	ULong,

	Float,
	Double,

	Bool,

	Pointer
};

internal static class ManagedObject
{

	public readonly struct MethodKey : IEquatable<MethodKey>
	{
		public readonly string TypeName;
		public readonly string Name;
		public readonly ManagedType[] Types;
		public readonly int ParameterCount;

		public MethodKey(string InTypeName, string InName, ManagedType[] InTypes, int InParameterCount)
		{
			TypeName = InTypeName;
			Name = InName;
			Types = InTypes;
			ParameterCount = InParameterCount;
		}

		public override bool Equals([NotNullWhen(true)] object? obj) => obj is MethodKey other && Equals(other);

		bool IEquatable<MethodKey>.Equals(MethodKey other)
		{
			if (TypeName != other.TypeName || Name != other.Name)
				return false;

			for (int i = 0; i < Types.Length; i++)
			{
				if (Types[i] != other.Types[i])
					return false;
			}

			return ParameterCount == other.ParameterCount;
		}

		public override int GetHashCode()
		{
			// NOTE(Peter): Josh Bloch's Hash (from https://stackoverflow.com/questions/263400/what-is-the-best-algorithm-for-overriding-gethashcode)
			unchecked
			{
				int hash = 17;

				hash = hash * 23 + TypeName.GetHashCode();
				hash = hash * 23 + Name.GetHashCode();
				foreach (var type in Types)
					hash = hash * 23 + type.GetHashCode();
				hash = hash * 23 + ParameterCount.GetHashCode();

				return hash;
			}
		}
	}

	internal static Dictionary<MethodKey, MethodInfo> s_CachedMethods = new Dictionary<MethodKey, MethodInfo>();

	private struct ObjectData
	{
		public IntPtr Handle;
		public NativeString FullName;
	}

	[UnmanagedCallersOnly]
	private static unsafe ObjectData CreateObject(NativeString InTypeName, Bool32 InWeakRef, IntPtr InParameters, ManagedType* InParameterTypes, int InParameterCount)
	{
		try
		{
			var type = TypeInterface.FindType(InTypeName);

			if (type == null)
			{
				LogMessage($"Failed to find type with name '{InTypeName}'.", MessageLevel.Error);
			}

			ConstructorInfo? constructor = null;

			var currentType = type;
			while (currentType != null)
			{
				ReadOnlySpan<ConstructorInfo> constructors = currentType.GetConstructors(BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance);
				
				constructor = TypeInterface.FindSuitableMethod(".ctor", InParameterTypes, InParameterCount, constructors);

				if (constructor != null)
					break;

				currentType = currentType.BaseType;
			}

			if (constructor == null)
			{
				LogMessage($"Failed to find constructor for type {InTypeName} with {InParameterCount} parameters.", MessageLevel.Error);
				return default;
			}

			var parameters = Marshalling.MarshalParameterArray(InParameters, InParameterCount, constructor);

			object? result = null;

			if (currentType != type || parameters == null)
			{
				result = TypeInterface.CreateInstance(type);

				if (currentType != type)
					constructor.Invoke(result, parameters);
			}
			else
			{
				result = TypeInterface.CreateInstance(type, parameters);
			}

			if (result == null)
			{
				LogMessage($"Failed to instantiate type {InTypeName}.", MessageLevel.Error);
			}

			var handle = GCHandle.Alloc(result, InWeakRef ? GCHandleType.Weak : GCHandleType.Normal);
			AssemblyLoader.RegisterHandle(type.Assembly, handle);
			return new()
			{
				Handle = GCHandle.ToIntPtr(handle),
				FullName = type.FullName
			};
		}
		catch (Exception ex)
		{
			HandleException(ex);
			return new() { Handle = IntPtr.Zero, FullName = "" };
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
			HandleException(ex);
		}
	}

	private static unsafe MethodInfo? TryGetMethodInfo(Type InType, string InMethodName, ManagedType* InParameterTypes, int InParameterCount, BindingFlags InBindingFlags)
	{
		ReadOnlySpan<MethodInfo> methods = InType.GetMethods(InBindingFlags);

		MethodInfo? methodInfo = null;

		var parameterTypes = new ManagedType[InParameterCount];

		unsafe
		{
			fixed (ManagedType* parameterTypesPtr = parameterTypes)
			{
				ulong size = sizeof(ManagedType) * (ulong)InParameterCount;
				Buffer.MemoryCopy(InParameterTypes, parameterTypesPtr, size, size);
			}
		}

		var methodKey = new MethodKey(InType.FullName, InMethodName, parameterTypes, InParameterCount);

		if (!s_CachedMethods.TryGetValue(methodKey, out methodInfo))
		{
			methodInfo = TypeInterface.FindSuitableMethod(InMethodName, InParameterTypes, InParameterCount, methods);

			if (methodInfo == null)
			{
				LogMessage($"Failed to find method '{InMethodName}' for type {InType.FullName} with {InParameterCount} parameters.", MessageLevel.Error);
				return null;
			}

			s_CachedMethods.Add(methodKey, methodInfo);
		}

		return methodInfo;
	}

	[UnmanagedCallersOnly]
	private static unsafe void InvokeStaticMethod(Type* InType, NativeString InMethodName, IntPtr InParameters, ManagedType* InParameterTypes, int InParameterCount)
	{
		try
		{
			if (InType == null)
			{
				LogMessage($"Cannot invoke method {InMethodName} on a null type.", MessageLevel.Error);
				return;
			}

			var methodInfo = TryGetMethodInfo(*InType, InMethodName, InParameterTypes, InParameterCount, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static);
			var parameters = Marshalling.MarshalParameterArray(InParameters, InParameterCount, methodInfo);

			methodInfo.Invoke(null, parameters);
		}
		catch (Exception ex)
		{
			HandleException(ex);
		}
	}

	[UnmanagedCallersOnly]
	private static unsafe void InvokeStaticMethodRet(Type* InType, NativeString InMethodName, IntPtr InParameters, ManagedType* InParameterTypes, int InParameterCount, IntPtr InResultStorage)
	{
		try
		{
			if (InType == null)
			{
				LogMessage($"Cannot invoke method {InMethodName} on a null type.", MessageLevel.Error);
				return;
			}

			var methodInfo = TryGetMethodInfo(*InType, InMethodName, InParameterTypes, InParameterCount, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static);
			var methodParameters = Marshalling.MarshalParameterArray(InParameters, InParameterCount, methodInfo);

			object? value = methodInfo.Invoke(null, methodParameters);

			if (value == null)
				return;

			Marshalling.MarshalReturnValue(value, methodInfo.ReturnType, InResultStorage);
		}
		catch (Exception ex)
		{
			HandleException(ex);
		}
	}

	[UnmanagedCallersOnly]
	private static unsafe void InvokeMethod(IntPtr InObjectHandle, NativeString InMethodName, IntPtr InParameters, ManagedType* InParameterTypes, int InParameterCount)
	{
		try
		{
			var target = GCHandle.FromIntPtr(InObjectHandle).Target;

			if (target == null)
			{
				LogMessage($"Cannot invoke method {InMethodName} on a null type.", MessageLevel.Error);
				return;
			}

			var targetType = target.GetType();

			var methodInfo = TryGetMethodInfo(targetType, InMethodName, InParameterTypes, InParameterCount, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);
			var parameters = Marshalling.MarshalParameterArray(InParameters, InParameterCount, methodInfo);

			methodInfo.Invoke(target, parameters);
		}
		catch (Exception ex)
		{
			HandleException(ex);
		}
	}
	
	[UnmanagedCallersOnly]
	private static unsafe void InvokeMethodRet(IntPtr InObjectHandle, NativeString InMethodName, IntPtr InParameters, ManagedType* InParameterTypes, int InParameterCount, IntPtr InResultStorage)
	{
		try
		{
			var target = GCHandle.FromIntPtr(InObjectHandle).Target;

			if (target == null)
			{
				LogMessage($"Cannot invoke method {InMethodName} on object with handle {InObjectHandle}. Target was null.", MessageLevel.Error);
				return;
			}

			var targetType = target.GetType();

			var methodInfo = TryGetMethodInfo(targetType, InMethodName, InParameterTypes, InParameterCount, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);
			var methodParameters = Marshalling.MarshalParameterArray(InParameters, InParameterCount, methodInfo);
			
			object? value = methodInfo.Invoke(target, methodParameters);

			if (value == null)
				return;

			Marshalling.MarshalReturnValue(value, methodInfo.ReturnType, InResultStorage);
		}
		catch (Exception ex)
		{
			HandleException(ex);
		}
	}

	[UnmanagedCallersOnly]
	private static void SetFieldValue(IntPtr InTarget, NativeString InFieldName, IntPtr InValue)
	{
		try
		{
			var target = GCHandle.FromIntPtr(InTarget).Target;

			if (target == null)
			{
				LogMessage($"Cannot set value of field {InFieldName} on object with handle {InTarget}. Target was null.", MessageLevel.Error);
				return;
			}

			var targetType = target.GetType();
			var fieldInfo = targetType.GetField(InFieldName!, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);

			if (fieldInfo == null)
			{
				LogMessage($"Failed to find field '{InFieldName}' in type '{targetType.FullName}'.", MessageLevel.Error);
				return;
			}

			object? value = Marshalling.MarshalPointer(InValue, fieldInfo.FieldType);
			fieldInfo.SetValue(target, value);
		}
		catch (Exception ex)
		{
			HandleException(ex);
		}
	}

	[UnmanagedCallersOnly]
	private static void GetFieldValue(IntPtr InTarget, NativeString InFieldName, IntPtr OutValue)
	{
		try
		{
			var target = GCHandle.FromIntPtr(InTarget).Target;

			if (target == null)
			{
				LogMessage($"Cannot get value of field {InFieldName} from object with handle {InTarget}. Target was null.", MessageLevel.Error);
				return;
			}

			var targetType = target.GetType();
			var fieldInfo = targetType.GetField(InFieldName!, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);

			if (fieldInfo == null)
			{
				LogMessage($"Failed to find field '{InFieldName}' in type '{targetType.FullName}'.", MessageLevel.Error);
				return;
			}

			Marshalling.MarshalReturnValue(fieldInfo.GetValue(target), fieldInfo.FieldType, OutValue);
		}
		catch (Exception ex)
		{
			HandleException(ex);
		}
	}

	[UnmanagedCallersOnly]
	private static void SetPropertyValue(IntPtr InTarget, NativeString InPropertyName, IntPtr InValue)
	{
		try
		{
			var target = GCHandle.FromIntPtr(InTarget).Target;

			if (target == null)
			{
				LogMessage($"Cannot set value of property {InPropertyName} on object with handle {InTarget}. Target was null.", MessageLevel.Error);
				return;
			}

			var targetType = target.GetType();
			var propertyInfo = targetType.GetProperty(InPropertyName!, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);
		
			if (propertyInfo == null)
			{
				LogMessage($"Failed to find property '{InPropertyName}' in type '{targetType.FullName}'", MessageLevel.Error);
				return;
			}

			if (propertyInfo.SetMethod == null)
			{
				LogMessage($"Cannot set value of property '{InPropertyName}'. No setter was found.", MessageLevel.Error);
				return;
			}

			object? value = Marshalling.MarshalPointer(InValue, propertyInfo.PropertyType);
			propertyInfo.SetValue(target, value);
		}
		catch (Exception ex)
		{
			HandleException(ex);
		}
	}

	[UnmanagedCallersOnly]
	private static void GetPropertyValue(IntPtr InTarget, NativeString InPropertyName, IntPtr OutValue)
	{
		try
		{
			var target = GCHandle.FromIntPtr(InTarget).Target;

			if (target == null)
			{
				LogMessage($"Cannot get value of property '{InPropertyName}' from object with handle {InTarget}. Target was null.", MessageLevel.Error);
				return;
			}

			var targetType = target.GetType();
			var propertyInfo = targetType.GetProperty(InPropertyName!, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);

			if (propertyInfo == null)
			{
				LogMessage($"Failed to find property '{InPropertyName}' in type '{targetType.FullName}'.", MessageLevel.Error);
				return;
			}

			if (propertyInfo.GetMethod == null)
			{
				LogMessage($"Cannot get value of property '{InPropertyName}'. No getter was found.", MessageLevel.Error);
				return;
			}

			Marshalling.MarshalReturnValue(propertyInfo.GetValue(target), propertyInfo.PropertyType, OutValue);
		}
		catch (Exception ex)
		{
			HandleException(ex);
		}
	}
}
