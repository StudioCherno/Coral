using ICSharpCode.Decompiler.CSharp;
using ICSharpCode.Decompiler.TypeSystem;

using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace Coral.Generator
{
	internal class NativeCallableMethodList
	{
		private static FullTypeName NativeCallableAttribTypeName = new(new TopLevelTypeName("Coral.Managed", "NativeCallableAttribute"));

		private readonly ConcurrentBag<IMethod> _methods;
		private readonly ConcurrentQueue<string> _messages = new ConcurrentQueue<string>();

		internal NativeCallableMethodList(CSharpDecompiler decompiler)
		{
			_methods = new ConcurrentBag<IMethod>();

			var module = decompiler.TypeSystem.MainModule;
			var nativeCallableAttribType = decompiler.TypeSystem.FindType(NativeCallableAttribTypeName);

			// Find all static methods that have the NativeCallable attribute
			Parallel.ForEach(module.TypeDefinitions, typeDef =>
			{
				foreach (var method in typeDef.Methods)
				{
					foreach (var attrib in method.GetAttributes())
					{
						if (attrib.AttributeType != nativeCallableAttribType)
							continue;

						if (!method.IsStatic)
						{
							_messages.Enqueue($"Ignoring method {method.FullName} with attribute [NativeCallable] because method isn't static");
							continue;
						}

						_methods.Add(method);
					}
				}
			});

			_messages.Enqueue($"Found {_methods.Count} methods with attribute [NativeCallable]");
		}
		
		public IReadOnlyCollection<IMethod> Methods => _methods;
		public IReadOnlyCollection<string> Messages => _messages;
	}
}
