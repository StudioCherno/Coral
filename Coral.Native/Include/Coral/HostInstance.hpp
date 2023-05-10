#pragma once

#include "Core.hpp"

namespace Coral {

	using ErrorCallbackFn = void(*)(const CharType* InMessage);

	struct HostSettings
	{
		/// <summary>
		/// The file path to Coral.runtimeconfig.json (e.g C:\Dev\MyProject\ThirdParty\Coral)
		/// </summary>
		const CharType* CoralDirectory;
		
		ErrorCallbackFn ErrorCallback = nullptr;
	};

	struct InternalCallInfo
	{
		const CharType* MethodName;
		void* NativeFuncPtr;
	};

	class HostInstance
	{
	public:
		void Initialize(HostSettings InSettings);

		void AddInternalCall(const CharType* InMethodName, void* InFunctionPtr);

	private:
		void LoadHostFXR() const;
		void InitializeCoralManaged();

		void* LoadCoralManagedFunctionPtr(const std::filesystem::path& InAssemblyPath, const CharType* InTypeName, const CharType* InMethodName) const;

		template<typename TFunc>
		TFunc LoadCoralManagedFunctionPtr(const CharType* InTypeName, const CharType* InMethodName) const
		{
			return (TFunc)LoadCoralManagedFunctionPtr(m_CoralManagedAssemblyPath, InTypeName, InMethodName);
		}

	private:
		HostSettings m_Settings;

		std::filesystem::path m_CoralManagedAssemblyPath;

		void* m_HostFXRContext = nullptr;

		bool m_Initialized = false;

		std::vector<InternalCallInfo> m_InternalCalls;
	};

}
