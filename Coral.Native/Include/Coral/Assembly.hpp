#pragma once

#include "Core.hpp"
#include "ReflectionType.hpp"

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

	class ManagedAssembly
	{
	public:
		int32_t GetAssemblyID() const { return m_AssemblyID; }
		AssemblyLoadStatus GetLoadStatus() const { return m_LoadStatus; }

		void AddInternalCall(std::string_view InClassName, std::string_view InVariableName, void* InFunctionPtr);
		void UploadInternalCalls();

		const std::vector<ReflectionType>& GetTypes() const { return m_ReflectionTypes; }
		
	private:
		HostInstance* m_Host = nullptr;
		int32_t m_AssemblyID = -1;
		AssemblyLoadStatus m_LoadStatus = AssemblyLoadStatus::UnknownError;
		std::string m_Name;
		
		struct InternalCall
		{
			const CharType* Name;
			void* NativeFunctionPtr;
		};

	#if defined(CORAL_WIDE_CHARS)
		std::vector<std::wstring> m_InternalCallNameStorage;
	#else
		std::vector<std::string> m_InternalCallNameStorage;
	#endif
		
		std::vector<InternalCall*> m_InternalCalls;

		std::vector<ReflectionType> m_ReflectionTypes;
		
		friend class HostInstance;
	};

}
