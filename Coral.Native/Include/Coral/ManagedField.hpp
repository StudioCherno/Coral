#pragma once

#include "Core.hpp"
#include "NativeString.hpp"

namespace Coral {

	class ManagedField
	{
	public:
		NativeString Name;
		TypeVisibility visibility;
	};
	
}
