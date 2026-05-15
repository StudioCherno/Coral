#pragma once

#include "Core.hpp"
#include "String.hpp"

namespace Coral {

	class Type;
	class Attribute;

	class ConstructorInfo
	{
	public:
		String GetFriendlyName() const;

		TypeAccessibility GetAccessibility() const;

		std::vector<Attribute> GetAttributes() const;

	private:
		ManagedHandle m_Handle = -1;

		friend class Type;
	};

}
