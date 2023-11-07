#pragma once

#include "Core.hpp"
#include "NativeString.hpp"

namespace Coral {

	class Type;

	class Attribute
	{
	public:
		Type& GetType();

		template<typename TReturn>
		TReturn GetFieldValue(std::string_view InFieldName)
		{
			TReturn result;
			GetFieldValueInternal(InFieldName, &result);
			return result;
		}

	private:
		void GetFieldValueInternal(NativeString InFieldName, void* OutValue) const;

	private:
		ManagedHandle m_Handle = -1;
		Type* m_Type = nullptr;

		friend class Type;
		friend class MethodInfo;
		friend class FieldInfo;
		friend class PropertyInfo;
	};

}
