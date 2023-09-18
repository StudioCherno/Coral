#pragma once

#include "Core.hpp"
#include "NativeString.hpp"

namespace Coral {

	struct UnmanagedArray;
	enum class AssemblyLoadStatus;
	class ManagedObject;
	enum class GCCollectionMode;
	enum class ManagedType;
	class ReflectionType;
	class ManagedField;
	struct MethodInfo;

	using SetInternalCallsFn = void(*)(void*, int32_t);
	using CreateAssemblyLoadContextFn = int32_t(*)(NativeString);
	using UnloadAssemblyLoadContextFn = void(*)(int32_t);
	using LoadManagedAssemblyFn = int32_t(*)(int32_t, NativeString);
	using GetLastLoadStatusFn = AssemblyLoadStatus(*)();
	using GetAssemblyNameFn = NativeString(*)(int32_t);
	using QueryAssemblyTypesFn = void(*)(int32_t, ReflectionType*, int32_t*);
	using GetReflectionTypeFn = Bool32(*)(NativeString, ReflectionType*);
	using GetReflectionTypeFromObjectFn = Bool32(*)(void*, ReflectionType*);
	using GetFieldsFn = void(*)(NativeString, ManagedField*, int32_t*);
	using GetTypeMethodsFn = void(*)(NativeString, MethodInfo*, int32_t*);
	using GetTypeIdFn = void(*)(NativeString, TypeId*);
	
	using CreateObjectFn = ManagedObject(*)(NativeString, Bool32, const void**, int32_t);
	using InvokeMethodFn = void(*)(void*, NativeString, const void**, int32_t);
	using InvokeMethodRetFn = void(*)(void*, NativeString, const void**, int32_t, void*);
	using SetFieldValueFn = void(*)(void*, NativeString, void*);
	using GetFieldValueFn = void(*)(void*, NativeString, void*);
	using SetPropertyValueFn = void(*)(void*, NativeString, void*);
	using GetPropertyValueFn = void(*)(void*, NativeString, void*);
	using DestroyObjectFn = void(*)(void*);

	using IsAssignableToFn = Bool32(*)(NativeString, NativeString);
	using IsAssignableFromFn = Bool32(*)(NativeString, NativeString);

	using SetExceptionCallbackFn = void(*)(void(*)(NativeString));

	using CollectGarbageFn = void(*)(int32_t, GCCollectionMode, Bool32, Bool32);
	using WaitForPendingFinalizersFn = void(*)();

	struct ManagedFunctions
	{
		SetInternalCallsFn SetInternalCallsFptr = nullptr;
		LoadManagedAssemblyFn LoadManagedAssemblyFptr = nullptr;
		UnloadAssemblyLoadContextFn UnloadAssemblyLoadContextFptr = nullptr;
		GetLastLoadStatusFn GetLastLoadStatusFptr = nullptr;
		GetAssemblyNameFn GetAssemblyNameFptr = nullptr;
		QueryAssemblyTypesFn QueryAssemblyTypesFptr = nullptr;
		GetReflectionTypeFn GetReflectionTypeFptr = nullptr;
		GetReflectionTypeFromObjectFn GetReflectionTypeFromObjectFptr = nullptr;
		GetFieldsFn GetFieldsFptr = nullptr;
		GetTypeMethodsFn GetTypeMethodsFptr = nullptr;
		GetTypeIdFn GetTypeIdFptr = nullptr;
		
		CreateObjectFn CreateObjectFptr = nullptr;
		CreateAssemblyLoadContextFn CreateAssemblyLoadContextFptr = nullptr;
		InvokeMethodFn InvokeMethodFptr = nullptr;
		InvokeMethodRetFn InvokeMethodRetFptr = nullptr;
		SetFieldValueFn SetFieldValueFptr = nullptr;
		GetFieldValueFn GetFieldValueFptr = nullptr;
		SetPropertyValueFn SetPropertyValueFptr = nullptr;
		GetPropertyValueFn GetPropertyValueFptr = nullptr;
		DestroyObjectFn DestroyObjectFptr = nullptr;

		IsAssignableToFn IsTypeAssignableTo = nullptr;
		IsAssignableFromFn IsTypeAssignableFrom = nullptr;

		SetExceptionCallbackFn SetExceptionCallbackFptr = nullptr;

		CollectGarbageFn CollectGarbageFptr = nullptr;
		WaitForPendingFinalizersFn WaitForPendingFinalizersFptr = nullptr;
	};

	inline ManagedFunctions s_ManagedFunctions;

}
