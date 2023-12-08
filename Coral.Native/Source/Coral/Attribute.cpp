#include "Attribute.hpp"
#include "Type.hpp"
#include "CoralManagedFunctions.hpp"
#include "TypeCache.hpp"
#include "String.hpp"

namespace Coral {

	Type& Attribute::GetType()
	{
		if (!m_Type)
		{
			Type type;
			s_ManagedFunctions.GetAttributeTypeFptr(m_Handle, &type.m_Id);
			m_Type = TypeCache::Get().CacheType(std::move(type));
		}

		return *m_Type;
	}

	void Attribute::GetFieldValueInternal(std::string_view InFieldName, void* OutValue) const
	{
		auto fieldName = String::New(InFieldName);
		s_ManagedFunctions.GetAttributeFieldValueFptr(m_Handle, fieldName, OutValue);
		String::Free(fieldName);
	}

}
