#pragma once

#include "Core.hpp"

namespace Coral {

	class CSString
	{
	public:
		~CSString();

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
