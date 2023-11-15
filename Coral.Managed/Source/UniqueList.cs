using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.Serialization;

namespace Coral.Managed
{
	public class UniqueList<T>
	{
		// TODO(Peter): Implement custom object ID generator since the System provided one is now obsolete
		private ObjectIDGenerator m_IdGenerator;
		private readonly Dictionary<long, T?> m_Objects;

		public UniqueList()
		{
			m_IdGenerator = new ObjectIDGenerator();
			m_Objects = new Dictionary<long, T?>();
		}

		public bool Contains(long id)
		{
			return m_Objects.ContainsKey(id);
		}

		public long Add(T? obj)
		{
			if (obj == null)
			{
				throw new ArgumentNullException(nameof(obj));
			}

			long id = m_IdGenerator.GetId(obj, out var newId);

			if (!newId)
			{
				// Object should already exist in m_Objects
				Debug.Assert(m_Objects.ContainsKey(id));
				return id;
			}

			m_Objects.Add(id, obj);
			return id;
		}

		public bool TryGetValue(long id, out T? obj)
		{
			return m_Objects.TryGetValue(id, out obj);
		}

		public void Clear()
		{
			m_IdGenerator = new ObjectIDGenerator();
			m_Objects.Clear();
		}

	}
}
