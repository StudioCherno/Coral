#pragma once

#include "Core.hpp"

namespace Coral {

	class Type;
	class Attribute;

	class PropertyInfo
	{
	public:
		std::string GetName() const;
		Type GetType() const;

		std::vector<Attribute> GetAttributes() const;

	private:
		ManagedHandle m_Handle = nullptr;

		friend class Type;
	};
	
}
