using System;
using System.Linq;
using System.Runtime.InteropServices;
using System.Reflection;

namespace Coral.Managed.Interop;

[StructLayout(LayoutKind.Sequential)]
public readonly struct InternalCall
{
	private readonly IntPtr m_NamePtr;
	public readonly IntPtr NativeFunctionPtr;

	public string? Name => Marshal.PtrToStringAuto(m_NamePtr);
}

internal static class InternalCallsManager
{
	[UnmanagedCallersOnly]
	private static void SetInternalCalls(UnmanagedArray InArr)
	{
		var internalCalls = InArr.ToSpan<InternalCall>();

		try
		{
			foreach (var internalCall in internalCalls)
			{
				var name = internalCall.Name;

				if (name == null)
					throw new ArgumentNullException(nameof(name), "Internal call name is null!");

				var fieldNameStart = name.IndexOf('+');
				var fieldNameEnd = name.IndexOf(",", fieldNameStart, StringComparison.CurrentCulture);
				var fieldName = name.Substring(fieldNameStart + 1, fieldNameEnd - fieldNameStart - 1);
				var containingTypeName = name.Remove(fieldNameStart, fieldNameEnd - fieldNameStart);

				var type = TypeHelper.FindType(containingTypeName);

				if (type == null)
					throw new TypeAccessException($"Can't find internal call type '{containingTypeName}'");

				var bindingFlags = BindingFlags.Static | BindingFlags.NonPublic;
				var field = type.GetFields(bindingFlags).FirstOrDefault(field => field.Name == fieldName);

				if (field == null)
				{
					Console.WriteLine($"Couldn't find field {fieldName} in {containingTypeName}");
					continue;
				}

				// TODO(Peter): Changed to !field.FieldType.IsFunctionPointer when .NET 8 is out
				if (field.FieldType != typeof(IntPtr))
				{
					Console.WriteLine($"{fieldName} is not a function pointer!");
					continue;
				}

				field.SetValue(null, internalCall.NativeFunctionPtr);
			}
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
		}
	}
}
