#pragma once

#include "Core.hpp"
#include "CSString.hpp"

namespace Coral {

	class HostInstance;
	class ManagedField;
	struct MethodInfo;
	
	class ReflectionType
	{
	public:
		CSString FullName;
		CSString Name;
		CSString Namespace;
		CSString BaseTypeName;
		CSString AssemblyQualifiedName;

		ReflectionType& GetBaseType();

		const std::vector<ManagedField>& GetFields();
		const std::vector<MethodInfo>& GetMethods();

		bool IsAssignableTo(const ReflectionType& InOther) const;
		bool IsAssignableFrom(const ReflectionType& InOther) const;

	private:
		HostInstance* m_Host = nullptr;

		friend class HostInstance;
		friend class AssemblyLoadContext;
	};
	
}

