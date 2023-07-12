#pragma once

#include "Core.hpp"

namespace Coral {

	class HostInstance;
	
	class ReflectionType
	{
	public:
		const CharType* FullName;
		const CharType* Name;
		const CharType* Namespace;
		const CharType* BaseTypeName;

		ReflectionType& GetBaseType();

	private:
		HostInstance* m_Host = nullptr;

		friend class HostInstance;
	};
	
}

