#pragma once

namespace Coral {

	template<typename TArg, size_t TIndex>
	inline void AddToArrayI(const void** InArgumentsArr, TArg&& InArg)
	{
		if constexpr (std::is_pointer_v<std::remove_reference_t<TArg>>)
		{
			InArgumentsArr[TIndex] = reinterpret_cast<const void*>(InArg);
		}
		else
		{
			InArgumentsArr[TIndex] = reinterpret_cast<const void*>(&InArg);
		}
	}

	template<typename... TArgs, size_t... TIndices>
	inline void AddToArray(const void** InArgumentsArr, TArgs&&... InArgs, const std::index_sequence<TIndices...>&)
	{
		(AddToArrayI<TArgs, TIndices>(InArgumentsArr, std::forward<TArgs>(InArgs)), ...);
	}

}
