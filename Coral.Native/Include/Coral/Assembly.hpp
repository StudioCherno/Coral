#pragma once

#include "Type.hpp"

#include "StableVector.hpp"

namespace Coral {

	enum class AssemblyLoadStatus
	{
		Success,
		FileNotFound,
		FileLoadFailure,
		InvalidFilePath,
		InvalidAssembly,
		UnknownError
	};

	class HostInstance;

	class ManagedAssembly
	{
	public:
		int32_t GetAssemblyID() const { return m_AssemblyId; }
		AssemblyLoadStatus GetLoadStatus() const { return m_LoadStatus; }
		std::string_view GetName() const { return m_Name; }

		void AddInternalCall(std::string_view InClassName, std::string_view InVariableName, void* InFunctionPtr);
		void UploadInternalCalls();

		[[deprecated(CORAL_GLOBAL_ALC_MSG)]]
		Type& GetType(std::string_view InClassName) const;

		Type& GetLocalType(std::string_view InClassName) const;

		[[deprecated(CORAL_GLOBAL_ALC_MSG)]]
		const std::vector<Type*>& GetTypes() const;
		
	private:
		HostInstance* m_Host = nullptr;
		int32_t m_AssemblyId = -1;
		int32_t m_OwnerContextId = 0;
		AssemblyLoadStatus m_LoadStatus = AssemblyLoadStatus::UnknownError;
		std::string m_Name;

		std::vector<UCString> m_InternalCallNameStorage;

		std::vector<InternalCall> m_InternalCalls;

		std::vector<Type*> m_Types;
		std::vector<Type> m_LocalTypes;

		friend class HostInstance;
		friend class AssemblyLoadContext;
	};

	class AssemblyLoadContext
	{
	public:
		ManagedAssembly& LoadAssembly(std::string_view InFilePath);
		ManagedAssembly& LoadAssemblyFromMemory(const std::byte* data, int64_t dataLength);
		const StableVector<ManagedAssembly>& GetLoadedAssemblies() const { return m_LoadedAssemblies; }

	private:
		int32_t m_ContextId;
		StableVector<ManagedAssembly> m_LoadedAssemblies;

		HostInstance* m_Host = nullptr;

		friend class HostInstance;
	};

}
