#pragma once

#include "Core.hpp"
#include "NativeString.hpp"

namespace Coral {

	struct UnmanagedArray;
	enum class AssemblyLoadStatus;
	class ManagedObject;
	enum class GCCollectionMode;
	enum class ManagedType;
	class ManagedField;

	using SetInternalCallsFn = void(*)(void*, int32_t);
	using CreateAssemblyLoadContextFn = int32_t(*)(NativeString);
	using UnloadAssemblyLoadContextFn = void(*)(int32_t);
	using LoadManagedAssemblyFn = int32_t(*)(int32_t, NativeString);
	using GetLastLoadStatusFn = AssemblyLoadStatus(*)();
	using GetAssemblyNameFn = NativeString(*)(int32_t);
	/*
	using GetFieldsFn = void(*)(NativeString, ManagedField*, int32_t*);
	using GetTypeMethodsFn = void(*)(NativeString, MethodInfo*, int32_t*);
	*/

#pragma region TypeInterface

	using GetAssemblyTypesFn = void(*)(int32_t, TypeId*, int32_t*);
	using GetTypeIdFn = void(*)(NativeString, TypeId*);
	using GetFullTypeNameFn = NativeString(*)(const TypeId*);
	using GetAssemblyQualifiedNameFn = NativeString(*)(const TypeId*);
	using GetBaseTypeFn = void(*)(const TypeId*, TypeId*);
	using IsTypeAssignableToFn = Bool32(*)(const TypeId*, const TypeId*);
	using IsTypeAssignableFromFn = Bool32(*)(const TypeId*, const TypeId*);
	using GetTypeMethodsFn = void(*)(const TypeId*, ManagedHandle*, int32_t*);

#pragma endregion

#pragma region MethodInfo
	using GetMethodInfoNameFn = NativeString(*)(const ManagedHandle*);
	using GetMethodInfoReturnTypeFn = void(*)(const ManagedHandle*, TypeId*);
	using GetMethodInfoParameterTypesFn = void(*)(const ManagedHandle*, TypeId*, int32_t*);
#pragma endregion
	
	using CreateObjectFn = ManagedObject(*)(NativeString, Bool32, const void**, int32_t);
	using InvokeMethodFn = void(*)(void*, NativeString, const void**, int32_t);
	using InvokeMethodRetFn = void(*)(void*, NativeString, const void**, int32_t, void*);
	using SetFieldValueFn = void(*)(void*, NativeString, void*);
	using GetFieldValueFn = void(*)(void*, NativeString, void*);
	using SetPropertyValueFn = void(*)(void*, NativeString, void*);
	using GetPropertyValueFn = void(*)(void*, NativeString, void*);
	using DestroyObjectFn = void(*)(void*);

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
		/*
		GetFieldsFn GetFieldsFptr = nullptr;
		GetTypeMethodsFn GetTypeMethodsFptr = nullptr;
		*/
		
#pragma region TypeInterface

		GetAssemblyTypesFn GetAssemblyTypes = nullptr;
		GetTypeIdFn GetTypeIdFptr = nullptr;
		GetFullTypeNameFn GetFullTypeNameFptr = nullptr;
		GetAssemblyQualifiedNameFn GetAssemblyQualifiedNameFptr = nullptr;
		GetBaseTypeFn GetBaseTypeFptr = nullptr;
		IsTypeAssignableToFn IsTypeAssignableToFptr = nullptr;
		IsTypeAssignableFromFn IsTypeAssignableFromFptr = nullptr;
		GetTypeMethodsFn GetTypeMethodsFptr = nullptr;

#pragma endregion

#pragma region MethodInfo
		GetMethodInfoNameFn GetMethodInfoNameFptr = nullptr;
		GetMethodInfoReturnTypeFn GetMethodInfoReturnTypeFptr = nullptr;
		GetMethodInfoParameterTypesFn GetMethodInfoParameterTypesFptr = nullptr;
#pragma endregion
		
		CreateObjectFn CreateObjectFptr = nullptr;
		CreateAssemblyLoadContextFn CreateAssemblyLoadContextFptr = nullptr;
		InvokeMethodFn InvokeMethodFptr = nullptr;
		InvokeMethodRetFn InvokeMethodRetFptr = nullptr;
		SetFieldValueFn SetFieldValueFptr = nullptr;
		GetFieldValueFn GetFieldValueFptr = nullptr;
		SetPropertyValueFn SetPropertyValueFptr = nullptr;
		GetPropertyValueFn GetPropertyValueFptr = nullptr;
		DestroyObjectFn DestroyObjectFptr = nullptr;

		SetExceptionCallbackFn SetExceptionCallbackFptr = nullptr;

		CollectGarbageFn CollectGarbageFptr = nullptr;
		WaitForPendingFinalizersFn WaitForPendingFinalizersFptr = nullptr;
	};

	inline ManagedFunctions s_ManagedFunctions;

}
