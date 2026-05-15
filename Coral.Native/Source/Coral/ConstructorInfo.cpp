#include "Coral/ConstructorInfo.hpp"
#include "Coral/Type.hpp"
#include "Coral/Attribute.hpp"
#include "Coral/TypeCache.hpp"

#include "CoralManagedFunctions.hpp"

namespace Coral {

	String ConstructorInfo::GetFriendlyName() const
	{
		return s_ManagedFunctions.GetConstructorInfoFriendlyNameFptr(m_Handle);
	}

	TypeAccessibility ConstructorInfo::GetAccessibility() const
	{
		return s_ManagedFunctions.GetConstructorInfoAccessibilityFptr(m_Handle);
	}

	std::vector<Attribute> ConstructorInfo::GetAttributes() const
	{
		int32_t attributeCount;
		s_ManagedFunctions.GetConstructorInfoAttributesFptr(m_Handle, nullptr, &attributeCount);

		std::vector<ManagedHandle> attributeHandles(static_cast<size_t>(attributeCount));
		s_ManagedFunctions.GetConstructorInfoAttributesFptr(m_Handle, attributeHandles.data(), &attributeCount);

		std::vector<Attribute> result(attributeHandles.size());
		for (size_t i = 0; i < attributeHandles.size(); i++)
			result[i].m_Handle = attributeHandles[i];

		return result;
	}

}
