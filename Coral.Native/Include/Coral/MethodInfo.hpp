#pragma once

#include "Core.hpp"
#include "NativeString.hpp"

namespace Coral {

	struct MethodInfo
	{
		NativeString Name;
		TypeVisibility Visibility;
	};

}
