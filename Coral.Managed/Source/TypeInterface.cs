using Coral.Managed.Interop;

using System;
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

}
