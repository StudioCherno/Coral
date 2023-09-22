#include "FieldInfo.hpp"
#include "Type.hpp"
#include "Attribute.hpp"
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

	std::vector<Attribute> FieldInfo::GetAttributes() const
	{
		int32_t attributeCount;
		s_ManagedFunctions.GetFieldInfoAttributesFptr(&m_Handle, nullptr, &attributeCount);
		std::vector<ManagedHandle> attributeHandles(attributeCount);
		s_ManagedFunctions.GetFieldInfoAttributesFptr(&m_Handle, attributeHandles.data(), &attributeCount);

		std::vector<Attribute> result(attributeHandles.size());
		for (size_t i = 0; i < attributeHandles.size(); i++)
			result[i].m_Handle = attributeHandles[i];

		return result;
	}

}
