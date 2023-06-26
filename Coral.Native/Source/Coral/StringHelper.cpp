#include "StringHelper.hpp"

namespace Coral {

#if defined(CORAL_WIDE_CHARS)
	std::wstring StringHelper::ConvertUtf8ToWide(std::string_view InString)
	{
		int length = MultiByteToWideChar(CP_UTF8, 0, InString.data(), int32_t(InString.length()), nullptr, 0);
		auto result = std::wstring(length, wchar_t(0));
		MultiByteToWideChar(CP_UTF8, 0, InString.data(), int32_t(InString.length()), result.data(), length);
		return result;
	}
#else
	std::string StringHelper::ConvertUtf8ToWide(std::string_view InString)
	{
		return InString;
	}
#endif

}
