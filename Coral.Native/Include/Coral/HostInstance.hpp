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

		// Loads an assembly into the runtime's default load context, which never unloads (hostfxr's hdt_load_assembly,
		// .NET 8+). Coral.Managed itself lives in an isolated component context, so assemblies loaded here can't share
		// Coral's types; this is for hosts that need their own static entry points. False, with an error message, on
		// failure.
		bool LoadAssemblyIntoDefaultContext(const std::filesystem::path& InAssemblyPath);

		// The address of a static [UnmanagedCallersOnly] method of a type in the default load context (hostfxr's
		// hdt_get_function_pointer). `InTypeName` is assembly-qualified: "Namespace.Type, AssemblyName". Nullptr, with
		// an error message, when it can't be resolved.
		void* GetDefaultContextFunction(std::string_view InTypeName, std::string_view InMethodName);

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
