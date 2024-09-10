#pragma once

#include "Core.hpp"
#include "StableVector.hpp"

namespace Coral {

	class Type;

	class TypeCache
	{
	public:
		static TypeCache& Get();

		Type* CacheType(Type&& InType);
		Type* GetTypeByName(std::string_view InName) const;
		Type* GetTypeByID(TypeId InTypeID) const;

		void Clear();

	private:
		StableVector<Type> m_Types;
		std::unordered_map<std::string, Type*> m_NameCache;
		std::unordered_map<TypeId, Type*> m_IDCache;
	};

}
