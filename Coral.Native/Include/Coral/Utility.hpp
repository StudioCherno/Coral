#pragma once

#include "Core.hpp"
#include "String.hpp"

namespace Coral {

	enum class ManagedType
	{
		Unknown,

		SByte,
		Byte,
		Short,
		UShort,
		Int,
		UInt,
		Long,
		ULong,

		Float,
		Double,

		Bool,

		String,

		Pointer,
	};

	template<typename TArg>
	constexpr ManagedType GetManagedType()
	{
		if constexpr (std::is_pointer_v<std::remove_reference_t<TArg>>)
			return ManagedType::Pointer;
		else if constexpr (std::is_same_v<TArg, uint8_t> || std::is_same_v<TArg, std::byte>)
			return ManagedType::Byte;
		else if constexpr (std::is_same_v<TArg, uint16_t>)
			return ManagedType::UShort;
		else if constexpr (std::is_same_v<TArg, uint32_t> || (std::is_same_v<TArg, unsigned long> && sizeof(TArg) == 4))
			return ManagedType::UInt;
		else if constexpr (std::is_same_v<TArg, uint64_t> || (std::is_same_v<TArg, unsigned long> && sizeof(TArg) == 8))
			return ManagedType::ULong;
		else if constexpr (std::is_same_v<TArg, int8_t>)
			return ManagedType::SByte;
		else if constexpr (std::is_same_v<TArg, int16_t>)
			return ManagedType::Short;
		else if constexpr (std::is_same_v<TArg, int32_t> || (std::is_same_v<TArg, long> && sizeof(TArg) == 4))
			return ManagedType::Int;
		else if constexpr (std::is_same_v<TArg, int64_t> || (std::is_same_v<TArg, long> && sizeof(TArg) == 8))
			return ManagedType::Long;
		else if constexpr (std::is_same_v<TArg, float>)
			return ManagedType::Float;
		else if constexpr (std::is_same_v<TArg, double>)
			return ManagedType::Double;
		else if constexpr (std::is_same_v<TArg, bool>)
			return ManagedType::Bool;
		else if constexpr (std::is_same_v<TArg, std::string> || std::is_same_v<TArg, Coral::String>)
			return ManagedType::String;
		else
			return ManagedType::Unknown;
	}

	template <typename TArg, size_t TIndex>
	inline void AddToArrayI(const void** InArgumentsArr, ManagedType* InParameterTypes, TArg&& InArg)
	{
		ManagedType managedType = GetManagedType<std::remove_const_t<std::remove_reference_t<TArg>>>();
		InParameterTypes[TIndex] = managedType;

		if constexpr (std::is_pointer_v<std::remove_reference_t<TArg>>)
		{
			InArgumentsArr[TIndex] = reinterpret_cast<const void*>(InArg);
		}
		else
		{
			InArgumentsArr[TIndex] = reinterpret_cast<const void*>(&InArg);
		}
	}

	/*
	 * TODO(Emily): Work out a way to allow method invoke to use C++-y types (i.e. `std::string` instead of `Coral::String`).
	 * 				See Testing/Main.cpp:StringTest/BoolTest.
	 */
	template <typename... TArgs, size_t... TIndices>
	inline void AddToArray(const void** InArgumentsArr, ManagedType* InParameterTypes, TArgs&&... InArgs, const std::index_sequence<TIndices...>&)
	{
		(AddToArrayI<TArgs, TIndices>(InArgumentsArr, InParameterTypes, std::forward<TArgs>(InArgs)), ...);
	}

}
