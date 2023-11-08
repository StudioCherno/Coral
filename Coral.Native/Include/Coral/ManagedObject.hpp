#pragma once

#include "Core.hpp"
#include "Utility.hpp"

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
				AddToArray<TArgs...>(parameterValues, parameterTypes, std::forward<TArgs>(InParameters)..., std::make_index_sequence<parameterCount> {});
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
				AddToArray<TArgs...>(parameterValues, parameterTypes, std::forward<TArgs>(InParameters)..., std::make_index_sequence<parameterCount> {});
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
			SetFieldValueRaw(InFieldName, &InValue);
		}

		template<typename TReturn>
		TReturn GetFieldValue(std::string_view InFieldName)
		{
			TReturn result;
			GetFieldValueRaw(InFieldName, &result);
			return result;
		}

		template<typename TValue>
		void SetPropertyValue(std::string_view InPropertyName, TValue InValue)
		{
			SetPropertyValueRaw(InPropertyName, &InValue);
		}

		template<typename TReturn>
		TReturn GetPropertyValue(std::string_view InPropertyName)
		{
			TReturn result;
			GetPropertyValueRaw(InPropertyName, &result);
			return result;
		}

		void SetFieldValueRaw(std::string_view InFieldName, void* InValue) const;
		void GetFieldValueRaw(std::string_view InFieldName, void* OutValue) const;
		void SetPropertyValueRaw(std::string_view InPropertyName, void* InValue) const;
		void GetPropertyValueRaw(std::string_view InPropertyName, void* OutValue) const;

		const Type& GetType() const;
		
		void Destroy();

	private:
		void InvokeMethodInternal(std::string_view InMethodName, const void** InParameters, const ManagedType* InParameterTypes, size_t InLength) const;
		void InvokeMethodRetInternal(std::string_view InMethodName, const void** InParameters, const ManagedType* InParameterTypes, size_t InLength, void* InResultStorage) const;

	private:
		void* m_Handle = nullptr;
		Type* m_Type;

	private:
		friend class ManagedAssembly;
		friend class Type;
	};
	
}

