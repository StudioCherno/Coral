#include "NativeString.hpp"
#include "StringHelper.hpp"
#include "Memory.hpp"
#include "Verify.hpp"

namespace Coral {

	NativeString::NativeString(const NativeString& InOther)
	{
		if (InOther.m_Data.String != nullptr)
			m_Data.String = Memory::StringToCoTaskMemAuto(InOther.m_Data.String);
	}

	NativeString::NativeString(NativeString&& InOther) noexcept
	{
		m_Data.String = std::exchange(InOther.m_Data.String, nullptr);
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

	NativeString::NativeString(StringData InData)
	    : m_Data(std::move(InData))
	{
	}

	NativeString::~NativeString()
	{
		if (m_Data.String != nullptr)
			Memory::FreeCoTaskMem(m_Data.String);
	}

	void NativeString::Assign(std::string_view InString)
	{
		if (m_Data.String != nullptr)
			Memory::FreeCoTaskMem(m_Data.String);

		m_Data.String = Memory::StringToCoTaskMemAuto(StringHelper::ConvertUtf8ToWide(InString));
	}

	NativeString& NativeString::operator=(const NativeString& InOther)
	{
		if (InOther.m_Data.String != nullptr)
			m_Data.String = Memory::StringToCoTaskMemAuto(InOther.m_Data.String);

		return *this;
	}

	NativeString& NativeString::operator=(NativeString&& InOther) noexcept
	{
		m_Data.String = std::exchange(InOther.m_Data.String, nullptr);
		return *this;
	}

	NativeString::operator std::string() const
	{
		StringView string(m_Data.String);

#if defined(CORAL_WIDE_CHARS)
		return StringHelper::ConvertWideToUtf8(string);
#else
		return std::string(string);
#endif
	}

	bool NativeString::operator==(const NativeString& InOther) const
	{
#if defined(CORAL_WIDE_CHARS)
		return wcscmp(m_Data.String, InOther.m_Data.String) == 0;
#else
		return strcmp(m_Data.String, InOther.m_Data.String) == 0;
#endif
	}

}
