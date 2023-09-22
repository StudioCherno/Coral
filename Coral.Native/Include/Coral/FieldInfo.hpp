#pragma once

#include "Core.hpp"

namespace Coral {

	class Type;

	class FieldInfo
	{
	public:
		std::string GetName() const;
		Type GetType() const;

		TypeAccessibility GetAccessibility() const;

	private:
		ManagedHandle m_Handle = nullptr;

		friend class Type;
	};
	
}
