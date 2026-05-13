#include "Coral/TypeCache.hpp"
#include "Coral/Type.hpp"

namespace Coral {

	TypeCache& TypeCache::Get()
	{
		static TypeCache s_Instance;
		return s_Instance;
	}

	Type* TypeCache::CacheType(Type&& InType)
	{
		Type* type = &m_Types.Insert(std::move(InType)).second;

		// Type.FullName is null for open generic params and other reflection sentinels
		String fullName = type->GetFullName();
		if (fullName.Data() != nullptr)
			m_NameCache[fullName] = type;

		m_IDCache[type->GetTypeId()] = type;
		return type;
	}

	Type* TypeCache::GetTypeByName(std::string_view InName) const
	{
		auto name = std::string(InName);
		auto it = m_NameCache.find(name);
		return it != m_NameCache.end() ? it->second : nullptr;
	}

	Type* TypeCache::GetTypeByID(TypeId InTypeID) const
	{
		auto it = m_IDCache.find(InTypeID);
		return it != m_IDCache.end() ? it->second : nullptr;
	}

	void TypeCache::Clear()
	{
		m_Types.Clear();
		m_NameCache.clear();
		m_IDCache.clear();
	}

}
