#include "StringHelper.hpp"

#include <codecvt>

namespace Coral {

	// TODO(Emily): This class assumes that WC platform is synonymous with Windows.

	UCString StringHelper::ConvertUtf8ToWide(std::string_view InString)
	{
#ifdef CORAL_WIDE_CHARS
		int length = MultiByteToWideChar(CP_UTF8, 0, InString.data(), static_cast<int>(InString.length()), nullptr, 0);
		auto result = UCString(length, UCChar(0));
		MultiByteToWideChar(CP_UTF8, 0, InString.data(), static_cast<int>(InString.length()), result.data(), length);
		return result;
#else
		return UCString(InString);
#endif
	}

	std::string StringHelper::ConvertWideToUtf8(UCStringView InString)
	{
#ifdef CORAL_WIDE_CHARS
		int requiredLength = WideCharToMultiByte(CP_UTF8, 0, InString.data(), static_cast<int>(InString.length()), nullptr, 0, nullptr, nullptr);
		std::string result(requiredLength, 0);
		(void)WideCharToMultiByte(CP_UTF8, 0, InString.data(), static_cast<int>(InString.length()), result.data(), requiredLength, nullptr, nullptr);
		return result;
#else
		return std::string(InString);
#endif
	}
}
