using Coral.Managed.Interop;

using System;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Coral.Managed;

internal static class ManagedObject
{
	[StructLayout(LayoutKind.Sequential)]
	public readonly struct ObjectCreateInfo
	{
		public readonly UnmanagedString TypeName;
		public readonly bool IsWeakRef;
		public readonly UnmanagedArray Parameters;
	}

	private struct ObjectData
	{
		public IntPtr Handle;
		public UnmanagedString FullName;
	}
	
	[UnmanagedCallersOnly]
	private static unsafe ObjectData CreateObject(ObjectCreateInfo* InCreateInfo)
	{
		try
		{
			var type = TypeHelper.FindType(InCreateInfo->TypeName);

			if (type == null)
			{
				throw new TypeNotFoundException($"Failed to find type with name {InCreateInfo->TypeName}");
			}

			ConstructorInfo? constructor = null;
			
			var currentType = type;
			while (currentType != null)
			{
				ReadOnlySpan<ConstructorInfo> constructors = currentType.GetConstructors(BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance);
				foreach (var constructorInfo in constructors)
				{
					if (constructorInfo.GetParameters().Length != InCreateInfo->Parameters.Length)
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

			var parameters = Marshalling.MarshalParameterArray(InCreateInfo->Parameters, constructor);

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
				return new() { Handle = IntPtr.Zero, FullName = UnmanagedString.Null() }; // TODO(Peter): Exception

			var handle = GCHandle.Alloc(result, InCreateInfo->IsWeakRef ? GCHandleType.Weak : GCHandleType.Normal);
			return new()
			{
				Handle = GCHandle.ToIntPtr(handle),
				FullName = UnmanagedString.FromString(type.FullName)
			};
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
			return new() { Handle = IntPtr.Zero, FullName = UnmanagedString.Null() };
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
	private static void InvokeMethod(IntPtr InObjectHandle, UnmanagedString InMethodName, UnmanagedArray InParameters)
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
				if (mi.Name != InMethodName || mi.GetParameters().Length != InParameters.Length)
					continue;

				methodInfo = mi;
				break;
			}

			if (methodInfo == null)
			{
				throw new MissingMethodException($"Method {InMethodName} wasn't found.");
			}

			var parameters = Marshalling.MarshalParameterArray(InParameters, methodInfo);

			methodInfo.Invoke(target, parameters);
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
		}
	}
	
	[UnmanagedCallersOnly]
	private static void InvokeMethodRet(IntPtr InObjectHandle, UnmanagedString InMethodName, UnmanagedArray InParameters, IntPtr InResultStorage)
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
				if (mi.Name != InMethodName || mi.GetParameters().Length != InParameters.Length)
					continue;
				
				methodInfo = mi;
				break;
			}

			if (methodInfo == null)
			{
				throw new MissingMethodException($"Method {InMethodName} wasn't found.");
			}

			var methodParameters = Marshalling.MarshalParameterArray(InParameters, methodInfo);
			
			object? value = methodInfo.Invoke(target, methodParameters);

			if (value == null)
				return;

			var returnType = methodInfo.ReturnType;
			
			if (value is string s)
			{
				var nativeString = UnmanagedString.FromString(s);
				Marshal.WriteIntPtr(InResultStorage, nativeString.m_NativeString);
			}
			else if (returnType.IsPointer)
			{
				unsafe
				{
					void* valuePointer = Pointer.Unbox(value);
					Buffer.MemoryCopy(&valuePointer, InResultStorage.ToPointer(), IntPtr.Size, IntPtr.Size);
				}
			}
			else if (returnType.IsSZArray)
			{
				Marshalling.CopyArrayToBuffer(InResultStorage, value as Array, returnType.GetElementType());
			}
			else
			{
				var valueSize = Marshal.SizeOf(returnType);
				var handle = GCHandle.Alloc(value, GCHandleType.Pinned);

				unsafe
				{
					Buffer.MemoryCopy(handle.AddrOfPinnedObject().ToPointer(), InResultStorage.ToPointer(), valueSize, valueSize);
				}
				
				handle.Free();
			}
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
		}
	}

	[UnmanagedCallersOnly]
	private static void SetFieldValue(IntPtr InTarget, UnmanagedString InFieldName, IntPtr InValue)
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

			if (fieldInfo.FieldType == typeof(string) || fieldInfo.FieldType.IsPointer || fieldInfo.FieldType == typeof(IntPtr))
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
	private static void GetFieldValue(IntPtr InTarget, UnmanagedString InFieldName, IntPtr OutValue)
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

			var value = fieldInfo.GetValue(target);

			if (fieldInfo.FieldType.IsSZArray)
			{
				var array = value as Array;
				var elementType = fieldInfo.FieldType.GetElementType();
				Marshalling.CopyArrayToBuffer(OutValue, array, elementType);
			}
			else if (value is string s)
			{
				var nativeString = UnmanagedString.FromString(s);
				Marshal.WriteIntPtr(OutValue, nativeString.m_NativeString);
			}
			else if (fieldInfo.FieldType.IsPointer)
			{
				unsafe
				{
					if (value == null)
					{
						Marshal.WriteIntPtr(OutValue, IntPtr.Zero);
					}
					else
					{
						void* valuePointer = Pointer.Unbox(value);
						Buffer.MemoryCopy(&valuePointer, OutValue.ToPointer(), IntPtr.Size, IntPtr.Size);
					}

				}
			}
			else
			{
				var valueSize = Marshal.SizeOf(fieldInfo.FieldType);
				var handle = GCHandle.Alloc(value, GCHandleType.Pinned);

				unsafe
				{
					Buffer.MemoryCopy(handle.AddrOfPinnedObject().ToPointer(), OutValue.ToPointer(), valueSize, valueSize);
				}

				handle.Free();
			}
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
		}
	}

	[UnmanagedCallersOnly]
	private static void SetPropertyValue(IntPtr InTarget, UnmanagedString InPropertyName, IntPtr InValue)
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
		
			if (propertyInfo.PropertyType == typeof(string) || propertyInfo.PropertyType.IsPointer || propertyInfo.PropertyType == typeof(IntPtr))
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
	private static void GetPropertyValue(IntPtr InTarget, UnmanagedString InPropertyName, IntPtr OutValue)
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

			var value = propertyInfo.GetValue(target);

			if (propertyInfo.PropertyType.IsSZArray)
			{
				var array = value as Array;
				var elementType = propertyInfo.PropertyType.GetElementType();
				Marshalling.CopyArrayToBuffer(OutValue, array, elementType);
			}
			else if (value is string s)
			{
				var nativeString = UnmanagedString.FromString(s);
				Marshal.WriteIntPtr(OutValue, nativeString.m_NativeString);
			}
			else if (propertyInfo.PropertyType.IsPointer)
			{
				unsafe
				{
					if (value == null)
					{
						Marshal.WriteIntPtr(OutValue, IntPtr.Zero);
					}
					else
					{
						void* valuePointer = Pointer.Unbox(value);
						Buffer.MemoryCopy(&valuePointer, OutValue.ToPointer(), IntPtr.Size, IntPtr.Size);
					}
				}
			}
			else
			{
				var valueSize = Marshal.SizeOf(propertyInfo.PropertyType);
				var handle = GCHandle.Alloc(value, GCHandleType.Pinned);

				unsafe
				{
					Buffer.MemoryCopy(handle.AddrOfPinnedObject().ToPointer(), OutValue.ToPointer(), valueSize, valueSize);
				}

				handle.Free();
			}
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
		}
	}
}
