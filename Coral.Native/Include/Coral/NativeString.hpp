#pragma once

#include "Core.hpp"

namespace Coral {

	struct StringData
	{
		CharType* String = nullptr;
		Bool32 IsDisposed = false;
	};

	class NativeString
	{
	public:
		NativeString() = default;
		NativeString(std::string_view InString);
		NativeString(const std::string& InString);
		NativeString(const char* InString);

		NativeString(StringData InData);

		NativeString(const NativeString& InOther);
		NativeString(NativeString&& InOther) noexcept;
		~NativeString();

		NativeString& operator=(const NativeString& InOther);
		NativeString& operator=(NativeString&& InOther) noexcept;

		void Assign(std::string_view InString);

		operator std::string() const;

		bool operator==(const NativeString& InOther) const;

		CharType* Data() { return m_Data.String; }
		const CharType* Data() const { return m_Data.String; }

	private:
		StringData m_Data;

	private:
		friend class HostInstance;
		friend class AssemblyLoadContext;
		friend class Assembly;
		friend class Type;
		friend class FieldInfo;
		friend class PropertyInfo;
		friend class MethodInfo;
		friend class Attribute;
		friend class ManagedObject;
		friend class Utility;
	};

}
