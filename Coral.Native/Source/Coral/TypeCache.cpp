#include "TypeCache.hpp"
#include "Type.hpp"

namespace Coral {

	TypeCache& TypeCache::Get()
	{
		static TypeCache s_Instance;
		return s_Instance;
	}

	Type* TypeCache::CacheType(Type&& InType)
	{
		Type* type = &m_Types.Insert(std::move(InType)).second;
		m_NameCache[type->GetFullName()] = type;
		return type;
	}

	Type* TypeCache::GetTypeByName(std::string_view InName) const
	{
		auto name = std::string(InName);
		return m_NameCache.contains(name) ? m_NameCache.at(name) : nullptr;
	}

}
