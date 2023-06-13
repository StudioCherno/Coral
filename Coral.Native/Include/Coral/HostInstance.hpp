#pragma once

#include "Core.hpp"
#include "Assembly.hpp"

#include <unordered_map>

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

	class ObjectHandle
	{
	public:
		bool IsValid() const { return m_Handle != nullptr; }

	private:
		void* m_Handle;

		friend class HostInstance;
	};

	class HostInstance
	{
	public:
		void Initialize(HostSettings InSettings);
		AssemblyLoadStatus LoadAssembly(const CharType* InFilePath, AssemblyHandle& OutHandle);

		void AddInternalCall(const CharType* InMethodName, void* InFunctionPtr);

		void UploadInternalCalls();

		ObjectHandle CreateInstance(const CharType* InTypeName);
		void DestroyInstance(ObjectHandle& InObjectHandle);

	private:
		void LoadHostFXR() const;
		void InitializeCoralManaged();

		void* LoadCoralManagedFunctionPtr(const std::filesystem::path& InAssemblyPath, const CharType* InTypeName, const CharType* InMethodName, const CharType* InDelegateType = CORAL_UNMANAGED_CALLERS_ONLY) const;

		template<typename TFunc>
		TFunc LoadCoralManagedFunctionPtr(const CharType* InTypeName, const CharType* InMethodName, const CharType* InDelegateType = CORAL_UNMANAGED_CALLERS_ONLY) const
		{
			return (TFunc)LoadCoralManagedFunctionPtr(m_CoralManagedAssemblyPath, InTypeName, InMethodName, InDelegateType);
		}

	public:
		struct InternalCall
		{
			const CharType* Name;
			void* NativeFunctionPtr;
		};

	private:
		HostSettings m_Settings;
		std::filesystem::path m_CoralManagedAssemblyPath;
		void* m_HostFXRContext = nullptr;
		bool m_Initialized = false;

		std::vector<InternalCall*> m_InternalCalls;

		//std::unordered_map<uint16_t, AssemblyData> m_LoadedAssemblies;
	};

}
