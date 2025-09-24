#pragma once

#include "Core.hpp"
#include "MessageLevel.hpp"
#include "Assembly.hpp"
#include "ManagedObject.hpp"

#include <functional>

namespace Coral {

	using ExceptionCallbackFn = std::function<void(std::string_view)>;

	struct HostSettings
	{
		/// <summary>
		/// The file path to Coral.runtimeconfig.json (e.g C:\Dev\MyProject\ThirdParty\Coral)
		/// </summary>
		std::string CoralDirectory;

		MessageCallbackFn MessageCallback = nullptr;
		MessageLevel MessageFilter = MessageLevel::All;

		ExceptionCallbackFn ExceptionCallback = nullptr;
	};

	enum class CoralInitStatus
	{
		Success,
		CoralManagedNotFound,
		CoralManagedInitError,
		DotNetNotFound,
	};

	class HostInstance
	{
	public:
		CoralInitStatus Initialize(HostSettings InSettings);
		void Shutdown();

		AssemblyLoadContext CreateAssemblyLoadContext(std::string_view InName);
		void UnloadAssemblyLoadContext(AssemblyLoadContext& InLoadContext);

		// `InDllPath` is a colon-separated list of paths from which AssemblyLoader will try and resolve load paths at runtime.
		// This does not affect the behaviour of LoadAssembly from native code.
		AssemblyLoadContext CreateAssemblyLoadContext(std::string_view InName, std::string_view InDllPath);

	private:
		bool LoadHostFXR() const;
		bool InitializeCoralManaged();
		void LoadCoralFunctions();

		void* LoadCoralManagedFunctionPtr(const std::filesystem::path& InAssemblyPath, const UCChar* InTypeName, const UCChar* InMethodName, const UCChar* InDelegateType = CORAL_UNMANAGED_CALLERS_ONLY) const;

		template<typename TFunc>
		TFunc LoadCoralManagedFunctionPtr(const UCChar* InTypeName, const UCChar* InMethodName, const UCChar* InDelegateType = CORAL_UNMANAGED_CALLERS_ONLY) const
		{
			return (TFunc) LoadCoralManagedFunctionPtr(m_CoralManagedAssemblyPath, InTypeName, InMethodName, InDelegateType);
		}

	private:
		HostSettings m_Settings;
		std::filesystem::path m_CoralManagedAssemblyPath;
		void* m_HostFXRContext = nullptr;
		bool m_Initialized = false;

		friend class AssemblyLoadContext;
	};

}
