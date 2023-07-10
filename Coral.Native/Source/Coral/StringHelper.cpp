#include "StringHelper.hpp"

#include <codecvt>

namespace Coral {

#if defined(CORAL_WIDE_CHARS)
	std::wstring StringHelper::ConvertUtf8ToWide(std::string_view InString)
	{
		int length = MultiByteToWideChar(CP_UTF8, 0, InString.data(), int32_t(InString.length()), nullptr, 0);
		auto result = std::wstring(length, wchar_t(0));
		MultiByteToWideChar(CP_UTF8, 0, InString.data(), int32_t(InString.length()), result.data(), length);
		return result;
	}

	std::string StringHelper::ConvertWideToUtf8(const std::wstring& InString)
	{
		return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(InString);
	}
	
#else
	std::string StringHelper::ConvertUtf8ToWide(std::string_view InString)
	{
		return InString;
	}
#endif

}
