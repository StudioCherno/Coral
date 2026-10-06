using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace Coral.Managed;

// Native handles are opaque, process-lifetime IDs. Removing an entry must not
// let a later object reuse its handle, including after a load-context unload.
public class UniqueIdList<T> where T : class
{
	private readonly object m_Gate = new();
	private readonly Dictionary<int, Entry> m_Objects = new();
	private readonly Dictionary<Entry, int> m_Ids = new(new EntryComparer());
	private int m_LastId;

	private readonly record struct Entry(T Value, object? Owner);
	private sealed class EntryComparer : IEqualityComparer<Entry>
	{
		public bool Equals(Entry a, Entry b) => ReferenceEquals(a.Value, b.Value) && ReferenceEquals(a.Owner, b.Owner);
		public int GetHashCode(Entry value) => HashCode.Combine(RuntimeHelpers.GetHashCode(value.Value),
			value.Owner == null ? 0 : RuntimeHelpers.GetHashCode(value.Owner));
	}

	public bool Contains(int id)
	{
		lock (m_Gate) return m_Objects.ContainsKey(id);
	}

	public int Add(T? obj, object? owner = null)
	{
		ArgumentNullException.ThrowIfNull(obj);
		lock (m_Gate)
		{
			var entry = new Entry(obj, owner);
			if (m_Ids.TryGetValue(entry, out int existing)) return existing;
			if (m_LastId == int.MaxValue) throw new InvalidOperationException("Coral reflection handle capacity exhausted.");
			int id = ++m_LastId;
			m_Objects.Add(id, entry);
			try { m_Ids.Add(entry, id); }
			catch { m_Objects.Remove(id); throw; }
			return id;
		}
	}

	public bool TryGetValue(int id, out T? obj)
	{
		lock (m_Gate)
		{
			if (m_Objects.TryGetValue(id, out var entry)) { obj = entry.Value; return true; }
			obj = null;
			return false;
		}
	}

	internal void RemoveWhere(Func<T, object?, bool> predicate)
	{
		lock (m_Gate)
		{
			List<int> removed = new();
			foreach (var entry in m_Objects)
				if (predicate(entry.Value.Value, entry.Value.Owner)) removed.Add(entry.Key);
			foreach (int id in removed)
			{
				m_Ids.Remove(m_Objects[id]);
				m_Objects.Remove(id);
			}
		}
	}

	public void Clear()
	{
		lock (m_Gate)
		{
			m_Objects.Clear();
			m_Ids.Clear();
		}
	}
}
