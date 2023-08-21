#include "Assembly.hpp"
#include "HostInstance.hpp"
#include "CoralManagedFunctions.hpp"
#include "Interop.hpp"
#include "Verify.hpp"
#include "StringHelper.hpp"

namespace Coral {

	void ManagedAssembly::AddInternalCall(std::string_view InClassName, std::string_view InVariableName, void* InFunctionPtr)
	{
		CORAL_VERIFY(InFunctionPtr != nullptr);

		std::string assemblyQualifiedName(InClassName);
		assemblyQualifiedName += "+";
		assemblyQualifiedName += InVariableName;
		assemblyQualifiedName += ", ";
		assemblyQualifiedName += m_Name;
		
		const auto& name = m_InternalCallNameStorage.emplace_back(StringHelper::ConvertUtf8ToWide(assemblyQualifiedName));

		InternalCall internalCall;
		internalCall.Name = name.c_str();
		internalCall.NativeFunctionPtr = InFunctionPtr;
		m_InternalCalls.emplace_back(internalCall);
	}

	void ManagedAssembly::UploadInternalCalls()
	{
		UnmanagedArray arr = { m_InternalCalls.data(), static_cast<int32_t>(m_InternalCalls.size()) };
		s_ManagedFunctions.SetInternalCallsFptr(&arr);
	}

}
