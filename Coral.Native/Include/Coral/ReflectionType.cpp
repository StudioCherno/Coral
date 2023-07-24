#include "ReflectionType.hpp"
#include "HostInstance.hpp"

namespace Coral {

	ReflectionType& ReflectionType::GetBaseType()
	{
		return m_Host->GetReflectionType(BaseTypeName);
	}

	const std::vector<ManagedField>& ReflectionType::GetFields()
	{
		return m_Host->GetFields(AssemblyQualifiedName);
	}
	
}
