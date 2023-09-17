#pragma once

#include "Core.hpp"

namespace Coral {

	class CSString
	{
	public:
		CSString() = default;
		CSString(const CSString& InOther);
		CSString(CSString&& InOther) noexcept;
		~CSString();

		CSString& operator=(const CSString& InOther);
		CSString& operator=(CSString&& InOther) noexcept;

		static CSString FromUTF8(std::string_view InString);
		static std::string ToUTF8(CSString InString);

		std::string ToString() const;

		bool operator==(const CSString& InOther) const;

		CharType* Data() { return m_String; }
		const CharType* Data() const { return m_String; }

	private:
		CharType* m_String = nullptr;
	};

}
