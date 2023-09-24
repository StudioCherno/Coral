#pragma once

#include "StableVector.hpp"

namespace Coral {

	class Type;

	class TypeCache
	{
	public:
		static TypeCache& Get();

		Type* CacheType(Type&& InType);
		Type* GetTypeByName(std::string_view InName) const;

	private:
		StableVector<Type> m_Types;
		std::unordered_map<std::string, Type*> m_NameCache;
	};

}
