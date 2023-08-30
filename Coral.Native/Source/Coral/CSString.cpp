#include "CSString.hpp"
#include "StringHelper.hpp"
#include "Memory.hpp"
#include "Verify.hpp"

namespace Coral {

	CSString::~CSString()
	{
		CORAL_VERIFY(m_String != nullptr);
		Memory::FreeCoTaskMem(m_String);
		m_String = nullptr;
	}

	CSString CSString::FromUTF8(std::string_view InString)
	{
		CSString result;

#if CORAL_WIDE_CHARS
		auto str = StringHelper::ConvertUtf8ToWide(InString);
		result.m_String = Memory::StringToCoTaskMemAuto(str);
#else
		result.m_String = Memory::StringToCoTaskMemAuto(InString);
#endif

		return result;
	}

	std::string CSString::ToUTF8(CSString InString)
	{
		StringView string(InString.m_String);

#if CORAL_WIDE_CHARS
		return StringHelper::ConvertWideToUtf8(string);
#else
		return string;
#endif
	}

	std::string CSString::ToString() const { return ToUTF8(*this); }

	bool CSString::operator==(const CSString& InOther) const
	{
#if CORAL_WIDE_CHARS
		return wcscmp(m_String, InOther.m_String) == 0;
#else
		return strcmp(m_String, InOther.m_String) == 0;
#endif
	}

}
