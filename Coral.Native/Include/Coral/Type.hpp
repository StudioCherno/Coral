#pragma once

#include "Core.hpp"
#include "NativeString.hpp"
#include "ManagedObject.hpp"
#include "MethodInfo.hpp"

namespace Coral {

	class Type
	{
	public:
		std::string_view GetName() const { return m_Name; }
		std::string_view GetNamespace() const { return m_Namespace; }
		std::string GetFullName() const;
		std::string GetAssemblyQualifiedName() const;

		Type GetBaseType() const;

		bool IsTypeAssignableTo(const Type& InOther);
		bool IsTypeAssignableFrom(const Type& InOther);

		std::vector<MethodInfo> GetMethods() const;

		bool operator==(const Type& InOther) const;

		operator bool() const { return m_TypePtr != nullptr; }

	public:
		template<typename... TArgs>
		ManagedObject CreateInstance(TArgs&&... InArguments)
		{
			constexpr size_t argumentCount = sizeof...(InArguments);

			ManagedObject result;

			if constexpr (argumentCount > 0)
			{
				const void* argumentsArr[argumentCount];
				AddToArray<TArgs...>(argumentsArr, std::forward<TArgs>(InArguments)..., std::make_index_sequence<argumentCount> {});
				result = CreateInstanceInternal(argumentsArr, argumentCount);
			}
			else
			{
				result = CreateInstanceInternal(nullptr, 0);
			}

			return result;
		}

	private:
		void RetrieveName();

		ManagedObject CreateInstanceInternal(const void** InParameters, size_t InLength);

	private:
		TypeId m_TypePtr = nullptr;
		std::string m_Name;
		std::string m_Namespace;

		friend class ManagedAssembly;
		friend class AssemblyLoadContext;
		friend class MethodInfo;
	};

}