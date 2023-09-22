#include "FieldInfo.hpp"
#include "Type.hpp"
#include "CoralManagedFunctions.hpp"

namespace Coral {

	std::string FieldInfo::GetName() const
	{
		return s_ManagedFunctions.GetFieldInfoNameFptr(&m_Handle).ToString();
	}

	Type FieldInfo::GetType() const
	{
		Type fieldType;
		s_ManagedFunctions.GetFieldInfoTypeFptr(&m_Handle, &fieldType.m_TypePtr);
		fieldType.RetrieveName();
		return fieldType;
	}

	TypeAccessibility FieldInfo::GetAccessibility() const
	{
		return s_ManagedFunctions.GetFieldInfoAccessibilityFptr(&m_Handle);
	}

}
