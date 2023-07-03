#pragma once

#include "Core.hpp"

namespace Coral {

	struct UnmanagedArray;
	enum class AssemblyLoadStatus;
	struct ObjectCreateInfo;

	using SetInternalCallsFn = void (*)(UnmanagedArray*);
	using LoadManagedAssemblyFn = int32_t(*)(const CharType*);
	using GetLastLoadStatusFn = AssemblyLoadStatus(*)();
	using FreeManagedStringFn = void (*)(const CharType*);
	using CreateObjectFn = void* (*)(const ObjectCreateInfo*);
	using DestroyObjectFn = void (*)(void*);

	struct ManagedFunctions
	{
		SetInternalCallsFn SetInternalCallsFptr = nullptr;
		LoadManagedAssemblyFn LoadManagedAssemblyFptr = nullptr;
		GetLastLoadStatusFn GetLastLoadStatusFptr = nullptr;
		FreeManagedStringFn FreeManagedStringFptr = nullptr;

		CreateObjectFn CreateObjectFptr = nullptr;
		DestroyObjectFn DestroyObjectFptr = nullptr;
	};

	inline static ManagedFunctions s_ManagedFunctions;

}
