using Coral.Managed.Interop;

using System;
using System.Runtime.InteropServices;

namespace Coral.Managed;

internal static class GarbageCollector
{

	[UnmanagedCallersOnly]
	internal static void CollectGarbage(int InGeneration, GCCollectionMode InCollectionMode, Bool32 InBlocking, Bool32 InCompacting)
	{
		try
		{
			if (InGeneration < 0)
				GC.Collect();
			else
				GC.Collect(InGeneration, InCollectionMode, InBlocking, InCompacting);
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
		}
	}

	[UnmanagedCallersOnly]
	internal static void WaitForPendingFinalizers()
	{
		try
		{
			GC.WaitForPendingFinalizers();
		}
		catch (Exception ex)
		{
			ManagedHost.HandleException(ex);
		}
	}

}
