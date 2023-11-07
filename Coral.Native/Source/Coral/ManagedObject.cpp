#include "ManagedObject.hpp"
#include "Assembly.hpp"
#include "CoralManagedFunctions.hpp"
#include "StringHelper.hpp"
#include "Type.hpp"

namespace Coral {

	void ManagedObject::InvokeMethodInternal(NativeString InMethodName, const void** InParameters, const ManagedType* InParameterTypes, size_t InLength) const
	{
		s_ManagedFunctions.InvokeMethodFptr(m_Handle, InMethodName.m_Data, InParameters, InParameterTypes, static_cast<int32_t>(InLength));
	}

	void ManagedObject::InvokeMethodRetInternal(NativeString InMethodName, const void** InParameters, const ManagedType* InParameterTypes, size_t InLength, void* InResultStorage) const
	{
		s_ManagedFunctions.InvokeMethodRetFptr(m_Handle, InMethodName.m_Data, InParameters, InParameterTypes, static_cast<int32_t>(InLength), InResultStorage);
	}

	void ManagedObject::SetFieldValueRaw(NativeString InFieldName, void* InValue) const
	{
		s_ManagedFunctions.SetFieldValueFptr(m_Handle, InFieldName.m_Data, InValue);
	}

	void ManagedObject::GetFieldValueRaw(NativeString InFieldName, void* OutValue) const
	{
		s_ManagedFunctions.GetFieldValueFptr(m_Handle, InFieldName.m_Data, OutValue);
	}

	void ManagedObject::SetPropertyValueRaw(NativeString InPropertyName, void* InValue) const
	{
		s_ManagedFunctions.SetPropertyValueFptr(m_Handle, InPropertyName.m_Data, InValue);
	}
	
	void ManagedObject::GetPropertyValueRaw(NativeString InPropertyName, void* OutValue) const
	{
		s_ManagedFunctions.GetPropertyValueFptr(m_Handle, InPropertyName.m_Data, OutValue);
	}

	const Type& ManagedObject::GetType() const
	{
		return *m_Type;
	}

	void ManagedObject::Destroy()
	{
		if (!m_Handle)
			return;

		s_ManagedFunctions.DestroyObjectFptr(m_Handle);
		m_Handle = nullptr;
		m_Type = nullptr;
	}

}

