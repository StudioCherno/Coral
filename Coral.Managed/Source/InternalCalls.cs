using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

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

		[UnmanagedCallersOnly]
		public static void SetInternalCalls(UnmanagedArray InArr)
		{
			var internalCalls = InArr.ToArray<InternalCall>();

			for (int i = 0; i < internalCalls.Length; i++)
			{
				var icall = internalCalls[i];
				var delegateType = Type.GetType(icall.Name);

				if (delegateType == null || !delegateType.IsDelegate())
				{
					Console.WriteLine($"[Coral.Managed]: Failed to find delegate type '{icall.Name}'");
					continue;
				}

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

			if (!delegateType.IsDelegate())
				throw new ArgumentException($"{delegateType.AssemblyQualifiedName} is not a delegate type");

			Delegate delegateInstance;
			if (!s_InternalCalls.TryGetValue(delegateType, out delegateInstance))
			{
				throw new ArgumentException($"{delegateType.AssemblyQualifiedName} is not an internal call");
			}

			delegateInstance.DynamicInvoke(InArgs);
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