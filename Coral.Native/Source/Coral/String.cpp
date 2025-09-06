#include "Coral/String.hpp"
#include "Coral/StringHelper.hpp"
#include "Coral/Memory.hpp"

#include "Verify.hpp"

namespace Coral {

	String String::New(const char* InString)
	{
		String result;
		result.Assign(InString);
		return result;
	}

	String String::New(std::string_view InString)
	{
		String result;
		result.Assign(InString);
		return result;
	}

	void String::Free(String& InString)
	{
		if (InString.m_String == nullptr)
			return;

		Memory::FreeCoTaskMem(InString.m_String);
		InString.m_String = nullptr;
	}

	void String::Assign(std::string_view InString)
	{
		if (m_String != nullptr)
			Memory::FreeCoTaskMem(m_String);

		m_String = Memory::StringToCoTaskMemAuto(StringHelper::ConvertUtf8ToWide(InString));
	}

	String::operator std::string() const
	{
		UCStringView string(m_String);

		return StringHelper::ConvertWideToUtf8(string);
	}

	bool String::operator==(const String& InOther) const
	{
		if (m_String == InOther.m_String)
		{
			return true;
		}

		if (m_String == nullptr || InOther.m_String == nullptr)
		{
			return false;
		}

#ifdef CORAL_WIDE_CHARS
		return wcscmp(m_String, InOther.m_String) == 0;
#else
		return strcmp(m_String, InOther.m_String) == 0;
#endif
	}

	bool String::operator==(std::string_view InOther) const
	{
		if (m_String == nullptr && InOther.empty())
		{
			return true;
		}

		if (m_String == nullptr)
			return false;

#ifdef CORAL_WIDE_CHARS
		auto str = StringHelper::ConvertUtf8ToWide(InOther);
		return wcscmp(m_String, str.data()) == 0;
#else
		return strcmp(m_String, InOther.data()) == 0;
#endif
	}

	bool String::operator!=(const String& InOther) const {
		return this->operator!=(InOther);
	}

	bool String::operator!=(std::string_view InOther) const {
		return this->operator!=(InOther);
	}
}
