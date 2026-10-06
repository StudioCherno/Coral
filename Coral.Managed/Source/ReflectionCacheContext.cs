using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.Loader;

namespace Coral.Managed;

// Called while the owner has quiesced the context. A framework generic such as
// List<PluginType> belongs to the framework assembly but still retains PluginType.
internal static class ReflectionCacheContext
{
	internal static bool References(Type? type, AssemblyLoadContext context)
	{
		if (type == null) return false;
		Stack<Type> pending = new();
		HashSet<Type> visited = new();
		pending.Push(type);
		while (pending.TryPop(out var current))
		{
			if (!visited.Add(current)) continue;
			if (current.IsFunctionPointer)
			{
				pending.Push(current.GetFunctionPointerReturnType());
				foreach (var parameter in current.GetFunctionPointerParameterTypes()) pending.Push(parameter);
				continue;
			}
			if (AssemblyLoadContext.GetLoadContext(current.Assembly) == context) return true;
			if (current.HasElementType && current.GetElementType() is Type element) pending.Push(element);
			if (current.IsConstructedGenericType)
				foreach (var argument in current.GetGenericArguments()) pending.Push(argument);
		}
		return false;
	}

	internal static bool References(MemberInfo member, AssemblyLoadContext context)
	{
		if (member is Type type) return References(type, context);
		if (AssemblyLoadContext.GetLoadContext(member.Module.Assembly) == context
			|| References(member.DeclaringType, context) || References(member.ReflectedType, context)) return true;
		switch (member)
		{
			case MethodInfo method:
				if (References(method.ReturnType, context)) return true;
				if (method.IsGenericMethod)
					foreach (var argument in method.GetGenericArguments())
						if (References(argument, context)) return true;
				return ParametersReference(method.GetParameters(), context);
			case ConstructorInfo constructor: return ParametersReference(constructor.GetParameters(), context);
			case FieldInfo field: return References(field.FieldType, context);
			case PropertyInfo property:
				return References(property.PropertyType, context) || ParametersReference(property.GetIndexParameters(), context);
			case EventInfo eventInfo: return References(eventInfo.EventHandlerType, context);
			default: return false;
		}
	}

	private static bool ParametersReference(ParameterInfo[] parameters, AssemblyLoadContext context)
	{
		foreach (var parameter in parameters)
			if (References(parameter.ParameterType, context)) return true;
		return false;
	}
}
