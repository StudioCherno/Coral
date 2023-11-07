#pragma once

#include "Core.hpp"
#include "NativeString.hpp"
#include "ManagedObject.hpp"
#include "MethodInfo.hpp"
#include "FieldInfo.hpp"
#include "PropertyInfo.hpp"

namespace Coral {

	class Type
	{
	public:
		std::string_view GetName() const { return m_Name; }
		std::string_view GetNamespace() const { return m_Namespace; }
		std::string GetFullName() const;
		NativeString GetAssemblyQualifiedName() const;

		Type& GetBaseType();

		bool IsSubclassOf(const Type& InOther);
		bool IsAssignableTo(const Type& InOther);
		bool IsAssignableFrom(const Type& InOther);

		std::vector<MethodInfo> GetMethods() const;
		std::vector<FieldInfo> GetFields() const;
		std::vector<PropertyInfo> GetProperties() const;

		std::vector<Attribute> GetAttributes() const;

		ManagedType GetManagedType() const;

		bool operator==(const Type& InOther) const;

		operator bool() const { return m_TypePtr != -1; }

		TypeId GetTypeId() const { return m_TypePtr; }

	public:
		template<typename... TArgs>
		ManagedObject CreateInstance(TArgs&&... InArguments)
		{
			constexpr size_t argumentCount = sizeof...(InArguments);

			ManagedObject result;

			if constexpr (argumentCount > 0)
			{
				const void* argumentsArr[argumentCount];
				ManagedType argumentTypes[argumentCount];
				Utility::AddToArray<TArgs...>(argumentsArr, argumentTypes, std::forward<TArgs>(InArguments)..., std::make_index_sequence<argumentCount> {});
				result = CreateInstanceInternal(argumentsArr, argumentTypes, argumentCount);
			}
			else
			{
				result = CreateInstanceInternal(nullptr, nullptr, 0);
			}

			return result;
		}

		template <typename TReturn, typename... TArgs>
		TReturn InvokeStaticMethod(std::string_view InMethodName, TArgs&&... InParameters)
		{
			constexpr size_t parameterCount = sizeof...(InParameters);

			TReturn result;

			if constexpr (parameterCount > 0)
			{
				const void* parameterValues[parameterCount];
				ManagedType parameterTypes[parameterCount];
				Utility::AddToArray<TArgs...>(parameterValues, parameterTypes, std::forward<TArgs>(InParameters)..., std::make_index_sequence<parameterCount> {});
				InvokeStaticMethodRetInternal(InMethodName, parameterValues, parameterTypes, parameterCount, &result);
			}
			else
			{
				InvokeStaticMethodRetInternal(InMethodName, nullptr, nullptr, 0, &result);
			}

			return result;
		}

		template <typename... TArgs>
		void InvokeStaticMethod(std::string_view InMethodName, TArgs&&... InParameters)
		{
			constexpr size_t parameterCount = sizeof...(InParameters);

			if constexpr (parameterCount > 0)
			{
				const void* parameterValues[parameterCount];
				ManagedType parameterTypes[parameterCount];
				Utility::AddToArray<TArgs...>(parameterValues, parameterTypes, std::forward<TArgs>(InParameters)..., std::make_index_sequence<parameterCount> {});
				InvokeStaticMethodInternal(InMethodName, parameterValues, parameterTypes, parameterCount);
			}
			else
			{
				InvokeStaticMethodInternal(InMethodName, nullptr, nullptr, 0);
			}
		}

	private:
		void RetrieveName();

		ManagedObject CreateInstanceInternal(const void** InParameters, const ManagedType* InParameterTypes, size_t InLength);
		void InvokeStaticMethodInternal(NativeString InMethodName, const void** InParameters, const ManagedType* InParameterTypes, size_t InLength) const;
		void InvokeStaticMethodRetInternal(NativeString InMethodName, const void** InParameters, const ManagedType* InParameterTypes, size_t InLength, void* InResultStorage) const;

	private:
		TypeId m_TypePtr = -1;
		Type* m_BaseType = nullptr;

		std::string m_Name;
		std::string m_Namespace;

		friend class HostInstance;
		friend class ManagedAssembly;
		friend class AssemblyLoadContext;
		friend class MethodInfo;
		friend class FieldInfo;
		friend class PropertyInfo;
		friend class Attribute;
	};

}
