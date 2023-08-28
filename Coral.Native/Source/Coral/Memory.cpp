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
		size_t length = (InString.length() + 1) * 2;

#if defined(_WIN32)
		auto* buffer = static_cast<CharType*>(CoTaskMemAlloc(length));
#else
		auto* buffer = static_cast<CharType*>(AllocHGlobal(length));
#endif

		if (buffer != nullptr)
		{
			memcpy(buffer, InString.data(), length);
			buffer[InString.length()] = '\0';
		}

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
