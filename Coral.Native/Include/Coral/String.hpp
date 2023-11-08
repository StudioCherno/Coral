#pragma once

#include "Core.hpp"

namespace Coral {

	class String
	{
	public:
		static String New(const CharType* InString);
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

}
