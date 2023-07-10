#pragma once

namespace Coral {

	enum class GCCollectionMode
	{
		// Default is the same as using Forced directly
		Default,

		// Forces the garbage collection to occur immediately
		Forced,

		// Allows the garbage collector to determine whether it should reclaim objects right now
		Optimized,

		// Requests that the garbage collector decommit as much memory as possible
		Aggressive
	};
	
	class GC
	{
	public:
		static void Collect();
		static void Collect(int32_t InGeneration, GCCollectionMode InCollectionMode = GCCollectionMode::Default, bool InBlocking = true, bool InCompacting = false);

		static void WaitForPendingFinalizers();
	};
	
}

