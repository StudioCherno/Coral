#pragma once

namespace Coral {

	struct Memory
	{
		static void* AllocHGlobal(size_t InSize);
		static void FreeHGlobal(void* InPtr);
	};

}
