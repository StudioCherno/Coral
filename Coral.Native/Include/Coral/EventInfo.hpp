#pragma once

#include "Core.hpp"
#include "String.hpp"

namespace Coral {

	class Type;
	class Attribute;

	class EventInfo
	{
	public:
		String GetName() const;
		Type& GetEventHandlerType();

		TypeAccessibility GetAccessibility() const;
		bool IsStatic() const;

		std::vector<Attribute> GetAttributes() const;

	private:
		ManagedHandle m_Handle = -1;
		Type* m_HandlerType = nullptr;

		friend class Type;
	};

}
