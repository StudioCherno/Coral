using Coral.Managed.Interop;

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Coral.Managed;

internal static class TypeInterface
{

	internal static Type? FindType(string? InTypeName)
	{
		var type = Type.GetType(InTypeName!,
			(name) => AssemblyLoader.ResolveAssembly(null, name),
			(assembly, name, ignore) => assembly != null ? assembly.GetType(name, false, ignore) : Type.GetType(name, false, ignore)
		);

		return type;
	}

	internal static object? CreateInstance(Type InType, params object?[]? InArguments)
	{
		return InType.Assembly.CreateInstance(InType.FullName ?? string.Empty, false, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance, null, InArguments!, null, null);
	}

	private static Dictionary<Type, ManagedType> s_TypeConverters = new()
	{
		{ typeof(sbyte), ManagedType.SByte },
		{ typeof(byte), ManagedType.Byte },
		{ typeof(short), ManagedType.Short },
		{ typeof(ushort), ManagedType.UShort },
		{ typeof(int), ManagedType.Int },
		{ typeof(uint), ManagedType.UInt },
		{ typeof(long), ManagedType.Long },
		{ typeof(ulong), ManagedType.ULong },
		{ typeof(float), ManagedType.Float },
		{ typeof(double), ManagedType.Double },
		{ typeof(bool), ManagedType.Bool },
		{ typeof(Bool32), ManagedType.Bool },
	};

	internal static unsafe T? FindSuitableMethod<T>(string? InMethodName, ManagedType* InParameterTypes, int InParameterCount, ReadOnlySpan<T> InMethods) where T : MethodBase
	{
		if (InMethodName == null)
			return null;

		T? result = null;

		foreach (var methodInfo in InMethods)
		{
			var methodParams = methodInfo.GetParameters();

			if (methodParams.Length != InParameterCount)
				continue;

			// Check if the method name matches the signature of methodInfo, if so we ignore the automatic type checking
			if (InMethodName == methodInfo.ToString())
			{
				result = methodInfo;
				break;
			}

			if (methodInfo.Name != InMethodName)
				continue;

			int matchingTypes = 0;

			for (int i = 0; i < methodParams.Length; i++)
			{
				ManagedType paramType;

				if (methodParams[i].ParameterType.IsPointer || methodParams[i].ParameterType == typeof(IntPtr))
				{
					paramType = ManagedType.Pointer;
				}
				else if (!s_TypeConverters.TryGetValue(methodParams[i].ParameterType, out paramType))
				{
					paramType = ManagedType.Unknown;
				}

				if (paramType == InParameterTypes[i])
				{
					matchingTypes++;
				}
			}

			if (matchingTypes == InParameterCount)
			{
				result = methodInfo;
				break;
			}
		}

		return result;
	}

	[UnmanagedCallersOnly]
	private static unsafe void GetAssemblyTypes(int InAssemblyId, Type* OutTypes, int* OutTypeCount)
	{
		try
		{
			if (!AssemblyLoader.TryGetAssembly(InAssemblyId, out var assembly))
			{
				return;
			}

			if (assembly == null)
				return;

			ReadOnlySpan<Type> assemblyTypes = assembly.GetTypes();

			if (OutTypeCount != null)
				*OutTypeCount = assemblyTypes.Length;

			if (OutTypes == null)
				return;

			for (int i = 0; i < assemblyTypes.Length; i++)
				OutTypes[i] = assemblyTypes[i];
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
		}
	}

	[UnmanagedCallersOnly]
	private static unsafe void GetTypeId(NativeString InName, Type* OutType)
	{
		try
		{
			var type = FindType(InName);

			if (type == null)
				return;

			*OutType = type;
		}
		catch (Exception e)
		{
			ManagedHost.HandleException(e);
		}
	}

	[UnmanagedCallersOnly]
	private static unsafe NativeString GetFullTypeName(Type* InType)
	{
		try
		{
			if (InType == null)
				return NativeString.Null();

			return InType->FullName;
		}
		catch (Exception e)
		{
			ManagedHost.HandleException(e);
			return NativeString.Null();
		}
	}

	[UnmanagedCallersOnly]
	private static unsafe NativeString GetAssemblyQualifiedName(Type* InType)
	{
		try
		{
			if (InType == null)
				return NativeString.Null();

			return InType->AssemblyQualifiedName;
		}
		catch (Exception e)
		{
			ManagedHost.HandleException(e);
			return NativeString.Null();
		}
	}

	[UnmanagedCallersOnly]
	private static unsafe void GetBaseType(Type* InType, Type* OutBaseType)
	{
		try
		{
			if (InType == null || OutBaseType == null)
				return;

			*OutBaseType = InType->BaseType;
		}
		catch (Exception e)
		{
			ManagedHost.HandleException(e);
		}
	}

	[UnmanagedCallersOnly]
	private static unsafe Bool32 IsTypeAssignableTo(Type* InType0, Type* InType1)
	{
		try
		{
			if (InType0 == null || InType1 == null)
				return false;

			return InType0->IsAssignableTo(*InType1);
		}
		catch (Exception e)
		{
			ManagedHost.HandleException(e);
			return false;
		}
	}

	[UnmanagedCallersOnly]
	private static unsafe Bool32 IsTypeAssignableFrom(Type* InType0, Type* InType1)
	{
		try
		{
			if (InType0 == null || InType1 == null)
				return false;

			return InType0->IsAssignableFrom(*InType1);
		}
		catch (Exception e)
		{
			ManagedHost.HandleException(e);
			return false;
		}
	}

	[UnmanagedCallersOnly]
	private static unsafe void GetTypeMethods(Type* InType, MethodInfo* InMethodArray, int* InMethodCount)
	{
		try
		{
			if (InType == null)
				return;

			ReadOnlySpan<MethodInfo> methods = InType->GetMethods(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Static);

			if (methods == null || methods.Length == 0)
			{
				*InMethodCount = 0;
				return;
			}

			*InMethodCount = methods.Length;

			if (InMethodArray == null)
				return;

			for (int i = 0; i < methods.Length; i++)
				InMethodArray[i] = methods[i];
		}
		catch (Exception e)
		{
			ManagedHost.HandleException(e);
		}
	}

	[UnmanagedCallersOnly]
	private static unsafe void GetTypeFields(Type* InType, FieldInfo* InFieldArray, int* InFieldCount)
	{
		try
		{
			if (InType == null)
				return;

			ReadOnlySpan<FieldInfo> fields = InType->GetFields(BindingFlags.Instance | BindingFlags.Static | BindingFlags.NonPublic | BindingFlags.Public);

			if (fields == null || fields.Length == 0)
			{
				*InFieldCount = 0;
				return;
			}

			*InFieldCount = fields.Length;

			if (InFieldArray == null)
				return;

			for (int i = 0; i < fields.Length; i++)
				InFieldArray[i] = fields[i];
		}
		catch (Exception e)
		{
			ManagedHost.HandleException(e);
		}
	}

	[UnmanagedCallersOnly]
	private static unsafe void GetTypeProperties(Type* InType, PropertyInfo* InPropertyArray, int* InPropertyCount)
	{
		try
		{
			if (InType == null)
				return;

			ReadOnlySpan<PropertyInfo> properties = InType->GetProperties(BindingFlags.Instance | BindingFlags.Static | BindingFlags.NonPublic | BindingFlags.Public);

			if (properties == null || properties.Length == 0)
			{
				*InPropertyCount = 0;
				return;
			}

			*InPropertyCount = properties.Length;

			if (InPropertyArray == null)
				return;

			for (int i = 0; i < properties.Length; i++)
				InPropertyArray[i] = properties[i];
		}
		catch (Exception e)
		{
			ManagedHost.HandleException(e);
		}
	}

	[UnmanagedCallersOnly]
	private static unsafe void GetTypeAttributes(Type* InType, Attribute* OutAttributes, int* OutAttributesCount)
	{
		try
		{
			if (InType == null)
				return;

			var attributes = InType->GetCustomAttributes().ToImmutableArray();

			if (attributes.Length == 0)
			{
				*OutAttributesCount = 0;
				return;
			}

			*OutAttributesCount = attributes.Length;

			if (OutAttributes == null)
				return;

			for (int i = 0; i < attributes.Length; i++)
				OutAttributes[i] = attributes[i];
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
		}
	}

	// TODO(Peter): Refactor this to GetMemberInfoName (should work for all types of members)
	[UnmanagedCallersOnly]
	private static unsafe NativeString GetMethodInfoName(MethodInfo* InMethodInfo)
	{
		try
		{
			if (InMethodInfo == null)
				return NativeString.Null();

			return InMethodInfo->Name;
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
			return NativeString.Null();
		}
	}

	[UnmanagedCallersOnly]
	private static unsafe void GetMethodInfoReturnType(MethodInfo* InMethodInfo, Type* OutReturnType)
	{
		try
		{
			if (InMethodInfo == null || OutReturnType == null)
				return;

			*OutReturnType = InMethodInfo->ReturnType;
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
		}
	}

	[UnmanagedCallersOnly]
	private static unsafe void GetMethodInfoParameterTypes(MethodInfo* InMethodInfo, Type* OutParameterTypes, int* OutParameterCount)
	{
		try
		{
			if (InMethodInfo == null)
				return;

			ReadOnlySpan<ParameterInfo> parameters = InMethodInfo->GetParameters();

			if (parameters == null || parameters.Length == 0)
			{
				*OutParameterCount = 0;
				return;
			}

			*OutParameterCount = parameters.Length;

			if (OutParameterTypes == null)
				return;

			for (int i = 0; i < parameters.Length; i++)
				OutParameterTypes[i] = parameters[i].ParameterType;
		}
		catch (Exception e)
		{
			ManagedHost.HandleException(e);
		}
	}

	[UnmanagedCallersOnly]
	private static unsafe void GetMethodInfoAttributes(MethodInfo* InMethodInfo, Attribute* OutAttributes, int* OutAttributesCount)
	{
		try
		{
			if (InMethodInfo == null)
				return;

			var attributes = InMethodInfo->GetCustomAttributes().ToImmutableArray();

			if (attributes.Length == 0)
			{
				*OutAttributesCount = 0;
				return;
			}

			*OutAttributesCount = attributes.Length;

			if (OutAttributes == null)
				return;

			for (int i = 0; i < attributes.Length; i++)
				OutAttributes[i] = attributes[i];
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
		}
	}

	internal enum TypeAccessibility
	{
		Public,
		Private,
		Protected,
		Internal,
		ProtectedPublic,
		PrivateProtected
	}

	private static TypeAccessibility GetTypeAccessibility(FieldInfo InFieldInfo)
	{
		if (InFieldInfo.IsPublic) return TypeAccessibility.Public;
		if (InFieldInfo.IsPrivate) return TypeAccessibility.Private;
		if (InFieldInfo.IsFamily) return TypeAccessibility.Protected;
		if (InFieldInfo.IsAssembly) return TypeAccessibility.Internal;
		if (InFieldInfo.IsFamilyOrAssembly) return TypeAccessibility.ProtectedPublic;
		if (InFieldInfo.IsFamilyAndAssembly) return TypeAccessibility.PrivateProtected;
		return TypeAccessibility.Public;
	}

	private static TypeAccessibility GetTypeAccessibility(MethodInfo InMethodInfo)
	{
		if (InMethodInfo.IsPublic) return TypeAccessibility.Public;
		if (InMethodInfo.IsPrivate) return TypeAccessibility.Private;
		if (InMethodInfo.IsFamily) return TypeAccessibility.Protected;
		if (InMethodInfo.IsAssembly) return TypeAccessibility.Internal;
		if (InMethodInfo.IsFamilyOrAssembly) return TypeAccessibility.ProtectedPublic;
		if (InMethodInfo.IsFamilyAndAssembly) return TypeAccessibility.PrivateProtected;
		return TypeAccessibility.Public;
	}

	[UnmanagedCallersOnly]
	private static unsafe TypeAccessibility GetMethodInfoAccessibility(MethodInfo* InMethodInfo)
	{
		try
		{
			return GetTypeAccessibility(*InMethodInfo);
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
			return TypeAccessibility.Public;
		}
	}

	[UnmanagedCallersOnly]
	private static unsafe NativeString GetFieldInfoName(FieldInfo* InFieldInfo)
	{
		try
		{
			if (InFieldInfo == null)
				return NativeString.Null();

			return InFieldInfo->Name;
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
			return NativeString.Null();
		}
	}

	[UnmanagedCallersOnly]
	private static unsafe void GetFieldInfoType(FieldInfo* InFieldInfo, Type* OutFieldType)
	{
		try
		{
			if (InFieldInfo == null)
				return;

			*OutFieldType = InFieldInfo->FieldType;
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
		}
	}

	[UnmanagedCallersOnly]
	private static unsafe TypeAccessibility GetFieldInfoAccessibility(FieldInfo* InFieldInfo)
	{
		try
		{
			return GetTypeAccessibility(*InFieldInfo);
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
			return TypeAccessibility.Public;
		}
	}

	[UnmanagedCallersOnly]
	private static unsafe void GetFieldInfoAttributes(FieldInfo* InFieldInfo, Attribute* OutAttributes, int* OutAttributesCount)
	{
		try
		{
			if (InFieldInfo == null)
				return;

			var attributes = InFieldInfo->GetCustomAttributes().ToImmutableArray();

			if (attributes.Length == 0)
			{
				*OutAttributesCount = 0;
				return;
			}

			*OutAttributesCount = attributes.Length;

			if (OutAttributes == null)
				return;

			for (int i = 0; i < attributes.Length; i++)
				OutAttributes[i] = attributes[i];
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
		}
	}

	[UnmanagedCallersOnly]
	private static unsafe NativeString GetPropertyInfoName(PropertyInfo* InPropertyInfo)
	{
		try
		{
			if (InPropertyInfo == null)
				return NativeString.Null();

			return InPropertyInfo->Name;
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
			return NativeString.Null();
		}
	}

	[UnmanagedCallersOnly]
	private static unsafe void GetPropertyInfoType(PropertyInfo* InPropertyInfo, Type* OutPropertyType)
	{
		try
		{
			if (InPropertyInfo == null)
				return;

			*OutPropertyType = InPropertyInfo->PropertyType;
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
		}
	}

	[UnmanagedCallersOnly]
	private static unsafe void GetPropertyInfoAttributes(PropertyInfo* InPropertyInfo, Attribute* OutAttributes, int* OutAttributesCount)
	{
		try
		{
			if (InPropertyInfo == null)
				return;

			var attributes = InPropertyInfo->GetCustomAttributes().ToImmutableArray();

			if (attributes.Length == 0)
			{
				*OutAttributesCount = 0;
				return;
			}

			*OutAttributesCount = attributes.Length;

			if (OutAttributes == null)
				return;

			for (int i = 0; i < attributes.Length; i++)
				OutAttributes[i] = attributes[i];
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
		}
	}

	[UnmanagedCallersOnly]
	private static unsafe void GetAttributeFieldValue(Attribute* InAttribute, NativeString InFieldName, IntPtr OutValue)
	{
		try
		{
			if (InAttribute == null)
				return;

			var targetType = InAttribute->GetType();
			var fieldInfo = targetType.GetField(InFieldName!, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);

			if (fieldInfo == null)
			{
				throw new MissingFieldException($"Failed to find field named {InFieldName} in {targetType}");
			}

			Marshalling.MarshalReturnValue(fieldInfo.GetValue(*InAttribute), fieldInfo.FieldType, OutValue);
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
		}
	}

	[UnmanagedCallersOnly]
	private static unsafe void GetAttributeType(Attribute* InAttribute, Type* OutType)
	{
		try
		{
			if (InAttribute == null)
				return;

			*OutType = InAttribute->GetType();
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
		}
	}
}
