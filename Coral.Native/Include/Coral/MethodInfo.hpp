#pragma once

#include "Core.hpp"

namespace Coral {

	class Type;

	class MethodInfo
	{
	public:
		std::string GetName() const;
		Type GetReturnType() const;


	private:
		ManagedHandle m_Handle = nullptr;

		friend class Type;
	};

}
