#pragma once

#include "Core.hpp"
#include "String.hpp"

namespace Coral {

	class Type;
	class Attribute;

	class MethodInfo
	{
	public:
		String GetName() const;

		Type& GetReturnType();
		const std::vector<Type*>& GetParameterTypes();

		TypeAccessibility GetAccessibility() const;

		std::vector<Attribute> GetAttributes() const;

	private:
		ManagedHandle m_Handle = -1;
		Type* m_ReturnType = nullptr;
		std::vector<Type*> m_ParameterTypes;

		friend class Type;
	};

}
