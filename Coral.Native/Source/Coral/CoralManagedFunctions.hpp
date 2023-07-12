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
	using GetAssemblyNameFn = const CharType*(*)(int32_t);
	using FreeManagedStringFn = void(*)(const CharType*);
	using CreateObjectFn = void*(*)(const ObjectCreateInfo*);
	using InvokeMethodFn = void(*)(void*, const CharType*, UnmanagedArray*);
	using InvokeMethodRetFn = void(*)(void*, const CharType*, UnmanagedArray*, void*);
	using SetFieldValueFn = void(*)(void*, const CharType*, void*);
	using GetFieldValueFn = void(*)(void*, const CharType*, void*);
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
		GetAssemblyNameFn GetAssemblyNameFptr = nullptr;
		FreeManagedStringFn FreeManagedStringFptr = nullptr;

		CreateObjectFn CreateObjectFptr = nullptr;
		InvokeMethodFn InvokeMethodFptr = nullptr;
		InvokeMethodRetFn InvokeMethodRetFptr = nullptr;
		SetFieldValueFn SetFieldValueFptr = nullptr;
		GetFieldValueFn GetFieldValueFptr = nullptr;
		DestroyObjectFn DestroyObjectFptr = nullptr;

		SetExceptionCallbackFn SetExceptionCallbackFptr = nullptr;

		CollectGarbageFn CollectGarbageFptr = nullptr;
		WaitForPendingFinalizersFn WaitForPendingFinalizersFptr = nullptr;
	};

	inline ManagedFunctions s_ManagedFunctions;

}
