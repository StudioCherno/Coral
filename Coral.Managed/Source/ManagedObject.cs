using Coral.Managed.Interop;

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Coral.Managed;

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
		public readonly string Name;
		public readonly ManagedType[] Types;
		public readonly int ParameterCount;

		public MethodKey(string InName, ManagedType[] InTypes, int InParameterCount)
		{
			Name = InName;
			Types = InTypes;
			ParameterCount = InParameterCount;
		}

		public override bool Equals([NotNullWhen(true)] object? obj) => obj is MethodKey other && Equals(other);

		bool IEquatable<MethodKey>.Equals(MethodKey other)
		{
			if (Name != other.Name)
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
				throw new TypeNotFoundException($"Failed to find type with name {InTypeName}");
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
				throw new MissingMethodException($"No suitable constructor found for type {type}");

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
				return new() { Handle = IntPtr.Zero, FullName = "" }; // TODO(Peter): Exception

			var handle = GCHandle.Alloc(result, InWeakRef ? GCHandleType.Weak : GCHandleType.Normal);
			return new()
			{
				Handle = GCHandle.ToIntPtr(handle),
				FullName = type.FullName
			};
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
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
			ManagedHost.HandleException(ex);
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
				throw new ArgumentNullException(nameof(InObjectHandle), $"Trying to invoke method {InMethodName} on a null object.");
			}

			var targetType = target.GetType();

			ReadOnlySpan<MethodInfo> methods = targetType.GetMethods();

			// NOTE(Peter): Consider caching this if it becomes to performance heavy
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

			var methodKey = new MethodKey(InMethodName, parameterTypes, InParameterCount);

			if (!s_CachedMethods.TryGetValue(methodKey, out methodInfo))
			{
				methodInfo = TypeInterface.FindSuitableMethod(InMethodName, InParameterTypes, InParameterCount, methods);

				if (methodInfo == null)
					throw new MissingMethodException($"Method {InMethodName} wasn't found.");

				s_CachedMethods.Add(methodKey, methodInfo);
			}

			var parameters = Marshalling.MarshalParameterArray(InParameters, InParameterCount, methodInfo);

			methodInfo.Invoke(target, parameters);
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
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
				throw new ArgumentNullException(nameof(InObjectHandle), $"Trying to invoke method {InMethodName} on a null object.");
			}

			var targetType = target.GetType();

			ReadOnlySpan<MethodInfo> methods = targetType.GetMethods();

			// NOTE(Peter): Consider caching this if it becomes to performance heavy
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

			var methodKey = new MethodKey(InMethodName, parameterTypes, InParameterCount);

			if (!s_CachedMethods.TryGetValue(methodKey, out methodInfo))
			{
				methodInfo = TypeInterface.FindSuitableMethod(InMethodName, InParameterTypes, InParameterCount, methods);

				if (methodInfo == null)
					throw new MissingMethodException($"Method {InMethodName} wasn't found.");

				s_CachedMethods.Add(methodKey, methodInfo);
			}

			var methodParameters = Marshalling.MarshalParameterArray(InParameters, InParameterCount, methodInfo);
			
			object? value = methodInfo.Invoke(target, methodParameters);

			if (value == null)
				return;

			Marshalling.MarshalReturnValue(value, methodInfo.ReturnType, InResultStorage);
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
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
				throw new ArgumentNullException(nameof(InTarget), $"Tried setting value of field {InFieldName} on a null object.");
			}

			var targetType = target.GetType();
			var fieldInfo = targetType.GetField(InFieldName!, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);

			if (fieldInfo == null)
			{
				throw new MissingFieldException($"Failed to find field named {InFieldName} in {targetType}");
			}

			object? value;

			if (fieldInfo.FieldType.IsPointer || fieldInfo.FieldType == typeof(IntPtr))
			{
				value = Marshalling.MarshalPointer(Marshal.ReadIntPtr(InValue), fieldInfo.FieldType);
			}
			else if (fieldInfo.FieldType.IsSZArray)
			{
				value = Marshalling.MarshalArray(InValue, fieldInfo.FieldType.GetElementType());
			}
			else
			{
				value = Marshalling.MarshalPointer(InValue, fieldInfo.FieldType);
			}

			fieldInfo.SetValue(target, value);
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
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
				throw new ArgumentNullException(nameof(InTarget), $"Tried getting value of field {InFieldName} on a null object.");
			}

			var targetType = target.GetType();
			var fieldInfo = targetType.GetField(InFieldName!, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);

			if (fieldInfo == null)
			{
				throw new MissingFieldException($"Failed to find field named {InFieldName} in {targetType}");
			}

			Marshalling.MarshalReturnValue(fieldInfo.GetValue(target), fieldInfo.FieldType, OutValue);
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
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
				throw new ArgumentNullException(nameof(InTarget), $"Tried setting value of property {InPropertyName} on a null object.");
			}

			var targetType = target.GetType();
			var propertyInfo = targetType.GetProperty(InPropertyName!, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);
		
			if (propertyInfo == null)
			{
				throw new MissingMemberException($"Failed to find property named {InPropertyName} in type {targetType}");
			}

			if (propertyInfo.SetMethod == null)
			{
				throw new InvalidOperationException($"Attempting to set value of property {InPropertyName} with no setter.");
			}

			object? value;
		
			if (propertyInfo.PropertyType.IsPointer || propertyInfo.PropertyType == typeof(IntPtr))
			{
				value = Marshalling.MarshalPointer(Marshal.ReadIntPtr(InValue), propertyInfo.PropertyType);
			}
			else if (propertyInfo.PropertyType.IsSZArray)
			{
				value = Marshalling.MarshalArray(InValue, propertyInfo.PropertyType.GetElementType());
			}
			else
			{
				value = Marshalling.MarshalPointer(InValue, propertyInfo.PropertyType);
			}
		
			propertyInfo.SetValue(target, value);
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
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
				throw new ArgumentNullException(nameof(InTarget), $"Tried getting value of property {InPropertyName} on a null object.");
			}

			var targetType = target.GetType();
			var propertyInfo = targetType.GetProperty(InPropertyName!, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);

			if (propertyInfo == null)
			{
				throw new MissingMemberException($"Failed to find property named {InPropertyName} in type {targetType}");
			}

			if (propertyInfo.GetMethod == null)
			{
				throw new InvalidOperationException($"Attempting to get value of property {InPropertyName} with no getter.");
			}

			Marshalling.MarshalReturnValue(propertyInfo.GetValue(target), propertyInfo.PropertyType, OutValue);
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
		}
	}
}
