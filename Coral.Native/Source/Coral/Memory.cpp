#include "Memory.hpp"

namespace Coral {

	void* Memory::AllocHGlobal(size_t InSize)
	{
#if defined(_WIN32)
		return LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, InSize);
#else
		return malloc(InSize);
#endif
	}

	void Memory::FreeHGlobal(void* InPtr)
	{
#if defined(_WIN32)
		LocalFree(InPtr);
#else
		free(InPtr);
#endif
	}

}
