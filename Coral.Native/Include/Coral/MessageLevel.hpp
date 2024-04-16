#pragma once

#include "String.hpp"

#include <functional>

namespace Coral {

	enum class MessageLevel
	{
		Info = 1 << 0,
		Warning = 1 << 1,
		Error = 1 << 2,
		All = Info | Warning | Error
	};

	template<typename T>
	constexpr auto ToUnderlying(T InValue)
	{
		return static_cast<std::underlying_type_t<T>>(InValue);
	}

	constexpr MessageLevel operator|(const MessageLevel InLHS, const MessageLevel InRHS) noexcept
	{
		return static_cast<MessageLevel>(ToUnderlying(InLHS) | ToUnderlying(InRHS));
	}
	constexpr bool operator&(const MessageLevel InLHS, const MessageLevel InRHS) noexcept
	{
		return (ToUnderlying(InLHS) & ToUnderlying(InRHS)) != 0;
	}
	constexpr MessageLevel operator~(const MessageLevel InValue) noexcept
	{
		return static_cast<MessageLevel>(~ToUnderlying(InValue));
	}
	constexpr MessageLevel& operator|=(MessageLevel& InLHS, const MessageLevel& InRHS) noexcept
	{
		return (InLHS = (InLHS | InRHS));
	}

	using MessageCallbackFn = std::function<void(std::string_view, MessageLevel)>;

}
