#include "MethodInfo.hpp"
#include "CoralManagedFunctions.hpp"
#include "Type.hpp"

namespace Coral {

	std::string MethodInfo::GetName() const
	{
		return s_ManagedFunctions.GetMethodInfoNameFptr(&m_Handle).ToString();
	}

	Type MethodInfo::GetReturnType() const
	{
		Type returnType;
		s_ManagedFunctions.GetMethodInfoReturnTypeFptr(&m_Handle, &returnType.m_TypePtr);
		returnType.RetrieveName();
		return returnType;
	}

}
