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

	NativeString::NativeString(std::string_view InString)
	{
		Assign(InString);
	}

	NativeString::NativeString(const std::string& InString)
	{
		Assign(InString);
	}

	NativeString::NativeString(const char* InString)
	{
		Assign(InString);
	}

	NativeString::~NativeString()
	{
		if (m_String != nullptr)
			Memory::FreeCoTaskMem(m_String);
	}

	void NativeString::Assign(std::string_view InString)
	{
		if (m_String != nullptr)
			Memory::FreeCoTaskMem(m_String);

		m_String = Memory::StringToCoTaskMemAuto(StringHelper::ConvertUtf8ToWide(InString));
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

	NativeString::operator std::string() const
	{
		StringView string(m_String);

#if defined(CORAL_WIDE_CHARS)
		return StringHelper::ConvertWideToUtf8(string);
#else
		return std::string(string);
#endif
	}

	bool NativeString::operator==(const NativeString& InOther) const
	{
#if defined(CORAL_WIDE_CHARS)
		return wcscmp(m_String, InOther.m_String) == 0;
#else
		return strcmp(m_String, InOther.m_String) == 0;
#endif
	}

}
