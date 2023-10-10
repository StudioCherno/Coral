#include "Attribute.hpp"
#include "NativeString.hpp"
#include "Type.hpp"
#include "CoralManagedFunctions.hpp"
#include "TypeCache.hpp"

namespace Coral {

	Type& Attribute::GetType()
	{
		if (!m_Type)
		{
			Type type;
			s_ManagedFunctions.GetAttributeTypeFptr(&m_Handle, &type.m_TypePtr);
			type.RetrieveName();
			m_Type = TypeCache::Get().CacheType(std::move(type));
		}

		return *m_Type;
	}

	void Attribute::GetFieldValueInternal(std::string_view InFieldName, void* OutValue) const
	{
		auto fieldName = NativeString::FromUTF8(InFieldName);
		s_ManagedFunctions.GetAttributeFieldValueFptr(&m_Handle, fieldName, OutValue);
	}

}
