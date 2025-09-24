#pragma once

#include "Core.hpp"

namespace Coral {

	// TODO(Emily): Could this benefit from retaining a var for size?
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

		bool operator!=(const String& InOther) const;
		bool operator!=(std::string_view InOther) const;

		UCChar* Data() { return m_String; }
		const UCChar* Data() const { return m_String; }

	public:
		UCChar* m_String = nullptr;
		[[maybe_unused]] Bool32 m_IsDisposed = false; // NOTE(Peter): Required for the layout to match the C# NativeString struct, unused in C++
	};

	static_assert(offsetof(String, m_String) == 0);
	static_assert(offsetof(String, m_IsDisposed) == 8);
	static_assert(sizeof(String) == 16);

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

		operator std::string() const { return m_String; }
		operator String() const { return m_String; }

		bool operator==(const ScopedString& InOther) const
		{
			return m_String == InOther.m_String;
		}

		bool operator==(std::string_view InOther) const
		{
			return m_String == InOther;
		}

		bool operator!=(const ScopedString& InOther) const
		{
			return m_String != InOther.m_String;
		}

		bool operator!=(std::string_view InOther) const
		{
			return m_String != InOther;
		}

	private:
		String m_String;
	};

}
