#pragma once

#include "Core.hpp"

namespace Coral {

	class Type;
	class Attribute;

	class FieldInfo
	{
	public:
		std::string GetName() const;
		Type& GetType();

		TypeAccessibility GetAccessibility() const;

		std::vector<Attribute> GetAttributes() const;

	private:
		ManagedHandle m_Handle = -1;
		Type* m_Type = nullptr;

		friend class Type;
	};
	
}
