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

	internal struct ManagedField
	{
		public NativeString Name;
		//public TypeVisibility Visibility;
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
			//managedField.Visibility = GetTypeVisibility(field);

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
