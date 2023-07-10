#include "ManagedObject.hpp"
#include "CoralManagedFunctions.hpp"
#include "StringHelper.hpp"

namespace Coral {

	void ManagedObject::InvokeMethodInternal(std::string_view InMethodName, ManagedType* InParameterTypes, const void** InParameters, size_t InLength) const
	{
		auto methodName = StringHelper::ConvertUtf8ToWide(InMethodName);
		s_ManagedFunctions.InvokeMethodFptr(m_Handle, methodName.c_str(), InParameterTypes, InParameters, static_cast<int32_t>(InLength));
	}

	void ManagedObject::InvokeMethodRetInternal(std::string_view InMethodName, ManagedType* InParameterTypes, const void** InParameters, size_t InLength, void* InResultStorage, uint64_t InResultSize, ManagedType InResultType) const
	{
		auto methodName = StringHelper::ConvertUtf8ToWide(InMethodName);
		s_ManagedFunctions.InvokeMethodRetFptr(m_Handle, methodName.c_str(), InParameterTypes, InParameters, static_cast<int32_t>(InLength), InResultStorage, InResultSize, InResultType);
	}
	
}

