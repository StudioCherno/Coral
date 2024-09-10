using System;
using System.Collections.Concurrent;
using System.Runtime.CompilerServices;

namespace Coral.Managed;

public class UniqueIdList<T>
{
	private readonly ConcurrentDictionary<int, T> m_Objects = new();

	public bool Contains(int id)
	{
		return m_Objects.ContainsKey(id);
	}

	public int Add(T? obj)
	{
		if (obj == null)
		{
			throw new ArgumentNullException(nameof(obj));
		}

		int hashCode = RuntimeHelpers.GetHashCode(obj);
		_ = m_Objects.TryAdd(hashCode, obj);
		return hashCode;
	}

	public bool TryGetValue(int id, out T? obj)
	{
		return m_Objects.TryGetValue(id, out obj);
	}

	public void Clear()
	{
		m_Objects.Clear();
	}
}
