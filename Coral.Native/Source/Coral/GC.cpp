#include "GC.hpp"
#include "CoralManagedFunctions.hpp"

namespace Coral {

	void GC::Collect()
	{
		Collect(-1, GCCollectionMode::Default, true, false);
	}
	
	void GC::Collect(int32_t InGeneration, GCCollectionMode InCollectionMode, bool InBlocking, bool InCompacting)
	{
		s_ManagedFunctions.CollectGarbageFptr(InGeneration, InCollectionMode, InBlocking, InCompacting);
	}

	void GC::WaitForPendingFinalizers()
	{
		s_ManagedFunctions.WaitForPendingFinalizersFptr();
	}
	
}
