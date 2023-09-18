#include "ReflectionType.hpp"
#include "HostInstance.hpp"
#include "CoralManagedFunctions.hpp"

namespace Coral {

	ReflectionType& ReflectionType::GetBaseType()
	{
		return m_Host->GetReflectionType(BaseTypeName);
	}

	const std::vector<ManagedField>& ReflectionType::GetFields()
	{
		return m_Host->GetFields(AssemblyQualifiedName);
	}

	const std::vector<MethodInfo>& ReflectionType::GetMethods()
	{
		return m_Host->GetMethods(AssemblyQualifiedName);
	}
	
	bool ReflectionType::IsAssignableTo(const ReflectionType& InOther) const
	{
		return s_ManagedFunctions.IsTypeAssignableTo(AssemblyQualifiedName, InOther.AssemblyQualifiedName);
	}

	bool ReflectionType::IsAssignableFrom(const ReflectionType& InOther) const
	{
		return s_ManagedFunctions.IsTypeAssignableFrom(AssemblyQualifiedName, InOther.AssemblyQualifiedName);
	}

}
