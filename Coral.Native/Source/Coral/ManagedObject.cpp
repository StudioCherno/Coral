#include "ManagedObject.hpp"
#include "Assembly.hpp"
#include "CoralManagedFunctions.hpp"
#include "StringHelper.hpp"
#include "Type.hpp"

namespace Coral {

	void ManagedObject::InvokeMethodInternal(std::string_view InMethodName, const void** InParameters, const ManagedType* InParameterTypes, size_t InLength) const
	{
		s_ManagedFunctions.InvokeMethodFptr(m_Handle, InMethodName, InParameters, InParameterTypes, static_cast<int32_t>(InLength));
	}

	void ManagedObject::InvokeMethodRetInternal(std::string_view InMethodName, const void** InParameters, const ManagedType* InParameterTypes, size_t InLength, void* InResultStorage) const
	{
		s_ManagedFunctions.InvokeMethodRetFptr(m_Handle, InMethodName, InParameters, InParameterTypes, static_cast<int32_t>(InLength), InResultStorage);
	}

	void ManagedObject::SetFieldValueRaw(std::string_view InFieldName, void* InValue) const
	{
		s_ManagedFunctions.SetFieldValueFptr(m_Handle, InFieldName, InValue);
	}

	void ManagedObject::GetFieldValueRaw(std::string_view InFieldName, void* OutValue) const
	{
		s_ManagedFunctions.GetFieldValueFptr(m_Handle, InFieldName, OutValue);
	}

	void ManagedObject::SetPropertyValueRaw(std::string_view InPropertyName, void* InValue) const
	{
		s_ManagedFunctions.SetPropertyValueFptr(m_Handle, InPropertyName, InValue);
	}
	
	void ManagedObject::GetPropertyValueRaw(std::string_view InPropertyName, void* OutValue) const
	{
		s_ManagedFunctions.GetPropertyValueFptr(m_Handle, InPropertyName, OutValue);
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

