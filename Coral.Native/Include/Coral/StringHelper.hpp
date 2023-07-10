#pragma once

#include "Core.hpp"

namespace Coral {

	class StringHelper
	{
	public:
	#if defined(CORAL_WIDE_CHARS)
		static std::wstring ConvertUtf8ToWide(std::string_view InString);
		static std::string ConvertWideToUtf8(const std::wstring& InString);
	#else
		static std::string ConvertUtf8ToWide(std::string_view InString);
	#endif
	};

}
