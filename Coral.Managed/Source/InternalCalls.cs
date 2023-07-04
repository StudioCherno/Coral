using System;
using System.Linq;
using System.Runtime.InteropServices;
using System.Reflection;

namespace Coral.Managed.Interop
{
	[StructLayout(LayoutKind.Sequential)]
	public readonly struct InternalCall
	{
		private readonly IntPtr m_NamePtr;
		public readonly IntPtr NativeFunctionPtr;

		public string Name => Marshal.PtrToStringAuto(m_NamePtr);
	}

	public static class InternalCallsManager
	{
		[UnmanagedCallersOnly]
		public static void SetInternalCalls(UnmanagedArray InArr)
		{
			var internalCalls = InArr.ToArray<InternalCall>();

			try
			{
				foreach (var internalCall in internalCalls)
				{
					var name = internalCall.Name;
					var fieldNameStart = name.IndexOf('+');
					var fieldNameEnd = name.IndexOf(",", fieldNameStart, StringComparison.CurrentCulture);
					var fieldName = name.Substring(fieldNameStart + 1, fieldNameEnd - fieldNameStart - 1);
					var containingTypeName = name.Remove(fieldNameStart, fieldNameEnd - fieldNameStart);

					var type = TypeHelper.FindType(containingTypeName);

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
				Console.WriteLine(ex);
			}
		}
	}

}