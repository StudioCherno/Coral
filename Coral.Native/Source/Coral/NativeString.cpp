#include "NativeString.hpp"
#include "StringHelper.hpp"
#include "Memory.hpp"
#include "Verify.hpp"

namespace Coral {

	NativeString::NativeString(const NativeString& InOther)
	{
		if (InOther.m_String != nullptr)
			m_String = Memory::StringToCoTaskMemAuto(InOther.m_String);
	}

	NativeString::NativeString(NativeString&& InOther) noexcept
	{
		m_String = std::exchange(InOther.m_String, nullptr);
	}

	NativeString::~NativeString()
	{
		if (m_String != nullptr)
			Memory::FreeCoTaskMem(m_String);
	}

	NativeString& NativeString::operator=(const NativeString& InOther)
	{
		if (InOther.m_String != nullptr)
			m_String = Memory::StringToCoTaskMemAuto(InOther.m_String);

		return *this;
	}

	NativeString& NativeString::operator=(NativeString&& InOther) noexcept
	{
		m_String = std::exchange(InOther.m_String, nullptr);
		return *this;
	}

	NativeString NativeString::FromUTF8(std::string_view InString)
	{
		NativeString result;

#if defined(CORAL_WIDE_CHARS)
		auto str = StringHelper::ConvertUtf8ToWide(InString);
		result.m_String = Memory::StringToCoTaskMemAuto(str);
#else
		result.m_String = Memory::StringToCoTaskMemAuto(InString);
#endif

		return result;
	}

	std::string NativeString::ToUTF8(NativeString InString)
	{
		StringView string(InString.m_String);

#if defined(CORAL_WIDE_CHARS)
		return StringHelper::ConvertWideToUtf8(string);
#else
		return std::string(string);
#endif
	}

	std::string NativeString::ToString() const { return ToUTF8(*this); }

	bool NativeString::operator==(const NativeString& InOther) const
	{
#if defined(CORAL_WIDE_CHARS)
		return wcscmp(m_String, InOther.m_String) == 0;
#else
		return strcmp(m_String, InOther.m_String) == 0;
#endif
	}

}
