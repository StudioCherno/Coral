#pragma once

#include <string_view>
#include <string>

#include <cstdint>
#include <cwchar>

#define CORAL_DEPRECATE_MSG_P(s, x) s ". See `" x "`"

#define CORAL_GLOBAL_ALC_MSG "Global type cache has been superseded by Assembly/ALC-local type APIs"
#define CORAL_GLOBAL_ALC_MSG_P(x) CORAL_DEPRECATE_MSG_P(CORAL_GLOBAL_ALC_MSG, #x)

#define CORAL_LEAK_UC_TYPES_MSG "Global namespace string type abstraction will be removed"
#define CORAL_LEAK_UC_TYPES_MSG_P(x) CORAL_DEPRECATE_MSG_P(CORAL_LEAK_UC_TYPES_MSG, #x)

#ifdef _WIN32
	#define CORAL_WINDOWS
#elif defined(__APPLE__)
	#define CORAL_APPLE
#endif

#ifdef CORAL_WINDOWS
	#define CORAL_CALLTYPE __cdecl
	#define CORAL_HOSTFXR_NAME "hostfxr.dll"

	// TODO(Emily): On Windows shouldn't this use the `UNICODE` macro?
	#ifdef _WCHAR_T_DEFINED
		#define CORAL_WIDE_CHARS
	#endif
#else
	#define CORAL_CALLTYPE

	#ifdef CORAL_APPLE
		#define CORAL_HOSTFXR_NAME "libhostfxr.dylib"
	#else
		#define CORAL_HOSTFXR_NAME "libhostfxr.so"
	#endif
#endif

#ifdef CORAL_WIDE_CHARS
	#define CORAL_STR(s) L##s

	using CharType [[deprecated(CORAL_LEAK_UC_TYPES_MSG_P(Coral::UCChar))]] = wchar_t;
	using StringView [[deprecated(CORAL_LEAK_UC_TYPES_MSG_P(Coral::UCStringView))]] = std::wstring_view;

	namespace Coral {
		using UCChar = wchar_t;
		using UCStringView = std::wstring_view;
		using UCString = std::wstring;
	}
#else
	#define CORAL_STR(s) s

	using CharType [[deprecated(CORAL_LEAK_UC_TYPES_MSG_P(Coral::UCChar))]] = char;
	using StringView [[deprecated(CORAL_LEAK_UC_TYPES_MSG_P(Coral::UCStringView))]] = std::string_view;

	namespace Coral {
		using UCChar = char;
		using UCStringView = std::string_view;
		using UCString = std::string;
	}
#endif

// TODO(Emily): Make a better system for supported version ranges (See `HostInstance.cpp:GetHostFXRPath()`)
/*
#define CORAL_DOTNET_TARGET_VERSION_MAJOR 8
#define CORAL_DOTNET_TARGET_VERSION_MAJOR_STR '8'
*/
#define CORAL_UNMANAGED_CALLERS_ONLY (std::bit_cast<const UCChar*>(-1ULL))

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

	using TypeId = int32_t;
	using ManagedHandle = int32_t;

	struct InternalCall
	{
		// TODO(Emily): Review all `UCChar*` refs to see if they could be `UCStringView`.
		const UCChar* Name;
		void* NativeFunctionPtr;
	};

}
