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

	TypeId ManagedAssembly::GetTypeId(std::string_view InClassName) const
	{
		auto name = StringHelper::ConvertUtf8ToWide(InClassName);
		TypeId typeId = nullptr;
		s_ManagedFunctions.GetTypeIdFptr(name.c_str(), &typeId);
		return typeId;
	}

	static bool IsInvalidType(const ReflectionType& InType)
	{
		static ReflectionType s_NullType;
		return memcmp(&InType, &s_NullType, sizeof(ReflectionType)) == 0;
	}

	ManagedAssembly& AssemblyLoadContext::LoadAssembly(std::string_view InFilePath)
	{
		ManagedAssembly& result = m_LoadedAssemblies.emplace_back();
		auto filepath = StringHelper::ConvertUtf8ToWide(InFilePath);
		result.m_Host = m_Host;
		result.m_AssemblyID = s_ManagedFunctions.LoadManagedAssemblyFptr(m_ContextId, filepath.c_str());
		result.m_LoadStatus = s_ManagedFunctions.GetLastLoadStatusFptr();

		if (result.m_LoadStatus == AssemblyLoadStatus::Success)
		{
			const auto* name = s_ManagedFunctions.GetAssemblyNameFptr(result.m_AssemblyID);
			result.m_Name = StringHelper::ConvertWideToUtf8(name);
			s_ManagedFunctions.FreeManagedStringFptr(name);

			int32_t typeCount;
			s_ManagedFunctions.QueryAssemblyTypesFptr(result.m_AssemblyID, nullptr, &typeCount);
			result.m_ReflectionTypes.resize(typeCount);
			s_ManagedFunctions.QueryAssemblyTypesFptr(result.m_AssemblyID, result.m_ReflectionTypes.data(), &typeCount);

			std::erase_if(result.m_ReflectionTypes, [](const ReflectionType& InType)
			{
				return IsInvalidType(InType);
			});

			for (auto& type : result.m_ReflectionTypes)
			{
				type.m_Host = m_Host;

				std::string str = type.FullName.ToString();
				size_t id = std::hash<std::string>()(str);
				m_Host->m_ReflectionTypes[id] = type;
			}
		}

		return result;
	}

}
