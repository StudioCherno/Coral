using Coral.Managed.Interop;

using System;
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
