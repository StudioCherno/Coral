#pragma once

#include "Core.hpp"
#include "String.hpp"
#include "ManagedObject.hpp"
#include "MethodInfo.hpp"
#include "FieldInfo.hpp"
#include "PropertyInfo.hpp"

namespace Coral {

	class Type
	{
	public:
		String GetFullName() const;
		String GetAssemblyQualifiedName() const;

		Type& GetBaseType();

		int32_t GetSize() const;

		bool IsSubclassOf(const Type& InOther) const;
		bool IsAssignableTo(const Type& InOther) const;
		bool IsAssignableFrom(const Type& InOther) const;

		std::vector<MethodInfo> GetMethods() const;
		std::vector<FieldInfo> GetFields() const;
		std::vector<PropertyInfo> GetProperties() const;

		bool HasAttribute(const Type& InAttributeType) const;
		std::vector<Attribute> GetAttributes() const;

		ManagedType GetManagedType() const;

		bool IsSZArray() const;
		Type& GetElementType();

		bool operator==(const Type& InOther) const;

		operator bool() const { return m_Id != -1; }

		TypeId GetTypeId() const { return m_Id; }

	public:
		template<typename... TArgs>
		ManagedObject CreateInstance(TArgs&&... InArguments) const
		{
			constexpr size_t argumentCount = sizeof...(InArguments);

			ManagedObject result;

			if constexpr (argumentCount > 0)
			{
				const void* argumentsArr[argumentCount];
				ManagedType argumentTypes[argumentCount];
				AddToArray<TArgs...>(argumentsArr, argumentTypes, std::forward<TArgs>(InArguments)..., std::make_index_sequence<argumentCount> {});
				result = CreateInstanceInternal(argumentsArr, argumentTypes, argumentCount);
			}
			else
			{
				result = CreateInstanceInternal(nullptr, nullptr, 0);
			}

			return result;
		}

		template <typename TReturn, typename... TArgs>
		TReturn InvokeStaticMethod(std::string_view InMethodName, TArgs&&... InParameters) const
		{
			constexpr size_t parameterCount = sizeof...(InParameters);

			TReturn result;

			if constexpr (parameterCount > 0)
			{
				const void* parameterValues[parameterCount];
				ManagedType parameterTypes[parameterCount];
				AddToArray<TArgs...>(parameterValues, parameterTypes, std::forward<TArgs>(InParameters)..., std::make_index_sequence<parameterCount> {});
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
				AddToArray<TArgs...>(parameterValues, parameterTypes, std::forward<TArgs>(InParameters)..., std::make_index_sequence<parameterCount> {});
				InvokeStaticMethodInternal(InMethodName, parameterValues, parameterTypes, parameterCount);
			}
			else
			{
				InvokeStaticMethodInternal(InMethodName, nullptr, nullptr, 0);
			}
		}

	private:
		ManagedObject CreateInstanceInternal(const void** InParameters, const ManagedType* InParameterTypes, size_t InLength) const;
		void InvokeStaticMethodInternal(std::string_view InMethodName, const void** InParameters, const ManagedType* InParameterTypes, size_t InLength) const;
		void InvokeStaticMethodRetInternal(std::string_view InMethodName, const void** InParameters, const ManagedType* InParameterTypes, size_t InLength, void* InResultStorage) const;

	private:
		TypeId m_Id = -1;
		Type* m_BaseType = nullptr;
		Type* m_ElementType = nullptr;

		friend class HostInstance;
		friend class ManagedAssembly;
		friend class AssemblyLoadContext;
		friend class MethodInfo;
		friend class FieldInfo;
		friend class PropertyInfo;
		friend class Attribute;
		friend class ReflectionType;
		friend class ManagedObject;
	};

	class ReflectionType
	{
	public:
		operator Type&() const;

	private:
		TypeId m_TypeID;
	};

}
