#include "CSString.hpp"
#include "StringHelper.hpp"
#include "Memory.hpp"
#include "Verify.hpp"

namespace Coral {

	CSString::CSString(const CSString& InOther)
	{
		if (InOther.m_String != nullptr)
			m_String = Memory::StringToCoTaskMemAuto(InOther.m_String);
	}

	CSString::CSString(CSString&& InOther) noexcept
	{
		m_String = std::exchange(InOther.m_String, nullptr);
	}

	CSString::~CSString()
	{
		if (m_String != nullptr)
			Memory::FreeCoTaskMem(m_String);
	}

	CSString& CSString::operator=(const CSString& InOther)
	{
		if (InOther.m_String != nullptr)
			m_String = Memory::StringToCoTaskMemAuto(InOther.m_String);

		return *this;
	}

	CSString& CSString::operator=(CSString&& InOther) noexcept
	{
		m_String = std::exchange(InOther.m_String, nullptr);
		return *this;
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
