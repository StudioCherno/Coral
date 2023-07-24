#pragma once

#include "Core.hpp"

namespace Coral {

	enum class TypeVisibility
	{
		Public,
		Private,
		Protected,
		Internal,
		ProtectedPublic,
		PrivateProtected
	};

	class ManagedField
	{
	public:
		const CharType* Name;
		TypeVisibility visibility;
	};
	
}
