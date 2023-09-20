#pragma once

#include "Core.hpp"
#include "Assembly.hpp"
#include "ManagedObject.hpp"
#include "ManagedField.hpp"
#include "MethodInfo.hpp"

#include <functional>

namespace Coral {

	using ErrorCallbackFn = std::function<void(std::string_view)>;
	using ExceptionCallbackFn = std::function<void(std::string_view)>;

	struct HostSettings
	{
		/// <summary>
		/// The file path to Coral.runtimeconfig.json (e.g C:\Dev\MyProject\ThirdParty\Coral)
		/// </summary>
		std::string_view CoralDirectory;
		
		ErrorCallbackFn ErrorCallback;
		ExceptionCallbackFn ExceptionCallback;
	};

	class HostInstance
	{
	public:
		bool Initialize(HostSettings InSettings);
		void Shutdown();

		AssemblyLoadContext CreateAssemblyLoadContext(std::string_view InName);
		void UnloadAssemblyLoadContext(AssemblyLoadContext& InLoadContext);
		
		/*const std::vector<ManagedField>& GetFields(const NativeString& InTypeName);
		const std::vector<MethodInfo>& GetMethods(const NativeString& InTypeName);*/
		
	private:
		void LoadHostFXR() const;
		bool InitializeCoralManaged();
		void LoadCoralFunctions();

		void* LoadCoralManagedFunctionPtr(const std::filesystem::path& InAssemblyPath, const CharType* InTypeName, const CharType* InMethodName, const CharType* InDelegateType = CORAL_UNMANAGED_CALLERS_ONLY) const;

		template<typename TFunc>
		TFunc LoadCoralManagedFunctionPtr(const CharType* InTypeName, const CharType* InMethodName, const CharType* InDelegateType = CORAL_UNMANAGED_CALLERS_ONLY) const
		{
			return (TFunc)LoadCoralManagedFunctionPtr(m_CoralManagedAssemblyPath, InTypeName, InMethodName, InDelegateType);
		}

	private:
		HostSettings m_Settings;
		std::filesystem::path m_CoralManagedAssemblyPath;
		void* m_HostFXRContext = nullptr;
		bool m_Initialized = false;

		std::unordered_map<size_t, std::vector<ManagedField>> m_Fields;
		std::unordered_map<size_t, std::vector<MethodInfo>> m_Methods;

		friend class AssemblyLoadContext;
	};

}
