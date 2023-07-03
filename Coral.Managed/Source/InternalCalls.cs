using System;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Reflection.Emit;
using System.Diagnostics;

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
		private static Dictionary<Type, Delegate> s_InternalCalls = new Dictionary<Type, Delegate>();
		private static Dictionary<Type, DynamicMethod> s_DynamicMethods = new Dictionary<Type, DynamicMethod>();

		[UnmanagedCallersOnly]
		public static void SetInternalCalls(UnmanagedArray InArr)
		{
			var internalCalls = InArr.ToArray<InternalCall>();

			for (int i = 0; i < internalCalls.Length; i++)
			{
				var icall = internalCalls[i];
				var delegateType = TypeHelper.FindType(icall.Name);

				if (delegateType == null || !delegateType.IsDelegate())
				{
					Console.WriteLine($"[Coral.Managed]: Failed to find delegate type '{icall.Name}'");
					continue;
				}

				/*var delegateInvoke = delegateType.GetMethod("Invoke");
				var method = new DynamicMethod("InvokeNative", delegateInvoke.ReturnType, Array.Empty<Type>());
				var ilGenerator = method.GetILGenerator();
				ilGenerator.Emit(OpCodes.Ldc_I8, (long)icall.NativeFunctionPtr);
				ilGenerator.Emit(OpCodes.Conv_I);
				ilGenerator.EmitCalli(OpCodes.Calli, CallingConvention.StdCall, delegateInvoke.ReturnType, Array.Empty<Type>());
				ilGenerator.Emit(OpCodes.Ret);
				s_DynamicMethods.Add(delegateType, method);
				
				var del = method.CreateDelegate(delegateType);*/

				var del = Marshal.GetDelegateForFunctionPointer(icall.NativeFunctionPtr, delegateType);

				if (!s_InternalCalls.TryAdd(delegateType, del))
				{
					Console.WriteLine($"[Coral.Managed]: Internal call {icall.Name}");
				}
			}
		}

		public static void Invoke<D>(params object[] InArgs)
		{
			var delegateType = typeof(D);

			//if (!delegateType.IsDelegate())
			//	throw new ArgumentException($"{delegateType.AssemblyQualifiedName} is not a delegate type");

			//Delegate delegateInstance;
			/*if (!s_InternalCalls.TryGetValue(delegateType, out delegateInstance))
			{
				throw new ArgumentException($"{delegateType.AssemblyQualifiedName} is not an internal call");
			}*/
			s_InternalCalls[delegateType].DynamicInvoke(InArgs);
		}

		public static R Invoke<D, R>(params object[] InArgs)
		{
			var delegateType = typeof(D);

			if (!delegateType.IsDelegate())
				throw new ArgumentException($"{delegateType.AssemblyQualifiedName} is not a delegate type");

			Delegate delegateInstance;
			if (!s_InternalCalls.TryGetValue(delegateType, out delegateInstance))
			{
				throw new ArgumentException($"{delegateType.AssemblyQualifiedName} is not an internal call");
			}

			return (R)delegateInstance.DynamicInvoke(InArgs);
		}
	}

}