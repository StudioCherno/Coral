#include "FieldInfo.hpp"
#include "Type.hpp"
#include "Attribute.hpp"
#include "NativeCallables.generated.hpp"
#include "TypeCache.hpp"

namespace Coral {

	String FieldInfo::GetName() const
	{
		return s_NativeCallables.GetFieldInfoNameFptr(m_Handle);
	}

	Type& FieldInfo::GetType()
	{
		if (!m_Type)
		{
			Type fieldType;
			s_NativeCallables.GetFieldInfoTypeFptr(m_Handle, &fieldType.m_Id);
			m_Type = TypeCache::Get().CacheType(std::move(fieldType));
		}

		return *m_Type;
	}

	TypeAccessibility FieldInfo::GetAccessibility() const
	{
		return s_NativeCallables.GetFieldInfoAccessibilityFptr(m_Handle);
	}

	std::vector<Attribute> FieldInfo::GetAttributes() const
	{
		int32_t attributeCount;
		s_NativeCallables.GetFieldInfoAttributesFptr(m_Handle, nullptr, &attributeCount);
		std::vector<ManagedHandle> attributeHandles(attributeCount);
		s_NativeCallables.GetFieldInfoAttributesFptr(m_Handle, attributeHandles.data(), &attributeCount);

		std::vector<Attribute> result(attributeHandles.size());
		for (size_t i = 0; i < attributeHandles.size(); i++)
			result[i].m_Handle = attributeHandles[i];

		return result;
	}

}
