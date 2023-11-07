#pragma once

#include "Core.hpp"
#include "Utility.hpp"
#include "NativeString.hpp"

namespace Coral {

	class ManagedAssembly;
	class Type;

	class ManagedObject
	{
	public:
		template<typename TReturn, typename... TArgs>
		TReturn InvokeMethod(std::string_view InMethodName, TArgs&&... InParameters)
		{
			constexpr size_t parameterCount = sizeof...(InParameters);

			TReturn result;
			
			if constexpr (parameterCount > 0)
			{
				const void* parameterValues[parameterCount];
				ManagedType parameterTypes[parameterCount];
				Utility::AddToArray<TArgs...>(parameterValues, parameterTypes, std::forward<TArgs>(InParameters)..., std::make_index_sequence<parameterCount> {});
				InvokeMethodRetInternal(InMethodName, parameterValues, parameterTypes, parameterCount, &result);
			}
			else
			{
				InvokeMethodRetInternal(InMethodName, nullptr, nullptr, 0, &result);
			}

			return result;
		}

		template<typename... TArgs>
		void InvokeMethod(std::string_view InMethodName, TArgs&&... InParameters)
		{
			constexpr size_t parameterCount = sizeof...(InParameters);

			if constexpr (parameterCount > 0)
			{
				const void* parameterValues[parameterCount];
				ManagedType parameterTypes[parameterCount];
				Utility::AddToArray<TArgs...>(parameterValues, parameterTypes, std::forward<TArgs>(InParameters)..., std::make_index_sequence<parameterCount> {});
				InvokeMethodInternal(InMethodName, parameterValues, parameterTypes, parameterCount);
			}
			else
			{
				InvokeMethodInternal(InMethodName, nullptr, nullptr, 0);
			}
		}

		template<typename TValue>
		void SetFieldValue(std::string_view InFieldName, TValue InValue)
		{
			if constexpr (std::constructible_from<NativeString, TValue>)
			{
				NativeString string(InValue);
				SetFieldValueRaw(InFieldName, &string);
			}
			else
			{
				SetFieldValueRaw(InFieldName, &InValue);
			}
		}

		template<typename TReturn>
		TReturn GetFieldValue(std::string_view InFieldName)
		{
			TReturn result;
			
			if constexpr (std::same_as<TReturn, NativeString>)
			{
				StringData stringData;
				GetFieldValueRaw(InFieldName, &stringData);
				result = TReturn(stringData);
			}
			else
			{
				GetFieldValueRaw(InFieldName, &result);
			}

			return result;
		}

		template<typename TValue>
		void SetPropertyValue(std::string_view InPropertyName, TValue InValue)
		{
			if constexpr (std::constructible_from<NativeString, TValue>)
			{
				NativeString string(InValue);
				SetPropertyValueRaw(InPropertyName, &string);
			}
			else
			{
				SetPropertyValueRaw(InPropertyName, &InValue);
			}
		}

		template<typename TReturn>
		TReturn GetPropertyValue(std::string_view InPropertyName)
		{
			TReturn result;

			if constexpr (std::same_as<TReturn, NativeString>)
			{
				StringData stringData;
				GetPropertyValueRaw(InPropertyName, &stringData);
				result = TReturn(stringData);
			}
			else
			{
				GetPropertyValueRaw(InPropertyName, &result);
			}

			return result;
		}

		void SetFieldValueRaw(NativeString InFieldName, void* InValue) const;
		void GetFieldValueRaw(NativeString InFieldName, void* OutValue) const;
		void SetPropertyValueRaw(NativeString InPropertyName, void* InValue) const;
		void GetPropertyValueRaw(NativeString InPropertyName, void* OutValue) const;

		const Type& GetType() const;
		
		void Destroy();

	private:
		void InvokeMethodInternal(NativeString InMethodName, const void** InParameters, const ManagedType* InParameterTypes, size_t InLength) const;
		void InvokeMethodRetInternal(NativeString InMethodName, const void** InParameters, const ManagedType* InParameterTypes, size_t InLength, void* InResultStorage) const;

	private:
		void* m_Handle = nullptr;
		Type* m_Type;

	private:
		friend class ManagedAssembly;
		friend class Type;
	};
	
}

