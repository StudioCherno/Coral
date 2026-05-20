#pragma once

#include "Coral/Core.hpp"
#include "Coral/String.hpp"

namespace Coral {

	struct UnmanagedArray;
	enum class AssemblyLoadStatus;
	class ManagedObject;
	enum class GCCollectionMode;
	enum class ManagedType;
	class ManagedField;

	using SetInternalCallsFn = void (*)(int32_t, void*, int32_t);
	using CreateAssemblyLoadContextFn = int32_t (*)(String, String);
	using UnloadAssemblyLoadContextFn = void (*)(int32_t);
	using LoadAssemblyFn = int32_t(*)(int32_t, String);
	using LoadAssemblyFromMemoryFn = int32_t(*)(int32_t, const std::byte*, int64_t);
	using GetLastLoadStatusFn = AssemblyLoadStatus (*)();
	using GetAssemblyNameFn = String (*)(int32_t, int32_t);

#pragma region DotnetServices
	using RunMSBuildFn = void(*)(String, Bool32, Bool32*);
#pragma endregion DotnetServices

#pragma region TypeInterface

	using GetAssemblyTypesFn = void (*)(int32_t, int32_t, TypeId*, int32_t*);
	using GetTypeIdFn = void (*)(String, TypeId*);
	using GetFullTypeNameFn = String (*)(TypeId);
	using GetAssemblyQualifiedNameFn = String (*)(TypeId);
	using GetBaseTypeFn = void (*)(TypeId, TypeId*);
	using GetInterfaceTypeCountFn = void (*)(TypeId, int32_t*);
	using GetInterfaceTypesFn = void (*)(TypeId, TypeId*);
	using GetTypeSizeFn = int32_t (*)(TypeId);
	using IsTypeSubclassOfFn = Bool32 (*)(TypeId, TypeId);
	using IsTypeAssignableToFn = Bool32 (*)(TypeId, TypeId);
	using IsTypeAssignableFromFn = Bool32 (*)(TypeId, TypeId);
	using IsTypeSZArrayFn = Bool32 (*)(TypeId);
	using GetElementTypeFn = void (*)(TypeId, TypeId*);
	using GetTypeMethodsFn = void (*)(TypeId, int32_t, ManagedHandle*, int32_t*);
	using GetTypeFieldsFn = void (*)(TypeId, int32_t, ManagedHandle*, int32_t*);
	using GetTypePropertiesFn = void (*)(TypeId, int32_t, ManagedHandle*, int32_t*);
	using GetTypeConstructorsFn = void (*)(TypeId, ManagedHandle*, int32_t*);
	using GetTypeEventsFn = void (*)(TypeId, ManagedHandle*, int32_t*);
	using GetTypeNestedTypesFn = void (*)(TypeId, TypeId*, int32_t*);
	using HasTypeAttributeFn = Bool32 (*)(TypeId, TypeId);
	using GetTypeAttributesFn = void (*)(ManagedHandle, TypeId*, int32_t*);
	using GetTypeManagedTypeFn = ManagedType (*)(TypeId);

#pragma endregion

#pragma region MethodInfo
	using GetMethodInfoNameFn = String (*)(ManagedHandle);
	using GetMethodInfoFriendlyNameFn = String (*)(ManagedHandle);
	using GetMethodInfoReturnTypeFn = void (*)(ManagedHandle, TypeId*);
	using GetMethodInfoParameterTypesFn = void (*)(ManagedHandle, TypeId*, int32_t*);
	using GetMethodInfoAccessibilityFn = TypeAccessibility (*)(ManagedHandle);
	using GetMethodInfoAttributesFn = void (*)(ManagedHandle, TypeId*, int32_t*);
#pragma endregion

#pragma region FieldInfo
	using GetFieldInfoNameFn = String (*)(ManagedHandle);
	using GetFieldInfoTypeFn = void (*)(ManagedHandle, TypeId*);
	using GetFieldInfoAccessibilityFn = TypeAccessibility (*)(ManagedHandle);
	using GetFieldInfoIsStaticFn = Bool32 (*)(ManagedHandle);
	using GetFieldInfoIsLiteralFn = Bool32 (*)(ManagedHandle);
	using GetFieldInfoIsInitOnlyFn = Bool32 (*)(ManagedHandle);
	using GetFieldInfoAttributesFn = void (*)(ManagedHandle, TypeId*, int32_t*);
#pragma endregion

#pragma region PropertyInfo
	using GetPropertyInfoNameFn = String (*)(ManagedHandle);
	using GetPropertyInfoTypeFn = void (*)(ManagedHandle, TypeId*);
	using GetPropertyInfoHasGetterFn = Bool32 (*)(ManagedHandle);
	using GetPropertyInfoHasSetterFn = Bool32 (*)(ManagedHandle);
	using GetPropertyInfoGetterAccessibilityFn = TypeAccessibility (*)(ManagedHandle);
	using GetPropertyInfoSetterAccessibilityFn = TypeAccessibility (*)(ManagedHandle);
	using GetPropertyInfoIsStaticFn = Bool32 (*)(ManagedHandle);
	using GetPropertyInfoAttributesFn = void (*)(ManagedHandle, TypeId*, int32_t*);
#pragma endregion

#pragma region ConstructorInfo
	using GetConstructorInfoFriendlyNameFn = String (*)(ManagedHandle);
	using GetConstructorInfoAccessibilityFn = TypeAccessibility (*)(ManagedHandle);
	using GetConstructorInfoAttributesFn = void (*)(ManagedHandle, TypeId*, int32_t*);
#pragma endregion

#pragma region EventInfo
	using GetEventInfoNameFn = String (*)(ManagedHandle);
	using GetEventInfoFriendlyNameFn = String (*)(ManagedHandle);
	using GetEventInfoHandlerTypeFn = void (*)(ManagedHandle, TypeId*);
	using GetEventInfoAccessibilityFn = TypeAccessibility (*)(ManagedHandle);
	using GetEventInfoIsStaticFn = Bool32 (*)(ManagedHandle);
	using GetEventInfoAttributesFn = void (*)(ManagedHandle, TypeId*, int32_t*);
#pragma endregion

#pragma region Attribute
	using GetAttributeFieldValueFn = void (*)(ManagedHandle, String, void*);
	using GetAttributeTypeFn = void (*)(ManagedHandle, TypeId*);
#pragma endregion

	using CreateObjectFn = void* (*)(TypeId, Bool32, const void**, const ManagedType*, int32_t);
	using CopyObjectFn = void* (*)(void*);
	using InvokeMethodFn = void (*)(void*, String, const void**, const ManagedType*, int32_t);
	using InvokeMethodRetFn = void (*)(void*, String, const void**, const ManagedType*, int32_t, void*);
	using InvokeStaticMethodFn = void (*)(TypeId, String, const void**, const ManagedType*, int32_t);
	using InvokeStaticMethodRetFn = void (*)(TypeId, String, const void**, const ManagedType*, int32_t, void*);
	using SetFieldValueFn = void (*)(void*, String, void*);
	using GetFieldValueFn = void (*)(void*, String, void*);
	using SetPropertyValueFn = void (*)(void*, String, void*);
	using GetPropertyValueFn = void (*)(void*, String, void*);
	using DestroyObjectFn = void (*)(void*);
	using GetObjectTypeIdFn = void (*)(void*, int32_t*);

	using CollectGarbageFn = void (*)(int32_t, GCCollectionMode, Bool32, Bool32);
	using WaitForPendingFinalizersFn = void (*)();

	struct ManagedFunctions
	{
		SetInternalCallsFn SetInternalCallsFptr = nullptr;
		LoadAssemblyFn LoadAssemblyFptr = nullptr;
		LoadAssemblyFromMemoryFn LoadAssemblyFromMemoryFptr = nullptr;
		UnloadAssemblyLoadContextFn UnloadAssemblyLoadContextFptr = nullptr;
		GetLastLoadStatusFn GetLastLoadStatusFptr = nullptr;
		GetAssemblyNameFn GetAssemblyNameFptr = nullptr;

#pragma region DotnetServices
		RunMSBuildFn RunMSBuildFptr = nullptr;
#pragma endregion DotnetServices

#pragma region TypeInterface

		GetAssemblyTypesFn GetAssemblyTypesFptr = nullptr;
		GetFullTypeNameFn GetFullTypeNameFptr = nullptr;
		GetAssemblyQualifiedNameFn GetAssemblyQualifiedNameFptr = nullptr;
		GetBaseTypeFn GetBaseTypeFptr = nullptr;
		GetInterfaceTypeCountFn GetInterfaceTypeCountFptr = nullptr;
		GetInterfaceTypesFn GetInterfaceTypesFptr = nullptr;
		GetTypeSizeFn GetTypeSizeFptr = nullptr;
		IsTypeSubclassOfFn IsTypeSubclassOfFptr = nullptr;
		IsTypeAssignableToFn IsTypeAssignableToFptr = nullptr;
		IsTypeAssignableFromFn IsTypeAssignableFromFptr = nullptr;
		IsTypeSZArrayFn IsTypeSZArrayFptr = nullptr;
		GetElementTypeFn GetElementTypeFptr = nullptr;
		GetTypeMethodsFn GetTypeMethodsFptr = nullptr;
		GetTypeFieldsFn GetTypeFieldsFptr = nullptr;
		GetTypePropertiesFn GetTypePropertiesFptr = nullptr;
		GetTypeConstructorsFn GetTypeConstructorsFptr = nullptr;
		GetTypeEventsFn GetTypeEventsFptr = nullptr;
		GetTypeNestedTypesFn GetTypeNestedTypesFptr = nullptr;
		HasTypeAttributeFn HasTypeAttributeFptr = nullptr;
		GetTypeAttributesFn GetTypeAttributesFptr = nullptr;
		GetTypeManagedTypeFn GetTypeManagedTypeFptr = nullptr;

#pragma endregion

#pragma region MethodInfo
		GetMethodInfoNameFn GetMethodInfoNameFptr = nullptr;
		GetMethodInfoFriendlyNameFn GetMethodInfoFriendlyNameFptr = nullptr;
		GetMethodInfoReturnTypeFn GetMethodInfoReturnTypeFptr = nullptr;
		GetMethodInfoParameterTypesFn GetMethodInfoParameterTypesFptr = nullptr;
		GetMethodInfoAccessibilityFn GetMethodInfoAccessibilityFptr = nullptr;
		GetMethodInfoAttributesFn GetMethodInfoAttributesFptr = nullptr;
#pragma endregion

#pragma region FieldInfo
		GetFieldInfoNameFn GetFieldInfoNameFptr = nullptr;
		GetFieldInfoTypeFn GetFieldInfoTypeFptr = nullptr;
		GetFieldInfoAccessibilityFn GetFieldInfoAccessibilityFptr = nullptr;
		GetFieldInfoIsStaticFn GetFieldInfoIsStaticFptr = nullptr;
		GetFieldInfoIsLiteralFn GetFieldInfoIsLiteralFptr = nullptr;
		GetFieldInfoIsInitOnlyFn GetFieldInfoIsInitOnlyFptr = nullptr;
		GetFieldInfoAttributesFn GetFieldInfoAttributesFptr = nullptr;
#pragma endregion

#pragma region PropertyInfo
		GetPropertyInfoNameFn GetPropertyInfoNameFptr = nullptr;
		GetPropertyInfoTypeFn GetPropertyInfoTypeFptr = nullptr;
		GetPropertyInfoHasGetterFn GetPropertyInfoHasGetterFptr = nullptr;
		GetPropertyInfoHasSetterFn GetPropertyInfoHasSetterFptr = nullptr;
		GetPropertyInfoGetterAccessibilityFn GetPropertyInfoGetterAccessibilityFptr = nullptr;
		GetPropertyInfoSetterAccessibilityFn GetPropertyInfoSetterAccessibilityFptr = nullptr;
		GetPropertyInfoIsStaticFn GetPropertyInfoIsStaticFptr = nullptr;
		GetPropertyInfoAttributesFn GetPropertyInfoAttributesFptr = nullptr;
#pragma endregion

#pragma region ConstructorInfo
		GetConstructorInfoFriendlyNameFn GetConstructorInfoFriendlyNameFptr = nullptr;
		GetConstructorInfoAccessibilityFn GetConstructorInfoAccessibilityFptr = nullptr;
		GetConstructorInfoAttributesFn GetConstructorInfoAttributesFptr = nullptr;
#pragma endregion

#pragma region EventInfo
		GetEventInfoNameFn GetEventInfoNameFptr = nullptr;
		GetEventInfoFriendlyNameFn GetEventInfoFriendlyNameFptr = nullptr;
		GetEventInfoHandlerTypeFn GetEventInfoHandlerTypeFptr = nullptr;
		GetEventInfoAccessibilityFn GetEventInfoAccessibilityFptr = nullptr;
		GetEventInfoIsStaticFn GetEventInfoIsStaticFptr = nullptr;
		GetEventInfoAttributesFn GetEventInfoAttributesFptr = nullptr;
#pragma endregion

#pragma region Attribute
		GetAttributeFieldValueFn GetAttributeFieldValueFptr = nullptr;
		GetAttributeTypeFn GetAttributeTypeFptr = nullptr;
#pragma endregion

		CreateObjectFn CreateObjectFptr = nullptr;
		CopyObjectFn CopyObjectFptr = nullptr;
		CreateAssemblyLoadContextFn CreateAssemblyLoadContextFptr = nullptr;
		InvokeMethodFn InvokeMethodFptr = nullptr;
		InvokeMethodRetFn InvokeMethodRetFptr = nullptr;
		InvokeStaticMethodFn InvokeStaticMethodFptr = nullptr;
		InvokeStaticMethodRetFn InvokeStaticMethodRetFptr = nullptr;
		SetFieldValueFn SetFieldValueFptr = nullptr;
		GetFieldValueFn GetFieldValueFptr = nullptr;
		SetPropertyValueFn SetPropertyValueFptr = nullptr;
		GetPropertyValueFn GetPropertyValueFptr = nullptr;
		DestroyObjectFn DestroyObjectFptr = nullptr;
		GetObjectTypeIdFn GetObjectTypeIdFptr = nullptr;

		CollectGarbageFn CollectGarbageFptr = nullptr;
		WaitForPendingFinalizersFn WaitForPendingFinalizersFptr = nullptr;
	};

	inline ManagedFunctions s_ManagedFunctions;

}
