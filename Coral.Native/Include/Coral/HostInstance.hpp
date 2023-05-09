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
		void LoadFunctions() const;
		void InitializeCoralManaged() const;

	private:
		HostSettings m_Settings;
		bool m_Initialized = false;

		std::vector<InternalCallInfo> m_InternalCalls;
	};

}
