#include "Memory.hpp"

namespace Coral {

	void* Memory::AllocHGlobal(size_t InSize)
	{
#ifdef CORAL_WINDOWS
		return LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, InSize);
#else
		return malloc(InSize);
#endif
	}

	void Memory::FreeHGlobal(void* InPtr)
	{
#ifdef CORAL_WINDOWS
		LocalFree(InPtr);
#else
		free(InPtr);
#endif
	}

	UCChar* Memory::StringToCoTaskMemAuto(UCStringView InString)
	{
		size_t length = InString.length() + 1;
		size_t size = length * sizeof(UCChar);

		// TODO(Emily): This (and a few other places) misuse the WC/MB split assuming Windows is the only WC platform.
		//				If we want to decide that is a general assumption we can entirely remove the `CORAL_WIDE_CHARS`
		//				Macro and its exclusive effects.
#ifdef CORAL_WINDOWS
		auto* buffer = static_cast<UCChar*>(CoTaskMemAlloc(size));

		if (buffer != nullptr)
		{
			memset(buffer, 0xCE, size);
			wcscpy(buffer, InString.data());
		}
#else
		UCChar* buffer;

		if ((buffer = static_cast<UCChar*>(calloc(length, sizeof(UCChar)))))
		{
			strcpy(buffer, InString.data());
		}
#endif

		return buffer;
	}

	void Memory::FreeCoTaskMem(void* InMemory)
	{
#ifdef CORAL_WINDOWS
		CoTaskMemFree(InMemory);
#else
		FreeHGlobal(InMemory);
#endif
	}

}
