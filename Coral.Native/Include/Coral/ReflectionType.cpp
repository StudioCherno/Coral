#include "ReflectionType.hpp"
#include "HostInstance.hpp"

namespace Coral {

	ReflectionType& ReflectionType::GetBaseType()
	{
		return m_Host->GetReflectionType(BaseTypeName);
	}
	
}
