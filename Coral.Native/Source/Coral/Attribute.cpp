#include "Attribute.hpp"
#include "NativeString.hpp"
#include "Type.hpp"
#include "CoralManagedFunctions.hpp"

namespace Coral {

	Type Attribute::GetType() const
	{
		Type type;
		s_ManagedFunctions.GetAttributeTypeFptr(&m_Handle, &type.m_TypePtr);
		type.RetrieveName();
		return type;
	}

	void Attribute::GetFieldValueInternal(std::string_view InFieldName, void* OutValue) const
	{
		auto fieldName = NativeString::FromUTF8(InFieldName);
		s_ManagedFunctions.GetAttributeFieldValueFptr(&m_Handle, fieldName, OutValue);
	}

}
