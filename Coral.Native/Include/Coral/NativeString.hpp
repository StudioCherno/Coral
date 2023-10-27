#pragma once

#include "Core.hpp"

namespace Coral {

	class NativeString
	{
	public:
		NativeString() = default;
		NativeString(std::string_view InString);
		NativeString(const std::string& InString);
		NativeString(const char* InString);

		NativeString(const NativeString& InOther);
		NativeString(NativeString&& InOther) noexcept;
		~NativeString();

		NativeString& operator=(const NativeString& InOther);
		NativeString& operator=(NativeString&& InOther) noexcept;

		void Assign(std::string_view InString);

		operator std::string() const;

		bool operator==(const NativeString& InOther) const;

		CharType* Data() { return m_String; }
		const CharType* Data() const { return m_String; }

	private:
		CharType* m_String = nullptr;
		Bool32 m_IsDisposed = false;
	};

}
