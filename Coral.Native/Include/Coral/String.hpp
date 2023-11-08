#pragma once

#include "Core.hpp"

namespace Coral {

	class String
	{
	public:
		static String New(const char* InString);
		static String New(std::string_view InString);
		static void Free(String& InString);

		void Assign(std::string_view InString);

		operator std::string() const;

		bool operator==(const String& InOther) const;
		bool operator==(std::string_view InOther) const;

		CharType* Data() { return m_String; }
		const CharType* Data() const { return m_String; }

	private:
		CharType* m_String = nullptr;
		Bool32 m_IsDisposed = false; // TODO(Peter): Try getting rid of this and simply aligning this m_String to 12 bytes
	};

	struct ScopedString
	{
		ScopedString(String InString)
		    : m_String(InString) {}

		ScopedString& operator=(String InOther)
		{
			String::Free(m_String);
			m_String = InOther;
			return *this;
		}

		ScopedString& operator=(const ScopedString& InOther)
		{
			String::Free(m_String);
			m_String = InOther.m_String;
			return *this;
		}

		~ScopedString()
		{
			String::Free(m_String);
		}

		operator String() const { return m_String; }

		bool operator==(const ScopedString& InOther) const
		{
			return m_String == InOther.m_String;
		}

		bool operator==(std::string_view InOther) const
		{
			return m_String == InOther;
		}

	private:
		String m_String;
	};

}
