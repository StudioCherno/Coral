#pragma once

#include "Core.hpp"

namespace Coral {

	struct UnmanagedArray;
	enum class AssemblyLoadStatus;
	struct ObjectCreateInfo;
	enum class GCCollectionMode;
	enum class ManagedType;

	using ExceptionCallbackFn = void(*)(const CharType*);

	using SetInternalCallsFn = void(*)(UnmanagedArray*);
	using LoadManagedAssemblyFn = int32_t(*)(const CharType*);
	using UnloadAssemblyLoadContextFn = void(*)(int32_t);
	using GetLastLoadStatusFn = AssemblyLoadStatus(*)();
	using FreeManagedStringFn = void(*)(const CharType*);
	using CreateObjectFn = void*(*)(const ObjectCreateInfo*);
	using InvokeMethodFn = void(*)(void*, const CharType*, const ManagedType*, const void**, int32_t);
	using InvokeMethodRetFn = void(*)(void*, const CharType*, const ManagedType*, const void**, int32_t, void*, uint64_t, ManagedType);
	using DestroyObjectFn = void(*)(void*);

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
		InvokeMethodFn InvokeMethodFptr = nullptr;
		InvokeMethodRetFn InvokeMethodRetFptr = nullptr;
		DestroyObjectFn DestroyObjectFptr = nullptr;

		SetExceptionCallbackFn SetExceptionCallbackFptr = nullptr;

		CollectGarbageFn CollectGarbageFptr = nullptr;
		WaitForPendingFinalizersFn WaitForPendingFinalizersFptr = nullptr;
	};

	inline ManagedFunctions s_ManagedFunctions;

}
