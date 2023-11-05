#pragma once

#include <string_view>

#ifdef CORAL_WINDOWS
	#define CORAL_CALLTYPE __cdecl
	#define CORAL_HOSTFXR_NAME "hostfxr.dll"

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
	#define CORAL_HOSTFXR_NAME "libhostfxr.so"

	using CharType = char;
	using StringView = std::string_view;
#endif

#define CORAL_DOTNET_TARGET_VERSION_MAJOR 7
#define CORAL_DOTNET_TARGET_VERSION_MAJOR_STR '7'
#define CORAL_UNMANAGED_CALLERS_ONLY ((const CharType*)-1)

namespace Coral {

	using Bool32 = uint32_t;

	enum class TypeAccessibility
	{
		Public,
		Private,
		Protected,
		Internal,
		ProtectedPublic,
		PrivateProtected
	};

	using TypeId = void*;
	using ManagedHandle = void*;

	struct InternalCall
	{
		const CharType* Name;
		void* NativeFunctionPtr;
	};

}
