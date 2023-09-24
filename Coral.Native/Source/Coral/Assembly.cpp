#include "Assembly.hpp"
#include "HostInstance.hpp"
#include "CoralManagedFunctions.hpp"
#include "Verify.hpp"
#include "StringHelper.hpp"
#include "NativeArray.hpp"
#include "TypeCache.hpp"

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
		s_ManagedFunctions.SetInternalCallsFptr(m_InternalCalls.data(), static_cast<int32_t>(m_InternalCalls.size()));
	}

	Type& ManagedAssembly::GetType(std::string_view InClassName) const
	{
		static Type s_NullType;
		Type* type = TypeCache::Get().GetTypeByName(InClassName);
		return type != nullptr ? *type : s_NullType;
	}

	const std::vector<Type*>& ManagedAssembly::GetTypes() const
	{
		return m_Types;
	}

	ManagedAssembly& AssemblyLoadContext::LoadAssembly(std::string_view InFilePath)
	{
		ManagedAssembly& result = m_LoadedAssemblies.emplace_back();
		auto filepath = NativeString::FromUTF8(InFilePath);
		result.m_Host = m_Host;
		result.m_AssemblyID = s_ManagedFunctions.LoadManagedAssemblyFptr(m_ContextId, filepath);
		result.m_LoadStatus = s_ManagedFunctions.GetLastLoadStatusFptr();

		if (result.m_LoadStatus == AssemblyLoadStatus::Success)
		{
			auto name = s_ManagedFunctions.GetAssemblyNameFptr(result.m_AssemblyID);
			result.m_Name = NativeString::ToUTF8(name);

			int32_t typeCount = 0;
			s_ManagedFunctions.GetAssemblyTypes(result.m_AssemblyID, nullptr, &typeCount);
			std::vector<TypeId> typeIds(typeCount);
			s_ManagedFunctions.GetAssemblyTypes(result.m_AssemblyID, typeIds.data(), &typeCount);

			for (auto typeId : typeIds)
			{
				Type type;
				type.m_TypePtr = typeId;
				type.RetrieveName();
				result.m_Types.push_back(TypeCache::Get().CacheType(std::move(type)));
			}
		}

		return result;
	}

}
