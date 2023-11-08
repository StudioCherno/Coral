#include "MethodInfo.hpp"
#include "CoralManagedFunctions.hpp"
#include "Type.hpp"
#include "Attribute.hpp"
#include "TypeCache.hpp"

namespace Coral {

	String MethodInfo::GetName() const
	{
		return s_ManagedFunctions.GetMethodInfoNameFptr(m_Handle);
	}

	Type& MethodInfo::GetReturnType()
	{
		if (!m_ReturnType)
		{
			Type returnType;
			s_ManagedFunctions.GetMethodInfoReturnTypeFptr(m_Handle, &returnType.m_Id);
			m_ReturnType = TypeCache::Get().CacheType(std::move(returnType));
		}

		return *m_ReturnType;
	}

	const std::vector<Type*>& MethodInfo::GetParameterTypes()
	{
		if (m_ParameterTypes.empty())
		{
			int32_t parameterCount;
			s_ManagedFunctions.GetMethodInfoParameterTypesFptr(m_Handle, nullptr, &parameterCount);
			std::vector<TypeId> parameterTypes(parameterCount);
			s_ManagedFunctions.GetMethodInfoParameterTypesFptr(m_Handle, parameterTypes.data(), &parameterCount);

			m_ParameterTypes.resize(parameterTypes.size());

			for (size_t i = 0; i < parameterTypes.size(); i++)
			{
				Type type;
				type.m_Id = parameterTypes[i];
				m_ParameterTypes[i] = TypeCache::Get().CacheType(std::move(type));
			}
		}

		return m_ParameterTypes;
	}

	TypeAccessibility MethodInfo::GetAccessibility() const
	{
		return s_ManagedFunctions.GetMethodInfoAccessibilityFptr(m_Handle);
	}

	std::vector<Attribute> MethodInfo::GetAttributes() const
	{
		int32_t attributeCount;
		s_ManagedFunctions.GetMethodInfoAttributesFptr(m_Handle, nullptr, &attributeCount);
		std::vector<ManagedHandle> attributeHandles(attributeCount);
		s_ManagedFunctions.GetMethodInfoAttributesFptr(m_Handle, attributeHandles.data(), &attributeCount);

		std::vector<Attribute> result(attributeHandles.size());
		for (size_t i = 0; i < attributeHandles.size(); i++)
			result[i].m_Handle = attributeHandles[i];

		return result;
	}

}
