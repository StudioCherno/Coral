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

	CharType* Memory::StringToCoTaskMemAuto(StringView InString)
	{
		size_t length = InString.length() + 1;
		size_t size = length * sizeof(CharType);

#if defined(_WIN32)
		auto* buffer = static_cast<CharType*>(CoTaskMemAlloc(size));

		if (buffer != nullptr)
		{
			memset(buffer, 0xCE, size);
			wcscpy(buffer, InString.data());
		}
#else
		auto* buffer = static_cast<CharType*>(AllocHGlobal(size));

		if (buffer != nullptr)
		{
			memset(buffer, 0, size);
			strcpy(buffer, InString.data());
		}
#endif

		return buffer;
	}

	void Memory::FreeCoTaskMem(void* InMemory)
	{
#if defined(_WIN32)
		CoTaskMemFree(InMemory);
#else
		FreeHGlobal(InMemory);
#endif
	}

}
