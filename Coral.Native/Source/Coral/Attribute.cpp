#include "Attribute.hpp"
#include "Type.hpp"
#include "NativeCallables.generated.hpp"
#include "TypeCache.hpp"
#include "String.hpp"

namespace Coral {

	Type& Attribute::GetType()
	{
		if (!m_Type)
		{
			Type type;
			s_NativeCallables.GetAttributeTypeFptr(m_Handle, &type.m_Id);
			m_Type = TypeCache::Get().CacheType(std::move(type));
		}

		return *m_Type;
	}

	void Attribute::GetFieldValueInternal(std::string_view InFieldName, void* OutValue) const
	{
		auto fieldName = String::New(InFieldName);
		s_NativeCallables.GetAttributeFieldValueFptr(m_Handle, fieldName, OutValue);
		String::Free(fieldName);
	}

}
