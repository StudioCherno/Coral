#pragma once

#include "Core.hpp"

namespace Coral {

	class NativeString
	{
	public:
		NativeString() = default;
		NativeString(const NativeString& InOther);
		NativeString(NativeString&& InOther) noexcept;
		~NativeString();

		NativeString& operator=(const NativeString& InOther);
		NativeString& operator=(NativeString&& InOther) noexcept;

		static NativeString FromUTF8(std::string_view InString);
		static std::string ToUTF8(NativeString InString);

		std::string ToString() const;

		bool operator==(const NativeString& InOther) const;

		CharType* Data() { return m_String; }
		const CharType* Data() const { return m_String; }

	private:
		CharType* m_String = nullptr;
		Bool32 m_IsDisposed = false;
	};

}
