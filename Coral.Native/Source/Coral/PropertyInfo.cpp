#include "Coral/PropertyInfo.hpp"
#include "Coral/Type.hpp"
#include "Coral/Attribute.hpp"
#include "Coral/TypeCache.hpp"

#include "CoralManagedFunctions.hpp"

namespace Coral {

	String PropertyInfo::GetName() const
	{
		return s_ManagedFunctions.GetPropertyInfoNameFptr(m_Handle);
	}

	Type& PropertyInfo::GetType()
	{
		if (!m_Type)
		{
			Type propertyType;
			s_ManagedFunctions.GetPropertyInfoTypeFptr(m_Handle, &propertyType.m_Id);
			m_Type = TypeCache::Get().CacheType(std::move(propertyType));
		}

		return *m_Type;
	}

	bool PropertyInfo::HasGetter() const
	{
		return s_ManagedFunctions.GetPropertyInfoHasGetterFptr(m_Handle);
	}

	bool PropertyInfo::HasSetter() const
	{
		return s_ManagedFunctions.GetPropertyInfoHasSetterFptr(m_Handle);
	}

	TypeAccessibility PropertyInfo::GetGetterAccessibility() const
	{
		return s_ManagedFunctions.GetPropertyInfoGetterAccessibilityFptr(m_Handle);
	}

	TypeAccessibility PropertyInfo::GetSetterAccessibility() const
	{
		return s_ManagedFunctions.GetPropertyInfoSetterAccessibilityFptr(m_Handle);
	}

	bool PropertyInfo::IsStatic() const
	{
		return s_ManagedFunctions.GetPropertyInfoIsStaticFptr(m_Handle);
	}

	std::vector<Attribute> PropertyInfo::GetAttributes() const
	{
		int32_t attributeCount;
		s_ManagedFunctions.GetPropertyInfoAttributesFptr(m_Handle, nullptr, &attributeCount);

		std::vector<ManagedHandle> attributeHandles(static_cast<size_t>(attributeCount));
		s_ManagedFunctions.GetPropertyInfoAttributesFptr(m_Handle, attributeHandles.data(), &attributeCount);

		std::vector<Attribute> result(attributeHandles.size());
		for (size_t i = 0; i < attributeHandles.size(); i++)
			result[i].m_Handle = attributeHandles[i];

		return result;
	}

}
