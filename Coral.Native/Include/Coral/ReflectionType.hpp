#pragma once

#include "Core.hpp"
#include "NativeString.hpp"

namespace Coral {

	class HostInstance;
	class ManagedField;
	struct MethodInfo;
	
	class ReflectionType
	{
	public:
		NativeString FullName;
		NativeString Name;
		NativeString Namespace;
		NativeString BaseTypeName;
		NativeString AssemblyQualifiedName;

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

