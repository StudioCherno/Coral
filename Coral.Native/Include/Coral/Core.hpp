#pragma once

#ifdef _WIN32
	#define CORAL_CALLTYPE __cdecl

	#ifdef _WCHAR_T_DEFINED
		#define CORAL_STR(s) L##s
		#define CORAL_WIDE_CHARS 1

		using CharType = wchar_t;
	#else
		#define CORAL_STR(s) s
		#define CORAL_WIDE_CHARS 0

		using CharType = unsigned short;
	#endif
#else
	#define CORAL_CALLTYPE
	#define CORAL_STR(s) s
	#define CORAL_WIDE_CHARS 0

	using CharType = char;
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

}
