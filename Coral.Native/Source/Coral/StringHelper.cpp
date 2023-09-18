#include "StringHelper.hpp"

#include <codecvt>

namespace Coral {

#if defined(CORAL_WIDE_CHARS)
	std::wstring StringHelper::ConvertUtf8ToWide(std::string_view InString)
	{
		int length = MultiByteToWideChar(CP_UTF8, 0, InString.data(), static_cast<int>(InString.length()), nullptr, 0);
		auto result = std::wstring(length, wchar_t(0));
		MultiByteToWideChar(CP_UTF8, 0, InString.data(), static_cast<int>(InString.length()), result.data(), length);
		return result;
	}

	std::string StringHelper::ConvertWideToUtf8(std::wstring_view InString)
	{
		int requiredLength = WideCharToMultiByte(CP_UTF8, 0, InString.data(), static_cast<int>(InString.length()), nullptr, 0, nullptr, nullptr);
		std::string result(requiredLength, 0);
		(void)WideCharToMultiByte(CP_UTF8, 0, InString.data(), static_cast<int>(InString.length()), result.data(), requiredLength, nullptr, nullptr);
		return result;
	}
	
#else
	std::string StringHelper::ConvertUtf8ToWide(std::string_view InString)
	{
		return std::string(InString);
	}

	std::string StringHelper::ConvertWideToUtf8(std::string_view InString)
	{
		return std::string(InString);
	}
#endif

}
