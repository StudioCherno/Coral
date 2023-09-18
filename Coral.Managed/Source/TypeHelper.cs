using System;
using System.Reflection;

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

public static class TypeHelper
{

	public static Type? FindType(string? InTypeName)
	{
		var type = Type.GetType(InTypeName!,
			(name) => AssemblyLoader.ResolveAssembly(null, name),
			(assembly, name, ignore) => assembly != null ? assembly.GetType(name, false, ignore) : Type.GetType(name, false, ignore)
		);

		return type;
	}

	public static object? CreateInstance(Type InType, params object?[]? InArguments)
	{
		return InType.Assembly.CreateInstance(InType.FullName ?? string.Empty, false, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance, null, InArguments!, null, null);
	}

}
