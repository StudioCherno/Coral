#include "Type.hpp"
#include "CoralManagedFunctions.hpp"

namespace Coral {

	std::string Type::GetFullName() const
	{
		return m_Namespace + "." + m_Name;
	}

	std::string Type::GetAssemblyQualifiedName() const
	{
		return s_ManagedFunctions.GetAssemblyQualifiedNameFptr(&m_TypePtr).ToString();
	}

	Type Type::GetBaseType() const
	{
		Type baseType;
		s_ManagedFunctions.GetBaseTypeFptr(&m_TypePtr, &baseType.m_TypePtr);
		baseType.RetrieveName();
		return baseType;
	}

	bool Type::IsTypeAssignableTo(const Type& InOther)
	{
		return s_ManagedFunctions.IsTypeAssignableToFptr(&m_TypePtr, &InOther.m_TypePtr);
	}

	bool Type::IsTypeAssignableFrom(const Type& InOther)
	{
		return s_ManagedFunctions.IsTypeAssignableFromFptr(&m_TypePtr, &InOther.m_TypePtr);
	}

	bool Type::operator==(const Type& InOther) const
	{
		return m_TypePtr == InOther.m_TypePtr;
	}

	void Type::RetrieveName()
	{
		if (m_TypePtr == nullptr)
			return;

		auto fullName = s_ManagedFunctions.GetFullTypeNameFptr(&m_TypePtr).ToString();
		size_t namespaceEnd = fullName.find_last_of('.');

		if (namespaceEnd == std::string::npos)
		{
			m_Name = fullName;
			m_Namespace = "";
			return;
		}

		m_Name = fullName.substr(namespaceEnd + 1);
		m_Namespace = fullName.substr(0, namespaceEnd);
	}

	ManagedObject Type::CreateInstanceInternal(const void** InParameters, size_t InLength)
	{
		auto name = NativeString::FromUTF8(GetAssemblyQualifiedName());
		auto result = s_ManagedFunctions.CreateObjectFptr(name, false, InParameters, static_cast<int32_t>(InLength));
		result.m_Type = this;
		return result;
	}

}
