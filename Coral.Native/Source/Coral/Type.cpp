#include "Type.hpp"
#include "CoralManagedFunctions.hpp"
#include "TypeCache.hpp"
#include "Attribute.hpp"

namespace Coral {

	String Type::GetFullName() const
	{
		return s_ManagedFunctions.GetFullTypeNameFptr(m_Id);
	}

	String Type::GetAssemblyQualifiedName() const
	{
		return s_ManagedFunctions.GetAssemblyQualifiedNameFptr(m_Id);
	}

	Type& Type::GetBaseType()
	{
		if (!m_BaseType)
		{
			Type baseType;
			s_ManagedFunctions.GetBaseTypeFptr(m_Id, &baseType.m_Id);
			m_BaseType = TypeCache::Get().CacheType(std::move(baseType));
		}

		return *m_BaseType;
	}

	int32_t Type::GetSize() const
	{
		return s_ManagedFunctions.GetTypeSizeFptr(m_Id);
	}

	bool Type::IsSubclassOf(const Type& InOther) const
	{
		return s_ManagedFunctions.IsTypeSubclassOfFptr(m_Id, InOther.m_Id);
	}

	bool Type::IsAssignableTo(const Type& InOther) const
	{
		return s_ManagedFunctions.IsTypeAssignableToFptr(m_Id, InOther.m_Id);
	}

	bool Type::IsAssignableFrom(const Type& InOther) const
	{
		return s_ManagedFunctions.IsTypeAssignableFromFptr(m_Id, InOther.m_Id);
	}

	std::vector<MethodInfo> Type::GetMethods() const
	{
		int32_t methodCount = 0;
		s_ManagedFunctions.GetTypeMethodsFptr(m_Id, nullptr, &methodCount);
		std::vector<ManagedHandle> handles(methodCount);
		s_ManagedFunctions.GetTypeMethodsFptr(m_Id, handles.data(), &methodCount);

		std::vector<MethodInfo> methods(handles.size());
		for (size_t i = 0; i < handles.size(); i++)
			methods[i].m_Handle = handles[i];

		return methods;
	}

	std::vector<FieldInfo> Type::GetFields() const
	{
		int32_t fieldCount = 0;
		s_ManagedFunctions.GetTypeFieldsFptr(m_Id, nullptr, &fieldCount);
		std::vector<ManagedHandle> handles(fieldCount);
		s_ManagedFunctions.GetTypeFieldsFptr(m_Id, handles.data(), &fieldCount);

		std::vector<FieldInfo> fields(handles.size());
		for (size_t i = 0; i < handles.size(); i++)
			fields[i].m_Handle = handles[i];

		return fields;
	}

	std::vector<PropertyInfo> Type::GetProperties() const
	{
		int32_t propertyCount = 0;
		s_ManagedFunctions.GetTypePropertiesFptr(m_Id, nullptr, &propertyCount);
		std::vector<ManagedHandle> handles(propertyCount);
		s_ManagedFunctions.GetTypePropertiesFptr(m_Id, handles.data(), &propertyCount);

		std::vector<PropertyInfo> properties(handles.size());
		for (size_t i = 0; i < handles.size(); i++)
			properties[i].m_Handle = handles[i];

		return properties;
	}

	bool Type::HasAttribute(const Type& InAttributeType) const
	{
		return s_ManagedFunctions.HasTypeAttributeFptr(m_Id, InAttributeType.m_Id);
	}

	std::vector<Attribute> Type::GetAttributes() const
	{
		int32_t attributeCount;
		s_ManagedFunctions.GetTypeAttributesFptr(m_Id, nullptr, &attributeCount);
		std::vector<ManagedHandle> attributeHandles(attributeCount);
		s_ManagedFunctions.GetTypeAttributesFptr(m_Id, attributeHandles.data(), &attributeCount);

		std::vector<Attribute> result(attributeHandles.size());
		for (size_t i = 0; i < attributeHandles.size(); i++)
			result[i].m_Handle = attributeHandles[i];

		return result;
	}

	ManagedType Type::GetManagedType() const
	{
		return s_ManagedFunctions.GetTypeManagedTypeFptr(m_Id);
	}

	bool Type::IsSZArray() const
	{
		return s_ManagedFunctions.IsTypeSZArrayFptr(m_Id);
	}

	Type& Type::GetElementType()
	{
		if (!m_ElementType)
		{
			Type elementType;
			s_ManagedFunctions.GetElementTypeFptr(m_Id, &elementType.m_Id);
			m_ElementType = TypeCache::Get().CacheType(std::move(elementType));
		}

		return *m_ElementType;
	}

	bool Type::operator==(const Type& InOther) const
	{
		return m_Id == InOther.m_Id;
	}

	ManagedObject Type::CreateInstanceInternal(const void** InParameters, const ManagedType* InParameterTypes, size_t InLength) const
	{
		ManagedObject result;
		result.m_Handle = s_ManagedFunctions.CreateObjectFptr(m_Id, false, InParameters, InParameterTypes, static_cast<int32_t>(InLength));
		result.m_Type = this;
		return result;
	}

	void Type::InvokeStaticMethodInternal(std::string_view InMethodName, const void** InParameters, const ManagedType* InParameterTypes, size_t InLength) const
	{
		auto methodName = String::New(InMethodName);
		s_ManagedFunctions.InvokeStaticMethodFptr(m_Id, methodName, InParameters, InParameterTypes, static_cast<int32_t>(InLength));
		String::Free(methodName);
	}

	void Type::InvokeStaticMethodRetInternal(std::string_view InMethodName, const void** InParameters, const ManagedType* InParameterTypes, size_t InLength, void* InResultStorage) const
	{
		auto methodName = String::New(InMethodName);
		s_ManagedFunctions.InvokeStaticMethodRetFptr(m_Id, methodName, InParameters, InParameterTypes, static_cast<int32_t>(InLength), InResultStorage);
		String::Free(methodName);
	}


	ReflectionType::operator Type&() const
	{
		static Type s_NullType;

		auto* result = TypeCache::Get().GetTypeByID(m_TypeID);

		if (result == nullptr)
		{
			Type type;
			type.m_Id = m_TypeID;
			result = TypeCache::Get().CacheType(std::move(type));
		}

		return result != nullptr ? *result : s_NullType;
	}

}
