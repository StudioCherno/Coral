using Coral.Managed.Interop;

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Diagnostics;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Threading;

namespace Coral.Managed;

using static ManagedHost;

internal static class TypeInterface
{

	internal readonly static UniqueIdList<Type> s_CachedTypes = new();
	internal readonly static UniqueIdList<MethodInfo> s_CachedMethods = new();
	internal readonly static UniqueIdList<FieldInfo> s_CachedFields = new();
	internal readonly static UniqueIdList<PropertyInfo> s_CachedProperties = new();
	internal readonly static UniqueIdList<Attribute> s_CachedAttributes = new();

	internal static Type? FindType(string? InTypeName)
	{
		var type = Type.GetType(InTypeName!,
			(name) => AssemblyLoader.ResolveAssembly(null, name),
			(assembly, name, ignore) =>
			{
				return assembly != null ? assembly.GetType(name, false, ignore) : Type.GetType(name, false, ignore);
			}
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
	internal static unsafe void GetAssemblyTypes(int InAssemblyId, int* OutTypes, int* OutTypeCount)
	{
		try
		{
			if (!AssemblyLoader.TryGetAssembly(InAssemblyId, out var assembly))
			{
				LogMessage($"Couldn't get types for assembly '{InAssemblyId}', assembly not found.", MessageLevel.Error);
				return;
			}

			if (assembly == null)
			{
				LogMessage($"Couldn't get types for assembly '{InAssemblyId}', assembly was null.", MessageLevel.Error);
				return;
			}

			ReadOnlySpan<Type> assemblyTypes = assembly.GetTypes();

			if (OutTypeCount != null)
				*OutTypeCount = assemblyTypes.Length;

			if (OutTypes == null)
				return;

			for (int i = 0; i < assemblyTypes.Length; i++)
			{
				OutTypes[i] = s_CachedTypes.Add(assemblyTypes[i]);
			}
		}
		catch (Exception ex)
		{
			HandleException(ex);
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe void GetTypeId(NativeString InName, int* OutType)
	{
		try
		{
			var type = FindType(InName);

			if (type == null)
			{
				LogMessage($"Failed to find type with name '{InName}'.", MessageLevel.Error);
				return;
			}

			*OutType = s_CachedTypes.Add(type);
		}
		catch (Exception e)
		{
			HandleException(e);
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe NativeString GetFullTypeName(int InType)
	{
		try
		{
			if (!s_CachedTypes.TryGetValue(InType, out var type))
				return NativeString.Null();

			return type.FullName;
		}
		catch (Exception e)
		{
			HandleException(e);
			return NativeString.Null();
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe NativeString GetAssemblyQualifiedName(int InType)
	{
		try
		{
			if (!s_CachedTypes.TryGetValue(InType, out var type))
				return NativeString.Null();

			return type.AssemblyQualifiedName;
		}
		catch (Exception e)
		{
			HandleException(e);
			return NativeString.Null();
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe void GetBaseType(int InType, int* OutBaseType)
	{
		try
		{
			if (!s_CachedTypes.TryGetValue(InType, out var type) || OutBaseType == null)
				return;

			if (type.BaseType == null)
			{
				*OutBaseType = 0;
				return;
			}

			*OutBaseType = s_CachedTypes.Add(type.BaseType);
		}
		catch (Exception e)
		{
			HandleException(e);
		}
	}

	[UnmanagedCallersOnly]
	internal static int GetTypeSize(int InType)
	{
		try
		{
			if (!s_CachedTypes.TryGetValue(InType, out var type))
				return -1;

			return Marshal.SizeOf(type);
		}
		catch (Exception e)
		{
			HandleException(e);
			return -1;
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe Bool32 IsTypeSubclassOf(int InType0, int InType1)
	{
		try
		{
			if (!s_CachedTypes.TryGetValue(InType0, out var type0) || !s_CachedTypes.TryGetValue(InType1, out var type1))
				return false;

			return type0.IsSubclassOf(type1);
		}
		catch (Exception e)
		{
			HandleException(e);
			return false;
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe Bool32 IsTypeAssignableTo(int InType0, int InType1)
	{
		try
		{
			if (!s_CachedTypes.TryGetValue(InType0, out var type0) || !s_CachedTypes.TryGetValue(InType1, out var type1))
				return false;

			return type0.IsAssignableTo(type1);
		}
		catch (Exception e)
		{
			HandleException(e);
			return false;
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe Bool32 IsTypeAssignableFrom(int InType0, int InType1)
	{
		try
		{
			if (!s_CachedTypes.TryGetValue(InType0, out var type0) || !s_CachedTypes.TryGetValue(InType1, out var type1))
				return false;

			return type0.IsAssignableFrom(type1);
		}
		catch (Exception e)
		{
			HandleException(e);
			return false;
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe Bool32 IsTypeSZArray(int InTypeID)
	{
		try
		{
			if (!s_CachedTypes.TryGetValue(InTypeID, out var type))
				return false;

			if (type == null)
			{
				return false;
			}

			return type.IsSZArray;
		}
		catch (Exception e)
		{
			HandleException(e);
			return false;
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe void GetElementType(int InTypeID, int* OutElementTypeID)
	{
		try
		{
			if (!s_CachedTypes.TryGetValue(InTypeID, out var type))
				return;

			var elementType = type.GetElementType();

			if (elementType == null)
				*OutElementTypeID = 0;

			*OutElementTypeID = s_CachedTypes.Add(elementType);
		}
		catch (Exception e)
		{
			HandleException(e);
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe void GetTypeMethods(int InType, int* InMethodArray, int* InMethodCount)
	{
		try
		{
			if (!s_CachedTypes.TryGetValue(InType, out var type))
				return;

			ReadOnlySpan<MethodInfo> methods = type.GetMethods(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Static);

			if (methods == null || methods.Length == 0)
			{
				*InMethodCount = 0;
				return;
			}

			*InMethodCount = methods.Length;

			if (InMethodArray == null)
				return;

			for (int i = 0; i < methods.Length; i++)
			{
				InMethodArray[i] = s_CachedMethods.Add(methods[i]);
			}
		}
		catch (Exception e)
		{
			HandleException(e);
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe void GetTypeFields(int InType, int* InFieldArray, int* InFieldCount)
	{
		try
		{
			if (!s_CachedTypes.TryGetValue(InType, out var type))
				return;

			ReadOnlySpan<FieldInfo> fields = type.GetFields(BindingFlags.Instance | BindingFlags.Static | BindingFlags.NonPublic | BindingFlags.Public);

			if (fields == null || fields.Length == 0)
			{
				*InFieldCount = 0;
				return;
			}

			*InFieldCount = fields.Length;

			if (InFieldArray == null)
				return;

			for (int i = 0; i < fields.Length; i++)
			{
				InFieldArray[i] = s_CachedFields.Add(fields[i]);
			}
		}
		catch (Exception e)
		{
			HandleException(e);
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe void GetTypeProperties(int InType, int* InPropertyArray, int* InPropertyCount)
	{
		try
		{
			if (!s_CachedTypes.TryGetValue(InType, out var type))
				return;

			ReadOnlySpan<PropertyInfo> properties = type.GetProperties(BindingFlags.Instance | BindingFlags.Static | BindingFlags.NonPublic | BindingFlags.Public);

			if (properties == null || properties.Length == 0)
			{
				*InPropertyCount = 0;
				return;
			}

			*InPropertyCount = properties.Length;

			if (InPropertyArray == null)
				return;

			for (int i = 0; i < properties.Length; i++)
			{
				InPropertyArray[i] = s_CachedProperties.Add(properties[i]);
			}
		}
		catch (Exception e)
		{
			HandleException(e);
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe Bool32 HasTypeAttribute(int InType, int InAttributeType)
	{
		try
		{
			if (!s_CachedTypes.TryGetValue(InType, out var type) || !s_CachedTypes.TryGetValue(InAttributeType, out var attributeType))
				return false;

			return type.GetCustomAttribute(attributeType) != null;
		}
		catch (Exception ex)
		{
			HandleException(ex);
			return false;
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe void GetTypeAttributes(int InType, int* OutAttributes, int* OutAttributesCount)
	{
		try
		{
			if (!s_CachedTypes.TryGetValue(InType, out var type))
				return;

			var attributes = type.GetCustomAttributes().ToImmutableArray();

			if (attributes.Length == 0)
			{
				*OutAttributesCount = 0;
				return;
			}

			*OutAttributesCount = attributes.Length;

			if (OutAttributes == null)
				return;

			for (int i = 0; i < attributes.Length; i++)
			{
				var attribute = attributes[i];
				OutAttributes[i] = s_CachedAttributes.Add(attribute);
			}
		}
		catch (Exception ex)
		{
			HandleException(ex);
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe ManagedType GetTypeManagedType(int InType)
	{
		try
		{
			if (!s_CachedTypes.TryGetValue(InType, out var type))
				return ManagedType.Unknown;

			if (!s_TypeConverters.TryGetValue(type, out var managedType))
				managedType = ManagedType.Unknown;

			return managedType;
		}
		catch (Exception ex)
		{
			HandleException(ex);
			return ManagedType.Unknown;
		}
	}

	// TODO(Peter): Refactor this to GetMemberInfoName (should work for all types of members)
	[UnmanagedCallersOnly]
	internal static unsafe NativeString GetMethodInfoName(int InMethodInfo)
	{
		try
		{
			if (!s_CachedMethods.TryGetValue(InMethodInfo, out var methodInfo))
				return NativeString.Null();

			return methodInfo.Name;
		}
		catch (Exception ex)
		{
			HandleException(ex);
			return NativeString.Null();
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe void GetMethodInfoReturnType(int InMethodInfo, int* OutReturnType)
	{
		try
		{
			if (!s_CachedMethods.TryGetValue(InMethodInfo, out var methodInfo) || OutReturnType == null)
				return;

			*OutReturnType = s_CachedTypes.Add(methodInfo.ReturnType);
		}
		catch (Exception ex)
		{
			HandleException(ex);
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe void GetMethodInfoParameterTypes(int InMethodInfo, int* OutParameterTypes, int* OutParameterCount)
	{
		try
		{
			if (!s_CachedMethods.TryGetValue(InMethodInfo, out var methodInfo))
				return;

			ReadOnlySpan<ParameterInfo> parameters = methodInfo.GetParameters();

			if (parameters == null || parameters.Length == 0)
			{
				*OutParameterCount = 0;
				return;
			}

			*OutParameterCount = parameters.Length;

			if (OutParameterTypes == null)
				return;

			for (int i = 0; i < parameters.Length; i++)
			{
				OutParameterTypes[i] = s_CachedTypes.Add(parameters[i].ParameterType);
			}
		}
		catch (Exception e)
		{
			HandleException(e);
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe void GetMethodInfoAttributes(int InMethodInfo, int* OutAttributes, int* OutAttributesCount)
	{
		try
		{
			if (!s_CachedMethods.TryGetValue(InMethodInfo, out var methodInfo))
				return;

			var attributes = methodInfo.GetCustomAttributes().ToImmutableArray();

			if (attributes.Length == 0)
			{
				*OutAttributesCount = 0;
				return;
			}

			*OutAttributesCount = attributes.Length;

			if (OutAttributes == null)
				return;

			for (int i = 0; i < attributes.Length; i++)
			{
				OutAttributes[i] = s_CachedAttributes.Add(attributes[i]);
			}
		}
		catch (Exception ex)
		{
			HandleException(ex);
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
	internal static unsafe TypeAccessibility GetMethodInfoAccessibility(int InMethodInfo)
	{
		try
		{
			if (!s_CachedMethods.TryGetValue(InMethodInfo, out var methodInfo))
				return TypeAccessibility.Internal;

			return GetTypeAccessibility(methodInfo);
		}
		catch (Exception ex)
		{
			HandleException(ex);
			return TypeAccessibility.Public;
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe NativeString GetFieldInfoName(int InFieldInfo)
	{
		try
		{
			if (!s_CachedFields.TryGetValue(InFieldInfo, out var fieldInfo))
				return NativeString.Null();

			return fieldInfo.Name;
		}
		catch (Exception ex)
		{
			HandleException(ex);
			return NativeString.Null();
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe void GetFieldInfoType(int InFieldInfo, int* OutFieldType)
	{
		try
		{
			if (!s_CachedFields.TryGetValue(InFieldInfo, out var fieldInfo))
				return;

			*OutFieldType = s_CachedTypes.Add(fieldInfo.FieldType);
		}
		catch (Exception ex)
		{
			HandleException(ex);
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe TypeAccessibility GetFieldInfoAccessibility(int InFieldInfo)
	{
		try
		{
			if (!s_CachedFields.TryGetValue(InFieldInfo, out var fieldInfo))
				return TypeAccessibility.Public;

			return GetTypeAccessibility(fieldInfo);
		}
		catch (Exception ex)
		{
			HandleException(ex);
			return TypeAccessibility.Public;
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe void GetFieldInfoAttributes(int InFieldInfo, int* OutAttributes, int* OutAttributesCount)
	{
		try
		{
			if (!s_CachedFields.TryGetValue(InFieldInfo, out var fieldInfo))
				return;

			var attributes = fieldInfo.GetCustomAttributes().ToImmutableArray();

			if (attributes.Length == 0)
			{
				*OutAttributesCount = 0;
				return;
			}

			*OutAttributesCount = attributes.Length;

			if (OutAttributes == null)
				return;

			for (int i = 0; i < attributes.Length; i++)
			{
				OutAttributes[i] = s_CachedAttributes.Add(attributes[i]);
			}
		}
		catch (Exception ex)
		{
			HandleException(ex);
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe NativeString GetPropertyInfoName(int InPropertyInfo)
	{
		try
		{
			if (!s_CachedProperties.TryGetValue(InPropertyInfo, out var propertyInfo))
				return NativeString.Null();

			return propertyInfo.Name;
		}
		catch (Exception ex)
		{
			HandleException(ex);
			return NativeString.Null();
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe void GetPropertyInfoType(int InPropertyInfo, int* OutPropertyType)
	{
		try
		{
			if (!s_CachedProperties.TryGetValue(InPropertyInfo, out var propertyInfo))
				return;

			*OutPropertyType = s_CachedTypes.Add(propertyInfo.PropertyType);
		}
		catch (Exception ex)
		{
			HandleException(ex);
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe void GetPropertyInfoAttributes(int InPropertyInfo, int* OutAttributes, int* OutAttributesCount)
	{
		try
		{
			if (!s_CachedProperties.TryGetValue(InPropertyInfo, out var propertyInfo))
				return;

			var attributes = propertyInfo.GetCustomAttributes().ToImmutableArray();

			if (attributes.Length == 0)
			{
				*OutAttributesCount = 0;
				return;
			}

			*OutAttributesCount = attributes.Length;

			if (OutAttributes == null)
				return;

			for (int i = 0; i < attributes.Length; i++)
			{
				OutAttributes[i] = s_CachedAttributes.Add(attributes[i]);
			}
		}
		catch (Exception ex)
		{
			HandleException(ex);
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe void GetAttributeFieldValue(int InAttribute, NativeString InFieldName, IntPtr OutValue)
	{
		try
		{
			if (!s_CachedAttributes.TryGetValue(InAttribute, out var attribute))
				return;

			var targetType = attribute.GetType();
			var fieldInfo = targetType.GetField(InFieldName!, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);

			if (fieldInfo == null)
			{
				LogMessage($"Failed to find field with name '{InFieldName}' in attribute {targetType.FullName}.", MessageLevel.Error);
				return;
			}

			Marshalling.MarshalReturnValue(attribute, fieldInfo.GetValue(attribute), fieldInfo, OutValue);
		}
		catch (Exception ex)
		{
			HandleException(ex);
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe void GetAttributeType(int InAttribute, int* OutType)
	{
		try
		{
			if (!s_CachedAttributes.TryGetValue(InAttribute, out var attribute))
				return;

			*OutType = s_CachedTypes.Add(attribute.GetType());
		}
		catch (Exception ex)
		{
			HandleException(ex);
		}
	}
}
