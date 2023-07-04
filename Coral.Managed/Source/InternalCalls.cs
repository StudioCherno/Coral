using System;
using System.Linq;
using System.Runtime.InteropServices;
using System.Reflection;

namespace Coral.Interop
{
	[StructLayout(LayoutKind.Sequential)]
	public struct InternalCall
	{
		public IntPtr NamePtr;
		public IntPtr NativeFunctionPtr;

		public string Name => Marshal.PtrToStringAuto(NamePtr);
	}

	public static class InternalCallsManager
	{
		[UnmanagedCallersOnly]
		public static void SetInternalCalls(UnmanagedArray InArr)
		{
			var internalCalls = InArr.ToArray<InternalCall>();

			for (int i = 0; i < internalCalls.Length; i++)
			{
				var icall = internalCalls[i];

				try
				{
					var name = icall.Name;
					var fieldNameStart = name.IndexOf('+');
					var fieldNameEnd = name.IndexOf(",", fieldNameStart);
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

					field.SetValue(null, icall.NativeFunctionPtr);
				}
				catch (Exception ex)
				{
					Console.WriteLine(ex);
				}
			}
		}
	}

}