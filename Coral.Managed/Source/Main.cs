using Coral.Managed.Interop;

using System;
using System.Diagnostics;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Coral.Managed;

internal static class ManagedHost
{
	private static unsafe delegate*<NativeString, void> s_ExceptionCallback;

	[UnmanagedCallersOnly]
	private static void Initialize()
	{
	}

	[UnmanagedCallersOnly]
	private static unsafe void SetExceptionCallback(delegate*<NativeString, void> InCallback)
	{
		s_ExceptionCallback = InCallback;
	}

	internal enum TypeVisibility
	{
		Public,
		Private,
		Protected,
		Internal,
		ProtectedPublic,
		PrivateProtected
	}

	private static TypeVisibility GetTypeVisibility(FieldInfo InFieldInfo)
	{
		if (InFieldInfo.IsPublic) return TypeVisibility.Public;
		if (InFieldInfo.IsPrivate) return TypeVisibility.Private;
		if (InFieldInfo.IsFamily) return TypeVisibility.Protected;
		if (InFieldInfo.IsAssembly) return TypeVisibility.Internal;
		if (InFieldInfo.IsFamilyOrAssembly) return TypeVisibility.ProtectedPublic;
		if (InFieldInfo.IsFamilyAndAssembly) return TypeVisibility.PrivateProtected;
		return TypeVisibility.Public;
	}

	private static TypeVisibility GetTypeVisibility(System.Reflection.MethodInfo InMethodInfo)
	{
		if (InMethodInfo.IsPublic) return TypeVisibility.Public;
		if (InMethodInfo.IsPrivate) return TypeVisibility.Private;
		if (InMethodInfo.IsFamily) return TypeVisibility.Protected;
		if (InMethodInfo.IsAssembly) return TypeVisibility.Internal;
		if (InMethodInfo.IsFamilyOrAssembly) return TypeVisibility.ProtectedPublic;
		if (InMethodInfo.IsFamilyAndAssembly) return TypeVisibility.PrivateProtected;
		return TypeVisibility.Public;
	}

	/*[UnmanagedCallersOnly]
	private static unsafe Bool32 GetReflectionTypeFromObject(IntPtr InObjectHandle, ReflectionType* OutReflectionType)
	{
		try
		{
			var target = GCHandle.FromIntPtr(InObjectHandle).Target;

			if (target == null)
			{
				throw new ArgumentNullException(nameof(InObjectHandle), "Trying to get reflection type from a null object.");
			}

			var reflectionType = BuildReflectionType(target.GetType());

			if (reflectionType == null)
				return false;

			*OutReflectionType = reflectionType.Value;
			return true;
		}
		catch (Exception e)
		{
			HandleException(e);
			return false;
		}
	}*/

	private struct MethodInfo
	{
		public NativeString Name;
		public TypeVisibility Visibility;
	}

	[UnmanagedCallersOnly]
	private static unsafe void GetTypeMethods(NativeString InTypeName, MethodInfo* InMethodArray, int* InMethodCount)
	{
		try
		{
			var type = TypeInterface.FindType(InTypeName);

			if (type == null)
				return;

			ReadOnlySpan<System.Reflection.MethodInfo> methods = type.GetMethods(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Static);

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
				InMethodArray[i] = new()
				{
					Name = methods[i].Name,
					Visibility = GetTypeVisibility(methods[i])
				};
			}
		}
		catch (Exception e)
		{
			HandleException(e);
		}
	}

	internal struct ManagedField
	{
		public NativeString Name;
		public TypeVisibility Visibility;
	}

	[UnmanagedCallersOnly]
	private static unsafe void QueryObjectFields(NativeString InTypeName, ManagedField* InFieldsArray, int* OutFieldCount)
	{
		var type = TypeInterface.FindType(InTypeName);

		if (type == null)
		{
			Console.WriteLine("Invalid type");
			*OutFieldCount = 0;
			return;
		}

		ReadOnlySpan<FieldInfo> fields = type.GetFields();
		*OutFieldCount = fields.Length;

		if (InFieldsArray == null)
			return;

		for (int i = 0; i < fields.Length; i++)
		{
			var field = fields[i];
			var managedField = new ManagedField();

			managedField.Name = field.Name;
			managedField.Visibility = GetTypeVisibility(field);

			InFieldsArray[i] = managedField;
		}
	}

	internal static void HandleException(Exception InException)
	{
		unsafe
		{
			if (s_ExceptionCallback == null)
				return;

			// NOTE(Peter): message will be cleaned up by C++ code
			NativeString message = InException.ToString();
			s_ExceptionCallback(message);
		}
	}

}
