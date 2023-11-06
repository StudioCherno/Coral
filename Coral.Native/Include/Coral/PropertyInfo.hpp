#pragma once

#include "Core.hpp"

namespace Coral {

	class Type;
	class Attribute;

	class PropertyInfo
	{
	public:
		std::string GetName() const;
		Type& GetType();

		std::vector<Attribute> GetAttributes() const;

	private:
		ManagedHandle m_Handle = -1;
		Type* m_Type = nullptr;

		friend class Type;
	};
	
}
