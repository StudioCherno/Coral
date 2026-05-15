#include "Coral/EventInfo.hpp"
#include "Coral/Type.hpp"
#include "Coral/Attribute.hpp"
#include "Coral/TypeCache.hpp"

#include "CoralManagedFunctions.hpp"

namespace Coral {

	String EventInfo::GetName() const
	{
		return s_ManagedFunctions.GetEventInfoNameFptr(m_Handle);
	}

	Type& EventInfo::GetEventHandlerType()
	{
		if (!m_HandlerType)
		{
			Type handlerType;
			s_ManagedFunctions.GetEventInfoHandlerTypeFptr(m_Handle, &handlerType.m_Id);
			m_HandlerType = TypeCache::Get().CacheType(std::move(handlerType));
		}

		return *m_HandlerType;
	}

	TypeAccessibility EventInfo::GetAccessibility() const
	{
		return s_ManagedFunctions.GetEventInfoAccessibilityFptr(m_Handle);
	}

	bool EventInfo::IsStatic() const
	{
		return s_ManagedFunctions.GetEventInfoIsStaticFptr(m_Handle);
	}

	std::vector<Attribute> EventInfo::GetAttributes() const
	{
		int32_t attributeCount;
		s_ManagedFunctions.GetEventInfoAttributesFptr(m_Handle, nullptr, &attributeCount);

		std::vector<ManagedHandle> attributeHandles(static_cast<size_t>(attributeCount));
		s_ManagedFunctions.GetEventInfoAttributesFptr(m_Handle, attributeHandles.data(), &attributeCount);

		std::vector<Attribute> result(attributeHandles.size());
		for (size_t i = 0; i < attributeHandles.size(); i++)
			result[i].m_Handle = attributeHandles[i];

		return result;
	}

}
