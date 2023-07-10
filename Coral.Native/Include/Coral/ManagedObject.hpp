#pragma once

#include "ManagedType.hpp"

namespace Coral {
	
	class ManagedObject
	{
	public:
		template<typename TReturn, typename... TArgs>
		TReturn InvokeMethod(std::string_view InMethodName, TArgs&&... InParameters)
		{
			constexpr size_t parameterCount = sizeof...(InParameters);

			TReturn result;
			ManagedType resultType = GetManagedType<TReturn>();
			
			if constexpr (parameterCount > 0)
			{
				const void* parameterValues[parameterCount];
				ManagedType parameterTypes[parameterCount];
				AddToArray<TArgs...>(parameterValues, parameterTypes, std::forward<TArgs>(InParameters)..., std::make_index_sequence<parameterCount>{});
				InvokeMethodRetInternal(InMethodName, parameterTypes, parameterValues, parameterCount, &result, sizeof(TReturn), resultType);
			}
			else
			{
				InvokeMethodRetInternal(InMethodName, nullptr, nullptr, 0, &result, sizeof(TReturn), resultType);
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
				AddToArray<TArgs...>(parameterValues, parameterTypes, std::forward<TArgs>(InParameters)..., std::make_index_sequence<parameterCount>{});
				InvokeMethodInternal(InMethodName, parameterTypes, parameterValues, parameterCount);
			}
			else
			{
				InvokeMethodInternal(InMethodName, nullptr, nullptr, 0);
			}
		}

	private:
		void InvokeMethodInternal(std::string_view InMethodName, ManagedType* InParameterTypes, const void** InParameters, size_t InLength) const;
		void InvokeMethodRetInternal(std::string_view InMethodName, ManagedType* InParameterTypes, const void** InParameters, size_t InLength, void* InResultStorage, uint64_t InResultSize, ManagedType InResultType) const;

	private:
		template<typename TArg, size_t TIndex>
		void AddToArrayI(const void** InArgumentsArr, ManagedType* InArgumentTypesArr, TArg&& InArg) const
		{
			if constexpr (std::is_pointer_v<std::remove_reference_t<TArg>>)
			{
				InArgumentsArr[TIndex] = reinterpret_cast<const void*>(InArg);
				InArgumentTypesArr[TIndex] = ManagedType::Pointer;
			}
			else
			{
				InArgumentsArr[TIndex] = reinterpret_cast<const void*>(&InArg);
				InArgumentTypesArr[TIndex] = GetManagedType<TArg>();
			}
		}

		template<typename... TArgs, size_t... TIndices>
		void AddToArray(const void** InArgumentsArr, ManagedType* InArgumentTypesArr, TArgs&&... InArgs, const std::index_sequence<TIndices...>&)
		{
			(AddToArrayI<TArgs, TIndices>(InArgumentsArr, InArgumentTypesArr, std::forward<TArgs>(InArgs)), ...);
		}
		
	private:
		void* m_Handle = nullptr;

	private:
		friend class HostInstance;
	};
	
}

