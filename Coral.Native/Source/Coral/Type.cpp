#include "Type.hpp"
#include "CoralManagedFunctions.hpp"
#include "TypeCache.hpp"
#include "Attribute.hpp"

namespace Coral {

	String Type::GetFullName() const
	{
		return s_ManagedFunctions.GetFullTypeNameFptr(m_TypePtr);
	}

	String Type::GetAssemblyQualifiedName() const
	{
		return s_ManagedFunctions.GetAssemblyQualifiedNameFptr(m_TypePtr);
	}

	Type& Type::GetBaseType()
	{
		if (!m_BaseType)
		{
			Type baseType;
			s_ManagedFunctions.GetBaseTypeFptr(m_TypePtr, &baseType.m_TypePtr);
			m_BaseType = TypeCache::Get().CacheType(std::move(baseType));
		}

		return *m_BaseType;
	}

	bool Type::IsSubclassOf(const Type& InOther)
	{
		return s_ManagedFunctions.IsTypeSubclassOfFptr(m_TypePtr, InOther.m_TypePtr);
	}

	bool Type::IsAssignableTo(const Type& InOther)
	{
		return s_ManagedFunctions.IsTypeAssignableToFptr(m_TypePtr, InOther.m_TypePtr);
	}

	bool Type::IsAssignableFrom(const Type& InOther)
	{
		return s_ManagedFunctions.IsTypeAssignableFromFptr(m_TypePtr, InOther.m_TypePtr);
	}

	std::vector<MethodInfo> Type::GetMethods() const
	{
		int32_t methodCount = 0;
		s_ManagedFunctions.GetTypeMethodsFptr(m_TypePtr, nullptr, &methodCount);
		std::vector<ManagedHandle> handles(methodCount);
		s_ManagedFunctions.GetTypeMethodsFptr(m_TypePtr, handles.data(), &methodCount);

		std::vector<MethodInfo> methods(handles.size());
		for (size_t i = 0; i < handles.size(); i++)
			methods[i].m_Handle = handles[i];

		return methods;
	}

	std::vector<FieldInfo> Type::GetFields() const
	{
		int32_t fieldCount = 0;
		s_ManagedFunctions.GetTypeFieldsFptr(m_TypePtr, nullptr, &fieldCount);
		std::vector<ManagedHandle> handles(fieldCount);
		s_ManagedFunctions.GetTypeFieldsFptr(m_TypePtr, handles.data(), &fieldCount);

		std::vector<FieldInfo> fields(handles.size());
		for (size_t i = 0; i < handles.size(); i++)
			fields[i].m_Handle = handles[i];

		return fields;
	}

	std::vector<PropertyInfo> Type::GetProperties() const
	{
		int32_t propertyCount = 0;
		s_ManagedFunctions.GetTypePropertiesFptr(m_TypePtr, nullptr, &propertyCount);
		std::vector<ManagedHandle> handles(propertyCount);
		s_ManagedFunctions.GetTypePropertiesFptr(m_TypePtr, handles.data(), &propertyCount);

		std::vector<PropertyInfo> properties(handles.size());
		for (size_t i = 0; i < handles.size(); i++)
			properties[i].m_Handle = handles[i];

		return properties;
	}

	std::vector<Attribute> Type::GetAttributes() const
	{
		int32_t attributeCount;
		s_ManagedFunctions.GetTypeAttributesFptr(m_TypePtr, nullptr, &attributeCount);
		std::vector<ManagedHandle> attributeHandles(attributeCount);
		s_ManagedFunctions.GetTypeAttributesFptr(m_TypePtr, attributeHandles.data(), &attributeCount);

		std::vector<Attribute> result(attributeHandles.size());
		for (size_t i = 0; i < attributeHandles.size(); i++)
			result[i].m_Handle = attributeHandles[i];

		return result;
	}

	ManagedType Type::GetManagedType() const
	{
		return s_ManagedFunctions.GetTypeManagedTypeFptr(m_TypePtr);
	}

	bool Type::operator==(const Type& InOther) const
	{
		return m_TypePtr == InOther.m_TypePtr;
	}

	ManagedObject Type::CreateInstanceInternal(const void** InParameters, const ManagedType* InParameterTypes, size_t InLength)
	{
		ManagedObject result;
		result.m_Handle = s_ManagedFunctions.CreateObjectFptr(m_TypePtr, false, InParameters, InParameterTypes, static_cast<int32_t>(InLength));
		result.m_Type = this;
		return result;
	}

	void Type::InvokeStaticMethodInternal(std::string_view InMethodName, const void** InParameters, const ManagedType* InParameterTypes, size_t InLength) const
	{
		auto methodName = String::New(InMethodName);
		s_ManagedFunctions.InvokeStaticMethodFptr(m_TypePtr, methodName, InParameters, InParameterTypes, static_cast<int32_t>(InLength));
		String::Free(methodName);
	}

	void Type::InvokeStaticMethodRetInternal(std::string_view InMethodName, const void** InParameters, const ManagedType* InParameterTypes, size_t InLength, void* InResultStorage) const
	{
		auto methodName = String::New(InMethodName);
		s_ManagedFunctions.InvokeStaticMethodRetFptr(m_TypePtr, methodName, InParameters, InParameterTypes, static_cast<int32_t>(InLength), InResultStorage);
		String::Free(methodName);
	}


}
