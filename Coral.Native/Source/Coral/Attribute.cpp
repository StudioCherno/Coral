#include "Attribute.hpp"
#include "Type.hpp"
#include "CoralManagedFunctions.hpp"
#include "TypeCache.hpp"

namespace Coral {

	Type& Attribute::GetType()
	{
		if (!m_Type)
		{
			Type type;
			s_ManagedFunctions.GetAttributeTypeFptr(m_Handle, &type.m_TypePtr);
			type.RetrieveName();
			m_Type = TypeCache::Get().CacheType(std::move(type));
		}

		return *m_Type;
	}

	void Attribute::GetFieldValueInternal(NativeString InFieldName, void* OutValue) const
	{
		s_ManagedFunctions.GetAttributeFieldValueFptr(m_Handle, InFieldName.m_Data, OutValue);
	}

}
