#pragma once

#include "Core.hpp"
#include "StableVector.hpp"

namespace Coral {
	class Type;

	class [[deprecated(CORAL_GLOBAL_ALC_MSG)]] TypeCache
	{
	public:
		[[deprecated(CORAL_GLOBAL_ALC_MSG)]]
		static TypeCache& Get();

		[[deprecated(CORAL_GLOBAL_ALC_MSG)]]
		Type* CacheType(Type&& InType);

		[[deprecated(CORAL_GLOBAL_ALC_MSG_P(ManagedAssembly::GetLocalType))]]
		Type* GetTypeByName(std::string_view InName) const;

		[[deprecated(CORAL_GLOBAL_ALC_MSG)]]
		Type* GetTypeByID(TypeId InTypeID) const;

		[[deprecated(CORAL_GLOBAL_ALC_MSG)]]
		void Clear();

	private:
		StableVector<Type> m_Types;
		std::unordered_map<std::string, Type*> m_NameCache;
		std::unordered_map<TypeId, Type*> m_IDCache;
	};

}
