#include "ManagedObject.hpp"
#include "Assembly.hpp"
#include "CoralManagedFunctions.hpp"
#include "String.hpp"
#include "StringHelper.hpp"
#include "Type.hpp"
#include "TypeCache.hpp"

namespace Coral {

	void ManagedObject::InvokeMethodInternal(std::string_view InMethodName, const void** InParameters, const ManagedType* InParameterTypes, size_t InLength) const
	{
		// NOTE(Peter): If you get an exception in this function it's most likely because you're using a Native only debugger type in Visual Studio
		//				and it's catching a C# exception even though it shouldn't. I recommend switching the debugger type to Mixed (.NET Core)
		//				which should be the default for Hazelnut, or simply press "Continue" until it works.
		//				This is a problem with the Visual Studio debugger and nothing we can change.
		auto methodName = String::New(InMethodName);
		s_ManagedFunctions.InvokeMethodFptr(m_Handle, methodName, InParameters, InParameterTypes, static_cast<int32_t>(InLength));
		String::Free(methodName);
	}

	void ManagedObject::InvokeMethodRetInternal(std::string_view InMethodName, const void** InParameters, const ManagedType* InParameterTypes, size_t InLength, void* InResultStorage) const
	{
		auto methodName = String::New(InMethodName);
		s_ManagedFunctions.InvokeMethodRetFptr(m_Handle, methodName, InParameters, InParameterTypes, static_cast<int32_t>(InLength), InResultStorage);
		String::Free(methodName);
	}

	void ManagedObject::SetFieldValueRaw(std::string_view InFieldName, void* InValue) const
	{
		auto fieldName = String::New(InFieldName);
		s_ManagedFunctions.SetFieldValueFptr(m_Handle, fieldName, InValue);
		String::Free(fieldName);
	}

	void ManagedObject::GetFieldValueRaw(std::string_view InFieldName, void* OutValue) const
	{
		auto fieldName = String::New(InFieldName);
		s_ManagedFunctions.GetFieldValueFptr(m_Handle, fieldName, OutValue);
		String::Free(fieldName);
	}

	void ManagedObject::SetPropertyValueRaw(std::string_view InPropertyName, void* InValue) const
	{
		auto propertyName = String::New(InPropertyName);
		s_ManagedFunctions.SetPropertyValueFptr(m_Handle, propertyName, InValue);
		String::Free(propertyName);
	}
	
	void ManagedObject::GetPropertyValueRaw(std::string_view InPropertyName, void* OutValue) const
	{
		auto propertyName = String::New(InPropertyName);
		s_ManagedFunctions.GetPropertyValueFptr(m_Handle, propertyName, OutValue);
		String::Free(propertyName);
	}

	const Type& ManagedObject::GetType()
	{
		if (!m_Type)
		{
			Type type;
			s_ManagedFunctions.GetObjectTypeIdFptr(m_Handle, &type.m_Id);
			m_Type = TypeCache::Get().CacheType(std::move(type));
		}

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

