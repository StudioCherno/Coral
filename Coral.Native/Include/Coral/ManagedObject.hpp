#pragma once

#include "Core.hpp"
#include "ManagedType.hpp"
#include "ReflectionType.hpp"
#include "Utility.hpp"

namespace Coral {

	class HostInstance;

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
				AddToArray<TArgs...>(parameterValues, std::forward<TArgs>(InParameters)..., std::make_index_sequence<parameterCount>{});
				InvokeMethodRetInternal(InMethodName, parameterValues, parameterCount, &result);
			}
			else
			{
				InvokeMethodRetInternal(InMethodName, nullptr, 0, &result);
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
				AddToArray<TArgs...>(parameterValues, std::forward<TArgs>(InParameters)..., std::make_index_sequence<parameterCount>{});
				InvokeMethodInternal(InMethodName, parameterValues, parameterCount);
			}
			else
			{
				InvokeMethodInternal(InMethodName, nullptr, 0);
			}
		}

		template<typename TValue>
		void SetFieldValue(std::string_view InFieldName, TValue InValue)
		{
			SetFieldValueInternal(InFieldName, &InValue);
		}

		template<typename TReturn>
		TReturn GetFieldValue(std::string_view InFieldName)
		{
			TReturn result;
			GetFieldValueInternal(InFieldName, &result);
			return result;
		}

		template<typename TValue>
		void SetPropertyValue(std::string_view InPropertyName, TValue InValue)
		{
			SetPropertyValueInternal(InPropertyName, &InValue);
		}

		template<typename TReturn>
		TReturn GetPropertyValue(std::string_view InPropertyName)
		{
			TReturn result;
			GetPropertyValueInternal(InPropertyName, &result);
			return result;
		}

		ReflectionType& GetType();
		
	private:
		void InvokeMethodInternal(std::string_view InMethodName, const void** InParameters, size_t InLength) const;
		void InvokeMethodRetInternal(std::string_view InMethodName, const void** InParameters, size_t InLength, void* InResultStorage) const;
		void SetFieldValueInternal(std::string_view InFieldName, void* InValue) const;
		void GetFieldValueInternal(std::string_view InFieldName, void* OutValue) const;
		void SetPropertyValueInternal(std::string_view InPropertyName, void* InValue) const;
		void GetPropertyValueInternal(std::string_view InPropertyName, void* OutValue) const;

	private:
		void* m_Handle = nullptr;
		CSString m_FullName;
		HostInstance* m_Host = nullptr;

	private:
		friend class HostInstance;
	};
	
}

