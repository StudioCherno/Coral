using Coral.Managed.Interop;

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Diagnostics;
using System.Globalization;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Runtime.Loader;

namespace Coral.Managed;

using static ManagedHost;

internal static class TypeInterface
{

	internal readonly static UniqueIdList<Type> s_CachedTypes = new();
	internal readonly static UniqueIdList<MethodInfo> s_CachedMethods = new();
	internal readonly static UniqueIdList<FieldInfo> s_CachedFields = new();
	internal readonly static UniqueIdList<PropertyInfo> s_CachedProperties = new();
	internal readonly static UniqueIdList<ConstructorInfo> s_CachedConstructors = new();
	internal readonly static UniqueIdList<EventInfo> s_CachedEvents = new();
	internal readonly static UniqueIdList<Attribute> s_CachedAttributes = new();

	internal static Type? FindType(int InAssemblyLoadContextId, string? InTypeName)
	{
		var type = Type.GetType(InTypeName!,
			(name) =>
			{
				AssemblyLoader.s_AssemblyContexts.TryGetValue(InAssemblyLoadContextId, out AssemblyLoadContext? alc);

				return AssemblyLoader.ResolveAssembly(alc, name);
			},
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
		{ typeof(Bool32), ManagedType.Bool },
		{ typeof(bool), ManagedType.Bool },
		{ typeof(NativeString), ManagedType.String },
		{ typeof(string), ManagedType.String },
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
	internal static unsafe void GetAssemblyTypes(int InAssemblyLoadContextId, int InAssemblyId, int* OutTypes, int* OutTypeCount)
	{
		try
		{
			if (!AssemblyLoader.TryGetAssembly(InAssemblyLoadContextId, InAssemblyId, out var assembly))
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
	internal static unsafe NativeString GetFullTypeName(int InType)
	{
		try
		{
			if (!s_CachedTypes.TryGetValue(InType, out var type) || type == null)
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
			if (!s_CachedTypes.TryGetValue(InType, out var type) || type == null)
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
			if (!s_CachedTypes.TryGetValue(InType, out var type) || OutBaseType == null || type == null)
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
    internal static unsafe void GetInterfaceTypeCount(int InType, int* OutCount)
    {
        try
        {
            if (!s_CachedTypes.TryGetValue(InType, out var type) || OutCount == null || type == null)
                return;

            var typeInterfaces = type.GetInterfaces();
            if (typeInterfaces == null)
            {
                *OutCount = 0;
                return;
            }

            *OutCount = typeInterfaces.Length;
        }
        catch (Exception e)
        {
            HandleException(e);
        }
    }

	[UnmanagedCallersOnly]
    internal static unsafe void GetInterfaceTypes(int InType, int* OutTypes)
    {
		try
		{
			if (!s_CachedTypes.TryGetValue(InType, out var type) || OutTypes == null || type == null)
				return;

            var typeInterfaces = type.GetInterfaces();
            if (typeInterfaces == null)
            {
                return;
            }

            for (int i = 0; i < typeInterfaces.Length; ++i)
            {
                OutTypes[i] = s_CachedTypes.Add(typeInterfaces[i]);
            }
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
			if (!s_CachedTypes.TryGetValue(InType, out var type) || type == null)
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
			if (!s_CachedTypes.TryGetValue(InType0, out var type0) || type0 == null || !s_CachedTypes.TryGetValue(InType1, out var type1) || type1 == null)
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
			if (!s_CachedTypes.TryGetValue(InType0, out var type0) || type0 == null || !s_CachedTypes.TryGetValue(InType1, out var type1) || type1 == null)
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
			if (!s_CachedTypes.TryGetValue(InType0, out var type0) || type0 == null || !s_CachedTypes.TryGetValue(InType1, out var type1) || type1 == null)
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
			if (!s_CachedTypes.TryGetValue(InTypeID, out var type) || type == null)
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
			if (!s_CachedTypes.TryGetValue(InType, out var type) || type == null)
				return;

			ReadOnlySpan<MethodInfo> methods = type.GetMethods(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Static | BindingFlags.DeclaredOnly);

			if (methods.Length == 0)
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
			if (!s_CachedTypes.TryGetValue(InType, out var type) || type == null)
				return;

			ReadOnlySpan<FieldInfo> fields = type.GetFields(BindingFlags.Instance | BindingFlags.Static | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.DeclaredOnly);

			if (fields.Length == 0)
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
			if (!s_CachedTypes.TryGetValue(InType, out var type) || type == null)
				return;

			ReadOnlySpan<PropertyInfo> properties = type.GetProperties(BindingFlags.Instance | BindingFlags.Static | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.DeclaredOnly);

			if (properties.Length == 0)
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
			if (!s_CachedTypes.TryGetValue(InType, out var type) || type == null || !s_CachedTypes.TryGetValue(InAttributeType, out var attributeType) || attributeType == null)
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
			if (!s_CachedTypes.TryGetValue(InType, out var type) || type == null)
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
			if (!s_CachedTypes.TryGetValue(InType, out var type) || type == null)
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
			if (!s_CachedMethods.TryGetValue(InMethodInfo, out var methodInfo) || methodInfo == null)
				return NativeString.Null();

			return methodInfo.Name;
		}
		catch (Exception ex)
		{
			HandleException(ex);
			return NativeString.Null();
		}
	}

	private static readonly Dictionary<Type, string> s_PrimitiveAliases = new()
	{
		{ typeof(void),    "void"    },
		{ typeof(bool),    "bool"    },
		{ typeof(byte),    "byte"    },
		{ typeof(sbyte),   "sbyte"   },
		{ typeof(short),   "short"   },
		{ typeof(ushort),  "ushort"  },
		{ typeof(int),     "int"     },
		{ typeof(uint),    "uint"    },
		{ typeof(long),    "long"    },
		{ typeof(ulong),   "ulong"   },
		{ typeof(float),   "float"   },
		{ typeof(double),  "double"  },
		{ typeof(decimal), "decimal" },
		{ typeof(char),    "char"    },
		{ typeof(string),  "string"  },
		{ typeof(object),  "object"  },
		{ typeof(nint),    "nint"    },
		{ typeof(nuint),   "nuint"   },
	};

	// Recursive type-name formatter that handles every case where Type.FullName is null.
	private static string FormatType(Type t)
	{
		if (t.IsByRef)
			return FormatType(t.GetElementType()!);

		if (t.IsPointer)
			return FormatType(t.GetElementType()!) + "*";

		if (t.IsArray)
			return FormatType(t.GetElementType()!) + "[]";

		if (t.IsGenericParameter)
			return t.Name;

		if (s_PrimitiveAliases.TryGetValue(t, out var alias))
			return alias;

		if (t.IsGenericType)
		{
			ReadOnlySpan<char> name = t.Name.AsSpan();
			int tick = name.IndexOf('`');
			if (tick >= 0)
				name = name.Slice(0, tick);

			var sb = new StringBuilder();
			if (!string.IsNullOrEmpty(t.Namespace))
				sb.Append(t.Namespace).Append('.');
			sb.Append(name).Append('<');

			var args = t.GetGenericArguments();
			for (int i = 0; i < args.Length; i++)
			{
				if (i > 0) sb.Append(", ");
				sb.Append(FormatType(args[i]));
			}
			sb.Append('>');
			return sb.ToString();
		}

		return t.FullName ?? t.Name;
	}

	private static string GetMethodModifier(MethodBase method)
	{
		if (method.IsAbstract)
			return "abstract ";

		if (method is MethodInfo mi && mi.IsVirtual)
		{
			bool isOverride = mi.GetBaseDefinition() != mi;
			if (isOverride)
				return mi.IsFinal ? "sealed override " : "override ";
			return mi.IsFinal ? "sealed " : "virtual ";
		}

		return "";
	}

	private static string FormatDefaultValue(object? value)
	{
		if (value == null)
			return "null";
		if (value is string s)
			return $"\"{s}\"";
		if (value is bool b)
			return b ? "true" : "false";
		if (value is char c)
			return $"'{c}'";
		return Convert.ToString(value, CultureInfo.InvariantCulture) ?? "";
	}

	private static string FormatMethod(MethodBase method)
	{
		var sb = new StringBuilder();
		bool isConstructor = method is ConstructorInfo;

		sb.Append(GetMethodModifier(method));

		if (method.IsStatic && !isConstructor)
			sb.Append("static ");

		if (!isConstructor && method is MethodInfo mi)
		{
			sb.Append(FormatType(mi.ReturnType)).Append(' ').Append(mi.Name);

			if (mi.IsGenericMethod)
			{
				sb.Append('<');
				var genArgs = mi.GetGenericArguments();
				for (int i = 0; i < genArgs.Length; i++)
				{
					if (i > 0) sb.Append(", ");
					sb.Append(FormatType(genArgs[i]));
				}
				sb.Append('>');
			}
		}
		else if (method.DeclaringType != null)
		{
			string typeName = method.DeclaringType.Name;
			int tick = typeName.IndexOf('`');
			if (tick >= 0)
				typeName = typeName.Substring(0, tick);
			sb.Append(typeName);
		}

		sb.Append('(');
		var parameters = method.GetParameters();
		for (int i = 0; i < parameters.Length; i++)
		{
			if (i > 0) sb.Append(", ");
			var p = parameters[i];

			if (p.GetCustomAttribute<ParamArrayAttribute>() != null)
				sb.Append("params ");
			else if (p.IsOut)
				sb.Append("out ");
			else if (p.ParameterType.IsByRef)
				sb.Append(p.IsIn ? "in " : "ref ");

			sb.Append(FormatType(p.ParameterType));
			if (!string.IsNullOrEmpty(p.Name))
				sb.Append(' ').Append(p.Name);

			if (p.HasDefaultValue)
				sb.Append(" = ").Append(FormatDefaultValue(p.DefaultValue));
		}
		sb.Append(')');

		return sb.ToString();
	}

	[UnmanagedCallersOnly]
	internal static unsafe NativeString GetMethodInfoFriendlyName(int InMethodInfo)
	{
		try
		{
			if (!s_CachedMethods.TryGetValue(InMethodInfo, out var methodInfo) || methodInfo == null)
				return NativeString.Null();

			return FormatMethod(methodInfo);
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
			if (!s_CachedMethods.TryGetValue(InMethodInfo, out var methodInfo) || OutReturnType == null || methodInfo == null)
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
			if (!s_CachedMethods.TryGetValue(InMethodInfo, out var methodInfo) || methodInfo == null)
				return;

			ReadOnlySpan<ParameterInfo> parameters = methodInfo.GetParameters();

			if (parameters.Length == 0)
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
			if (!s_CachedMethods.TryGetValue(InMethodInfo, out var methodInfo) || methodInfo == null)
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

	private static TypeAccessibility GetTypeAccessibility(MethodBase InMethod)
	{
		if (InMethod.IsPublic) return TypeAccessibility.Public;
		if (InMethod.IsPrivate) return TypeAccessibility.Private;
		if (InMethod.IsFamily) return TypeAccessibility.Protected;
		if (InMethod.IsAssembly) return TypeAccessibility.Internal;
		if (InMethod.IsFamilyOrAssembly) return TypeAccessibility.ProtectedPublic;
		if (InMethod.IsFamilyAndAssembly) return TypeAccessibility.PrivateProtected;
		return TypeAccessibility.Public;
	}

	[UnmanagedCallersOnly]
	internal static unsafe TypeAccessibility GetMethodInfoAccessibility(int InMethodInfo)
	{
		try
		{
			if (!s_CachedMethods.TryGetValue(InMethodInfo, out var methodInfo) || methodInfo == null)
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
			if (!s_CachedFields.TryGetValue(InFieldInfo, out var fieldInfo) || fieldInfo == null)
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
			if (!s_CachedFields.TryGetValue(InFieldInfo, out var fieldInfo) || fieldInfo == null)
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
			if (!s_CachedFields.TryGetValue(InFieldInfo, out var fieldInfo) || fieldInfo == null)
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
	internal static unsafe Bool32 GetFieldInfoIsStatic(int InFieldInfo)
	{
		try
		{
			if (!s_CachedFields.TryGetValue(InFieldInfo, out var fieldInfo) || fieldInfo == null)
				return false;

			return fieldInfo.IsStatic;
		}
		catch (Exception ex)
		{
			HandleException(ex);
			return false;
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe Bool32 GetFieldInfoIsLiteral(int InFieldInfo)
	{
		try
		{
			if (!s_CachedFields.TryGetValue(InFieldInfo, out var fieldInfo) || fieldInfo == null)
				return false;

			return fieldInfo.IsLiteral;
		}
		catch (Exception ex)
		{
			HandleException(ex);
			return false;
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe Bool32 GetFieldInfoIsInitOnly(int InFieldInfo)
	{
		try
		{
			if (!s_CachedFields.TryGetValue(InFieldInfo, out var fieldInfo) || fieldInfo == null)
				return false;

			return fieldInfo.IsInitOnly;
		}
		catch (Exception ex)
		{
			HandleException(ex);
			return false;
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe void GetFieldInfoAttributes(int InFieldInfo, int* OutAttributes, int* OutAttributesCount)
	{
		try
		{
			if (!s_CachedFields.TryGetValue(InFieldInfo, out var fieldInfo) || fieldInfo == null)
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
			if (!s_CachedProperties.TryGetValue(InPropertyInfo, out var propertyInfo) || propertyInfo == null)
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
			if (!s_CachedProperties.TryGetValue(InPropertyInfo, out var propertyInfo) || propertyInfo == null)
				return;

			*OutPropertyType = s_CachedTypes.Add(propertyInfo.PropertyType);
		}
		catch (Exception ex)
		{
			HandleException(ex);
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe Bool32 GetPropertyInfoHasGetter(int InPropertyInfo)
	{
		try
		{
			if (!s_CachedProperties.TryGetValue(InPropertyInfo, out var propertyInfo) || propertyInfo == null)
				return false;

			return propertyInfo.GetGetMethod(nonPublic: true) != null;
		}
		catch (Exception ex)
		{
			HandleException(ex);
			return false;
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe Bool32 GetPropertyInfoHasSetter(int InPropertyInfo)
	{
		try
		{
			if (!s_CachedProperties.TryGetValue(InPropertyInfo, out var propertyInfo) || propertyInfo == null)
				return false;

			return propertyInfo.GetSetMethod(nonPublic: true) != null;
		}
		catch (Exception ex)
		{
			HandleException(ex);
			return false;
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe TypeAccessibility GetPropertyInfoGetterAccessibility(int InPropertyInfo)
	{
		try
		{
			if (!s_CachedProperties.TryGetValue(InPropertyInfo, out var propertyInfo) || propertyInfo == null)
				return TypeAccessibility.Private;

			var getter = propertyInfo.GetGetMethod(nonPublic: true);
			return getter != null ? GetTypeAccessibility(getter) : TypeAccessibility.Private;
		}
		catch (Exception ex)
		{
			HandleException(ex);
			return TypeAccessibility.Private;
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe TypeAccessibility GetPropertyInfoSetterAccessibility(int InPropertyInfo)
	{
		try
		{
			if (!s_CachedProperties.TryGetValue(InPropertyInfo, out var propertyInfo) || propertyInfo == null)
				return TypeAccessibility.Private;

			var setter = propertyInfo.GetSetMethod(nonPublic: true);
			return setter != null ? GetTypeAccessibility(setter) : TypeAccessibility.Private;
		}
		catch (Exception ex)
		{
			HandleException(ex);
			return TypeAccessibility.Private;
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe Bool32 GetPropertyInfoIsStatic(int InPropertyInfo)
	{
		try
		{
			if (!s_CachedProperties.TryGetValue(InPropertyInfo, out var propertyInfo) || propertyInfo == null)
				return false;

			var accessor = propertyInfo.GetGetMethod(nonPublic: true) ?? propertyInfo.GetSetMethod(nonPublic: true);
			return accessor?.IsStatic ?? false;
		}
		catch (Exception ex)
		{
			HandleException(ex);
			return false;
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe void GetPropertyInfoAttributes(int InPropertyInfo, int* OutAttributes, int* OutAttributesCount)
	{
		try
		{
			if (!s_CachedProperties.TryGetValue(InPropertyInfo, out var propertyInfo) || propertyInfo == null)
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
	internal static unsafe void GetTypeConstructors(int InType, int* InConstructorArray, int* InConstructorCount)
	{
		try
		{
			if (!s_CachedTypes.TryGetValue(InType, out var type) || type == null)
				return;

			ReadOnlySpan<ConstructorInfo> constructors = type.GetConstructors(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Static | BindingFlags.DeclaredOnly);

			if (constructors.Length == 0)
			{
				*InConstructorCount = 0;
				return;
			}

			*InConstructorCount = constructors.Length;

			if (InConstructorArray == null)
				return;

			for (int i = 0; i < constructors.Length; i++)
			{
				InConstructorArray[i] = s_CachedConstructors.Add(constructors[i]);
			}
		}
		catch (Exception e)
		{
			HandleException(e);
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe NativeString GetConstructorInfoFriendlyName(int InConstructorInfo)
	{
		try
		{
			if (!s_CachedConstructors.TryGetValue(InConstructorInfo, out var ctorInfo) || ctorInfo == null)
				return NativeString.Null();

			return FormatMethod(ctorInfo);
		}
		catch (Exception ex)
		{
			HandleException(ex);
			return NativeString.Null();
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe TypeAccessibility GetConstructorInfoAccessibility(int InConstructorInfo)
	{
		try
		{
			if (!s_CachedConstructors.TryGetValue(InConstructorInfo, out var ctorInfo) || ctorInfo == null)
				return TypeAccessibility.Internal;

			return GetTypeAccessibility(ctorInfo);
		}
		catch (Exception ex)
		{
			HandleException(ex);
			return TypeAccessibility.Public;
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe void GetConstructorInfoAttributes(int InConstructorInfo, int* OutAttributes, int* OutAttributesCount)
	{
		try
		{
			if (!s_CachedConstructors.TryGetValue(InConstructorInfo, out var ctorInfo) || ctorInfo == null)
				return;

			var attributes = ctorInfo.GetCustomAttributes().ToImmutableArray();

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
	internal static unsafe void GetTypeEvents(int InType, int* InEventArray, int* InEventCount)
	{
		try
		{
			if (!s_CachedTypes.TryGetValue(InType, out var type) || type == null)
				return;

			ReadOnlySpan<EventInfo> events = type.GetEvents(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Static | BindingFlags.DeclaredOnly);

			if (events.Length == 0)
			{
				*InEventCount = 0;
				return;
			}

			*InEventCount = events.Length;

			if (InEventArray == null)
				return;

			for (int i = 0; i < events.Length; i++)
			{
				InEventArray[i] = s_CachedEvents.Add(events[i]);
			}
		}
		catch (Exception e)
		{
			HandleException(e);
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe NativeString GetEventInfoName(int InEventInfo)
	{
		try
		{
			if (!s_CachedEvents.TryGetValue(InEventInfo, out var eventInfo) || eventInfo == null)
				return NativeString.Null();

			return eventInfo.Name;
		}
		catch (Exception ex)
		{
			HandleException(ex);
			return NativeString.Null();
		}
	}

	private static string FormatEvent(EventInfo eventInfo)
	{
		var sb = new StringBuilder();

		var add = eventInfo.GetAddMethod(nonPublic: true);
		if (add != null && add.IsStatic)
			sb.Append("static ");

		sb.Append("event ");
		if (eventInfo.EventHandlerType != null)
			sb.Append(FormatType(eventInfo.EventHandlerType));
		sb.Append(' ').Append(eventInfo.Name);

		return sb.ToString();
	}

	[UnmanagedCallersOnly]
	internal static unsafe NativeString GetEventInfoFriendlyName(int InEventInfo)
	{
		try
		{
			if (!s_CachedEvents.TryGetValue(InEventInfo, out var eventInfo) || eventInfo == null)
				return NativeString.Null();

			return FormatEvent(eventInfo);
		}
		catch (Exception ex)
		{
			HandleException(ex);
			return NativeString.Null();
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe void GetEventInfoHandlerType(int InEventInfo, int* OutHandlerType)
	{
		try
		{
			if (!s_CachedEvents.TryGetValue(InEventInfo, out var eventInfo) || eventInfo == null || eventInfo.EventHandlerType == null)
				return;

			*OutHandlerType = s_CachedTypes.Add(eventInfo.EventHandlerType);
		}
		catch (Exception ex)
		{
			HandleException(ex);
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe TypeAccessibility GetEventInfoAccessibility(int InEventInfo)
	{
		try
		{
			if (!s_CachedEvents.TryGetValue(InEventInfo, out var eventInfo) || eventInfo == null)
				return TypeAccessibility.Private;

			var add = eventInfo.GetAddMethod(nonPublic: true);
			return add != null ? GetTypeAccessibility(add) : TypeAccessibility.Private;
		}
		catch (Exception ex)
		{
			HandleException(ex);
			return TypeAccessibility.Private;
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe Bool32 GetEventInfoIsStatic(int InEventInfo)
	{
		try
		{
			if (!s_CachedEvents.TryGetValue(InEventInfo, out var eventInfo) || eventInfo == null)
				return false;

			var add = eventInfo.GetAddMethod(nonPublic: true);
			return add?.IsStatic ?? false;
		}
		catch (Exception ex)
		{
			HandleException(ex);
			return false;
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe void GetEventInfoAttributes(int InEventInfo, int* OutAttributes, int* OutAttributesCount)
	{
		try
		{
			if (!s_CachedEvents.TryGetValue(InEventInfo, out var eventInfo) || eventInfo == null)
				return;

			var attributes = eventInfo.GetCustomAttributes().ToImmutableArray();

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
	internal static unsafe void GetTypeNestedTypes(int InType, int* InTypeArray, int* InTypeCount)
	{
		try
		{
			if (!s_CachedTypes.TryGetValue(InType, out var type) || type == null)
				return;

			Type[] nested = type.GetNestedTypes(BindingFlags.Public);

			if (nested.Length == 0)
			{
				*InTypeCount = 0;
				return;
			}

			*InTypeCount = nested.Length;

			if (InTypeArray == null)
				return;

			for (int i = 0; i < nested.Length; i++)
			{
				InTypeArray[i] = s_CachedTypes.Add(nested[i]);
			}
		}
		catch (Exception e)
		{
			HandleException(e);
		}
	}

	[UnmanagedCallersOnly]
	internal static unsafe void GetAttributeFieldValue(int InAttribute, NativeString InFieldName, IntPtr OutValue)
	{
		try
		{
			if (!s_CachedAttributes.TryGetValue(InAttribute, out var attribute) || attribute == null)
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
			if (!s_CachedAttributes.TryGetValue(InAttribute, out var attribute) || attribute == null)
				return;

			*OutType = s_CachedTypes.Add(attribute.GetType());
		}
		catch (Exception ex)
		{
			HandleException(ex);
		}
	}
}
