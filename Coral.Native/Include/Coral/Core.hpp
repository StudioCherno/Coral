#pragma once

#include <string_view>

#ifdef _WIN32
	#define CORAL_CALLTYPE __cdecl

	#ifdef _WCHAR_T_DEFINED
		#define CORAL_STR(s) L##s
		#define CORAL_WIDE_CHARS

		using CharType = wchar_t;
		using StringView = std::wstring_view;

	#else
		#define CORAL_STR(s) s

		using CharType = unsigned short;
		using StringView = std::string_view;
#endif
#else
	#define CORAL_CALLTYPE
	#define CORAL_STR(s) s

	using CharType = char;
	using StringView = std::string_view;
#endif

#define CORAL_UNMANAGED_CALLERS_ONLY ((const CharType*)-1)

namespace Coral {

	using Bool32 = uint32_t;

	enum class TypeVisibility
	{
		Public,
		Private,
		Protected,
		Internal,
		ProtectedPublic,
		PrivateProtected
	};

	using TypeId = void*;

	struct InternalCall
	{
		const CharType* Name;
		void* NativeFunctionPtr;
	};

}
