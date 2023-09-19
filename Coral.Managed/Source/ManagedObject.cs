using Coral.Managed.Interop;

using System;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Coral.Managed;

internal static class ManagedObject
{
	private struct ObjectData
	{
		public IntPtr Handle;
		public NativeString FullName;
	}
	
	[UnmanagedCallersOnly]
	private static unsafe ObjectData CreateObject(NativeString InTypeName, Bool32 InWeakRef, IntPtr InParameters, int InParameterCount)
	{
		try
		{
			var type = TypeHelper.FindType(InTypeName);

			if (type == null)
			{
				throw new TypeNotFoundException($"Failed to find type with name {InTypeName}");
			}

			ConstructorInfo? constructor = null;
			
			var currentType = type;
			while (currentType != null)
			{
				ReadOnlySpan<ConstructorInfo> constructors = currentType.GetConstructors(BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance);
				foreach (var constructorInfo in constructors)
				{
					if (constructorInfo.GetParameters().Length != InParameterCount)
						continue;

					constructor = constructorInfo;
					break;
				}

				if (constructor != null)
					break;

				currentType = currentType.BaseType;
			}

			if (constructor == null)
			{
				throw new MissingMethodException($"No suitable constructor found for type {type}");
			}

			var parameters = Marshalling.MarshalParameterArray(InParameters, InParameterCount, constructor);

			object? result = null;

			if (currentType != type || parameters == null)
			{
				result = TypeHelper.CreateInstance(type);

				if (currentType != type)
					constructor.Invoke(result, parameters);
			}
			else
			{
				result = TypeHelper.CreateInstance(type, parameters);
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
	private static void InvokeMethod(IntPtr InObjectHandle, NativeString InMethodName, IntPtr InParameters, int InParameterCount)
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

			MethodInfo? methodInfo = null;
			foreach (var mi in methods)
			{
				// TODO(Peter): Check types
				if (mi.Name != InMethodName || mi.GetParameters().Length != InParameterCount)
					continue;

				methodInfo = mi;
				break;
			}

			if (methodInfo == null)
			{
				throw new MissingMethodException($"Method {InMethodName} wasn't found.");
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
	private static void InvokeMethodRet(IntPtr InObjectHandle, NativeString InMethodName, IntPtr InParameters, int InParameterCount, IntPtr InResultStorage)
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

			MethodInfo? methodInfo = null;
			foreach (var mi in methods)
			{
				if (mi.Name != InMethodName || mi.GetParameters().Length != InParameterCount)
					continue;
				
				methodInfo = mi;
				break;
			}

			if (methodInfo == null)
			{
				throw new MissingMethodException($"Method {InMethodName} wasn't found.");
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
