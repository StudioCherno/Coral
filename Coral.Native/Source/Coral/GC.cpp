#include "GC.hpp"
#include "NativeCallables.generated.hpp"

namespace Coral {

	void GC::Collect()
	{
		Collect(-1, GCCollectionMode::Default, true, false);
	}
	
	void GC::Collect(int32_t InGeneration, GCCollectionMode InCollectionMode, bool InBlocking, bool InCompacting)
	{
		s_NativeCallables.CollectGarbageFptr(InGeneration, InCollectionMode, InBlocking, InCompacting);
	}

	void GC::WaitForPendingFinalizers()
	{
		s_NativeCallables.WaitForPendingFinalizersFptr();
	}
	
}
