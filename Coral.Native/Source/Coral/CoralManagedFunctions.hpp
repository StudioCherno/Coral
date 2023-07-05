#pragma once

#include "Core.hpp"

namespace Coral {

	struct UnmanagedArray;
	enum class AssemblyLoadStatus;
	struct ObjectCreateInfo;
	enum class GCCollectionMode;

	using ExceptionCallbackFn = void(*)(const CharType*);

	using SetInternalCallsFn = void (*)(UnmanagedArray*);
	using LoadManagedAssemblyFn = int32_t(*)(const CharType*);
	using UnloadAssemblyLoadContextFn = void(*)(int32_t);
	using GetLastLoadStatusFn = AssemblyLoadStatus(*)();
	using FreeManagedStringFn = void (*)(const CharType*);
	using CreateObjectFn = void* (*)(const ObjectCreateInfo*);
	using DestroyObjectFn = void (*)(void*);

	using SetExceptionCallbackFn = void(*)(ExceptionCallbackFn);

	using CollectGarbageFn = void(*)(int32_t, GCCollectionMode, Bool32, Bool32);
	using WaitForPendingFinalizersFn = void(*)();

	struct ManagedFunctions
	{
		SetInternalCallsFn SetInternalCallsFptr = nullptr;
		LoadManagedAssemblyFn LoadManagedAssemblyFptr = nullptr;
		UnloadAssemblyLoadContextFn UnloadAssemblyLoadContextFptr = nullptr;
		GetLastLoadStatusFn GetLastLoadStatusFptr = nullptr;
		FreeManagedStringFn FreeManagedStringFptr = nullptr;

		CreateObjectFn CreateObjectFptr = nullptr;
		DestroyObjectFn DestroyObjectFptr = nullptr;

		SetExceptionCallbackFn SetExceptionCallbackFptr = nullptr;

		CollectGarbageFn CollectGarbageFptr = nullptr;
		WaitForPendingFinalizersFn WaitForPendingFinalizersFptr = nullptr;
	};

	inline ManagedFunctions s_ManagedFunctions;

}
