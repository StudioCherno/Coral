#pragma once

#include "Core.hpp"

namespace Coral {

	class HostInstance;
	class ManagedField;
	
	class ReflectionType
	{
	public:
		const CharType* FullName = nullptr;
		const CharType* Name = nullptr;
		const CharType* Namespace = nullptr;
		const CharType* BaseTypeName = nullptr;
		const CharType* AssemblyQualifiedName = nullptr;

		ReflectionType& GetBaseType();

		const std::vector<ManagedField>& GetFields();

	private:
		HostInstance* m_Host = nullptr;

		friend class HostInstance;
	};
	
}

