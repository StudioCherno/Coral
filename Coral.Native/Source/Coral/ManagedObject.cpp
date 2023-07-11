#include "ManagedObject.hpp"
#include "CoralManagedFunctions.hpp"
#include "StringHelper.hpp"
#include "Interop.hpp"

namespace Coral {

	void ManagedObject::InvokeMethodInternal(std::string_view InMethodName, const void** InParameters, size_t InLength) const
	{
		auto methodName = StringHelper::ConvertUtf8ToWide(InMethodName);
		UnmanagedArray parameterArray = { InParameters, int32_t(InLength) };
		s_ManagedFunctions.InvokeMethodFptr(m_Handle, methodName.c_str(), &parameterArray);
	}

	void ManagedObject::InvokeMethodRetInternal(std::string_view InMethodName, const void** InParameters, size_t InLength, void* InResultStorage) const
	{
		auto methodName = StringHelper::ConvertUtf8ToWide(InMethodName);
		UnmanagedArray parameterArray = { InParameters, int32_t(InLength) };
		s_ManagedFunctions.InvokeMethodRetFptr(m_Handle, methodName.c_str(), &parameterArray, InResultStorage);
	}
	
}

