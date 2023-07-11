using System;
using System.Reflection;
using System.Runtime.InteropServices;
using Coral.Managed.Interop;

namespace Coral.Managed;

internal static class ManagedObject
{
	[StructLayout(LayoutKind.Sequential)]
	public readonly struct ObjectCreateInfo
	{
		public readonly UnmanagedString TypeName;
		public readonly bool IsWeakRef;
		public readonly UnmanagedArray Parameters;
		/*public readonly IntPtr Parameters;
		public readonly IntPtr ParameterTypes;
		public readonly int Length;*/
	}
	
	[UnmanagedCallersOnly]
	private static unsafe IntPtr CreateObject(ObjectCreateInfo* InCreateInfo)
	{
		try
		{
			var type = TypeHelper.FindType(InCreateInfo->TypeName);

			if (type == null)
			{
				Console.WriteLine($"[Coral.Managed]: Unknown type name '{InCreateInfo->TypeName}'");
				return IntPtr.Zero;
			}

			ConstructorInfo? constructor = null;
			
			foreach (var constructorInfo in type.GetConstructors())
			{
				if (constructorInfo.GetParameters().Length != InCreateInfo->Parameters.Length)
					continue;

				constructor = constructorInfo;
				break;
			}

			if (constructor == null)
				return IntPtr.Zero; // TODO(Peter): Throw

			var parameters = Marshalling.MarshalParameterArray(InCreateInfo->Parameters, constructor);
			object? result = parameters != null ? TypeHelper.CreateInstance(type, parameters) : TypeHelper.CreateInstance(type);

			if (result == null)
				return IntPtr.Zero;

			var handle = GCHandle.Alloc(result, InCreateInfo->IsWeakRef ? GCHandleType.Weak : GCHandleType.Pinned);
			return GCHandle.ToIntPtr(handle);
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
			return IntPtr.Zero;
		}
	}

	[UnmanagedCallersOnly]
	public static void DestroyObject(IntPtr InObjectHandle)
	{
		try
		{
			GCHandle.FromIntPtr(InObjectHandle).Free();
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
		}
	}

	[UnmanagedCallersOnly]
	private static void InvokeMethod(IntPtr InObjectHandle, UnmanagedString InMethodName, UnmanagedArray InParameters)
	{
		try
		{
			var target = GCHandle.FromIntPtr(InObjectHandle).Target;

			if (target == null)
				return; // TODO(Peter): Throw
			
			var targetType = target.GetType();

			MethodInfo? methodInfo = null;
			foreach (var mi in targetType.GetMethods())
			{
				if (mi.Name != InMethodName || mi.GetParameters().Length != InParameters.Length)
					continue;
				
				methodInfo = mi;
				break;
			}

			if (methodInfo == null)
				return; // TODO(Peter): Throw

			var parameterPointers = InParameters.ToIntPtrArray();
			object?[]? parameters = null;

			if (parameterPointers.Length > 0)
			{
				var parameterTypes = methodInfo.GetParameters();
				parameters = new object[parameterPointers.Length];
				
				for (int i = 0; i < parameterPointers.Length; i++)
					parameters[i] = Marshalling.MarshalPointer(parameterPointers[i], parameterTypes[i].ParameterType);
			}
			
			methodInfo.Invoke(target, parameters);
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
		}
	}
	
	[UnmanagedCallersOnly]
	private static void InvokeMethodRet(IntPtr InObjectHandle, UnmanagedString InMethodName, UnmanagedArray InParameters, IntPtr InResultStorage)
	{
		try
		{
			var target = GCHandle.FromIntPtr(InObjectHandle).Target;

			if (target == null)
				return;
			
			var targetType = target.GetType();

			MethodInfo? methodInfo = null;
			foreach (var mi in targetType.GetMethods())
			{
				if (mi.Name != InMethodName || mi.GetParameters().Length != InParameters.Length)
					continue;
				
				methodInfo = mi;
				break;
			}

			if (methodInfo == null)
				throw new MissingMethodException($"Couldn't find method called {InMethodName}");

			var methodParameters = Marshalling.MarshalParameterArray(InParameters, methodInfo);
			
			object? value = methodInfo.Invoke(target, methodParameters);

			if (value == null)
				return;

			var valueSize = Marshal.SizeOf(value.GetType());
			var handle = GCHandle.Alloc(value, GCHandleType.Pinned);

			unsafe
			{
				Buffer.MemoryCopy(handle.AddrOfPinnedObject().ToPointer(), InResultStorage.ToPointer(), valueSize, valueSize);
			}
			
			handle.Free();
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
		}
	}
}