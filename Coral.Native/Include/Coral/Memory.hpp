#pragma once

#include "Core.hpp"

namespace Coral {

	struct Memory
	{
		static void* AllocHGlobal(size_t InSize);
		static void FreeHGlobal(void* InPtr);

		static UCChar* StringToCoTaskMemAuto(UCStringView InString);
		static void FreeCoTaskMem(void* InMemory);

	};

}
