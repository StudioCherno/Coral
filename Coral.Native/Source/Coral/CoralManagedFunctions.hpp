#pragma once

#include "Core.hpp"

namespace Coral {

	struct UnmanagedArray;
	enum class AssemblyLoadStatus;
	struct ObjectCreateInfo;
	class ManagedObject;
	enum class GCCollectionMode;
	enum class ManagedType;
	class ReflectionType;
	class ManagedField;
	struct MethodInfo;

	using SetInternalCallsFn = void(*)(UnmanagedArray*);
	using CreateAssemblyLoadContextFn = int32_t(*)(const CharType*);
	using UnloadAssemblyLoadContextFn = void(*)(int32_t);
	using LoadManagedAssemblyFn = int32_t(*)(int32_t, const CharType*);
	using GetLastLoadStatusFn = AssemblyLoadStatus(*)();
	using GetAssemblyNameFn = const CharType*(*)(int32_t);
	using QueryAssemblyTypesFn = void(*)(int32_t, ReflectionType*, int32_t*);
	using GetReflectionTypeFn = Bool32(*)(const CharType*, ReflectionType*);
	using GetReflectionTypeFromObjectFn = Bool32(*)(void*, ReflectionType*);
	using GetFieldsFn = void(*)(const CharType*, ManagedField*, int32_t*);
	using GetTypeMethodsFn = void(*)(const CharType*, MethodInfo*, int32_t*);
	using GetTypeIdFn = void(*)(const CharType*, TypeId*);
	
	using FreeManagedStringFn = void(*)(const CharType*);
	
	using CreateObjectFn = ManagedObject(*)(const ObjectCreateInfo*);
	using InvokeMethodFn = void(*)(void*, const CharType*, UnmanagedArray*);
	using InvokeMethodRetFn = void(*)(void*, const CharType*, UnmanagedArray*, void*);
	using SetFieldValueFn = void(*)(void*, const CharType*, void*);
	using GetFieldValueFn = void(*)(void*, const CharType*, void*);
	using SetPropertyValueFn = void(*)(void*, const CharType*, void*);
	using GetPropertyValueFn = void(*)(void*, const CharType*, void*);
	using DestroyObjectFn = void(*)(void*);

	using IsAssignableToFn = Bool32(*)(const CharType*, const CharType*);
	using IsAssignableFromFn = Bool32(*)(const CharType*, const CharType*);

	using SetExceptionCallbackFn = void(*)(void(*)(const CharType*));

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
		
		FreeManagedStringFn FreeManagedStringFptr = nullptr;

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
