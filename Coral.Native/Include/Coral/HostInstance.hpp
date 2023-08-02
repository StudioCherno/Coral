#pragma once

#include "Core.hpp"
#include "Assembly.hpp"
#include "ManagedType.hpp"
#include "ManagedObject.hpp"
#include "ReflectionType.hpp"
#include "ManagedField.hpp"
#include "MethodInfo.hpp"

namespace Coral {

	using ErrorCallbackFn = void(*)(const CharType* InMessage);

	struct HostSettings
	{
		/// <summary>
		/// The file path to Coral.runtimeconfig.json (e.g C:\Dev\MyProject\ThirdParty\Coral)
		/// </summary>
		std::string_view CoralDirectory;
		
		ErrorCallbackFn ErrorCallback = nullptr;
	};

	class HostInstance
	{
	public:
		void Initialize(HostSettings InSettings);

		ManagedAssembly LoadAssembly(std::string_view InFilePath);
		void UnloadAssemblyLoadContext(ManagedAssembly& InAssembly);

		template<typename... TArgs>
		ManagedObject CreateInstance(std::string_view InTypeName, TArgs&&... InArguments)
		{
			constexpr size_t argumentCount = sizeof...(InArguments);

			ManagedObject result;

			if constexpr (argumentCount > 0)
			{
				const void* argumentsArr[argumentCount];
				ManagedType argumentTypes[argumentCount];
				AddToArray<TArgs...>(argumentsArr, argumentTypes, std::forward<TArgs>(InArguments)..., std::make_index_sequence<argumentCount> {});
				result = CreateInstanceInternal(InTypeName, argumentsArr, argumentCount);
			}
			else
			{
				result = CreateInstanceInternal(InTypeName, nullptr, 0);
			}

			return result;
		}
		void DestroyInstance(ManagedObject& InObjectHandle);

		void FreeString(const CharType* InString);

		using ExceptionCallbackFn = void(*)(const CharType*);
		void SetExceptionCallback(ExceptionCallbackFn InCallback);

		ReflectionType& GetReflectionType(const CharType* InTypeName);
		ReflectionType& GetReflectionType(ManagedObject InObject);

		const std::vector<ManagedField>& GetFields(const CharType* InTypeName);
		const std::vector<MethodInfo>& GetMethods(const CharType* InTypeName);
		
	private:
		void LoadHostFXR() const;
		void InitializeCoralManaged();
		void LoadCoralFunctions();

		void* LoadCoralManagedFunctionPtr(const std::filesystem::path& InAssemblyPath, const CharType* InTypeName, const CharType* InMethodName, const CharType* InDelegateType = CORAL_UNMANAGED_CALLERS_ONLY) const;

		template<typename TFunc>
		TFunc LoadCoralManagedFunctionPtr(const CharType* InTypeName, const CharType* InMethodName, const CharType* InDelegateType = CORAL_UNMANAGED_CALLERS_ONLY) const
		{
			return (TFunc)LoadCoralManagedFunctionPtr(m_CoralManagedAssemblyPath, InTypeName, InMethodName, InDelegateType);
		}

		ManagedObject CreateInstanceInternal(std::string_view InTypeName, const void** InParameters, size_t InLength);

	private:
		HostSettings m_Settings;
		std::filesystem::path m_CoralManagedAssemblyPath;
		void* m_HostFXRContext = nullptr;
		bool m_Initialized = false;

		std::unordered_map<size_t, ReflectionType> m_ReflectionTypes;
		std::unordered_map<size_t, std::vector<ManagedField>> m_Fields;
		std::unordered_map<size_t, std::vector<MethodInfo>> m_Methods;
	};

}
