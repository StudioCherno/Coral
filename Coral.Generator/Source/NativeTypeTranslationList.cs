using ICSharpCode.Decompiler.CSharp;
using ICSharpCode.Decompiler.TypeSystem;

using System;
using System.Collections.Concurrent;
using System.Threading.Tasks;

namespace Coral.Generator
{

	internal class NativeTypeTranslationList
	{
		private static FullTypeName NativeTypeAttribTypeName = new(new TopLevelTypeName("Coral.Managed", "NativeTypeAttribute"));

		internal ConcurrentDictionary<string, string> _managedToNativeTypeLookup;

		internal NativeTypeTranslationList(CSharpDecompiler decompiler)
		{
			_managedToNativeTypeLookup = new ConcurrentDictionary<string, string>();

			var module = decompiler.TypeSystem.MainModule;
			var nativeTypeAttribute = decompiler.TypeSystem.FindType(NativeTypeAttribTypeName);

			// Find all static methods that have the NativeCallable attribute
			Parallel.ForEach(module.TypeDefinitions, typeDef =>
			{
				foreach (var attrib in typeDef.GetAttributes())
				{
					if (attrib.AttributeType != nativeTypeAttribute)
						continue;

					var nativeType = attrib.FixedArguments[0].Value as string;
					_managedToNativeTypeLookup.TryAdd(typeDef.FullName, nativeType);
				}
			});

			//_messages.Enqueue($"Found {_methods.Count} methods with attribute [NativeCallable]");
		}

	}
}
