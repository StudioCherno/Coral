using System;

namespace Coral.Managed;

public class TypeNotFoundException : Exception
{
	public TypeNotFoundException()
	{
	}

	public TypeNotFoundException(string message)
		: base(message)
	{
	}

	public TypeNotFoundException(string message, Exception inner)
		: base(message, inner)
	{
	}
}
