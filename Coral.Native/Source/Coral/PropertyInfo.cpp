#include "PropertyInfo.hpp"
#include "Type.hpp"
#include "Attribute.hpp"
#include "CoralManagedFunctions.hpp"

namespace Coral {

	std::string PropertyInfo::GetName() const
	{
		return s_ManagedFunctions.GetPropertyInfoNameFptr(&m_Handle).ToString();
	}

	Type PropertyInfo::GetType() const
	{
		Type propertyType;
		s_ManagedFunctions.GetPropertyInfoTypeFptr(&m_Handle, &propertyType.m_TypePtr);
		propertyType.RetrieveName();
		return propertyType;
	}

	std::vector<Attribute> PropertyInfo::GetAttributes() const
	{
		int32_t attributeCount;
		s_ManagedFunctions.GetPropertyInfoAttributesFptr(&m_Handle, nullptr, &attributeCount);
		std::vector<ManagedHandle> attributeHandles(attributeCount);
		s_ManagedFunctions.GetPropertyInfoAttributesFptr(&m_Handle, attributeHandles.data(), &attributeCount);

		std::vector<Attribute> result(attributeHandles.size());
		for (size_t i = 0; i < attributeHandles.size(); i++)
			result[i].m_Handle = attributeHandles[i];

		return result;
	}

}
