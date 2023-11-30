using ICSharpCode.Decompiler.CSharp;
using ICSharpCode.Decompiler.TypeSystem;

using System.CodeDom.Compiler;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Threading.Tasks;

namespace Coral.Generator
{
	internal class NativeCallablesCPPGenerator : IFileGenerator
	{

		private static FullTypeName NativeConstAttribTypeName = new(new TopLevelTypeName("Coral.Managed", "NativeConstAttribute"));

		private readonly ConcurrentQueue<string> _messages = new ConcurrentQueue<string>();
		private readonly string _outputDir;
		private readonly NativeCallableMethodList _methodList;
		private readonly NativeTypeTranslationList _nativeTypeTranslationList;

		internal NativeCallablesCPPGenerator(string outputDir, NativeCallableMethodList methodList, NativeTypeTranslationList nativeTypeTranslationList)
		{
			_outputDir = outputDir;
			_methodList = methodList;
			_nativeTypeTranslationList = nativeTypeTranslationList;
		}

		public void GenerateFile(CSharpDecompiler decompiler)
		{
			var module = decompiler.TypeSystem.MainModule;
			var nativeConstAttribType = decompiler.TypeSystem.FindType(NativeConstAttribTypeName);

			string filepath = Path.Combine(_outputDir, "NativeCallables.generated.hpp");

			using var streamWriter = new StreamWriter(filepath);
			var writer = new IndentedTextWriter(streamWriter);
			writer.WriteLine("#pragma once");
			writer.WriteLine();
			writer.WriteLine($"namespace Coral {{");
			writer.WriteLine();
			writer.Indent += 1;

			writer.WriteLine($"namespace Internal {{");
			writer.Indent += 1;
			writer.WriteLine("using Bool32 = uint32_t;");
			writer.Indent -= 1;
			writer.WriteLine("}");

			writer.WriteLine();

			writer.WriteLine("enum class AssemblyLoadStatus;");
			writer.WriteLine("enum class GCCollectionMode;");
			writer.WriteLine("enum class ManagedType;");
			writer.WriteLine("enum class TypeAccessibility;");
			writer.WriteLine("class String;");

			writer.WriteLine();

			foreach (var method in _methodList.Methods)
			{
				WriteFunctionPointerDefinition(writer, method, nativeConstAttribType);
			}

			writer.WriteLine();

			writer.WriteLine("struct NativeCallables");
			writer.WriteLine("{");
			writer.Indent += 1;

			foreach (var method in _methodList.Methods)
			{
				WriteFunctionPointerDeclaration(writer, method);
			}

			writer.Indent -= 1;
			writer.WriteLine("};");

			writer.WriteLine("inline NativeCallables s_NativeCallables;");

			writer.Indent -= 1;
			writer.WriteLine("}");
		}

		private void WriteFunctionPointerDefinition(TextWriter writer, IMethod method, IType? nativeConstAttribute)
		{
			var parameterList = new StringBuilder();
			var parameterWriter = new ParameterWriter(_nativeTypeTranslationList, parameterList);

			var returnTypeStr = new StringBuilder();
			var returnTypeWriter = new ParameterWriter(_nativeTypeTranslationList, returnTypeStr);

			for (int i = 0; i < method.Parameters.Count; i++)
			{
				var parameter = method.Parameters[i];

				foreach (var attrib in parameter.GetAttributes())
				{
					if (attrib.AttributeType == nativeConstAttribute)
					{
						parameterList.Append("const ");
						break;
					}
				}

				parameter.Type.AcceptVisitor(parameterWriter);

				if (i < method.Parameters.Count - 1)
					parameterList.Append(", ");
			}

			method.ReturnType.AcceptVisitor(returnTypeWriter);

			writer.WriteLine($"using {method.Name}Fn = {returnTypeStr}(*)({parameterList});");
		}

		private void WriteFunctionPointerDeclaration(TextWriter writer, IMethod method)
		{
			writer.WriteLine($"{method.Name}Fn {method.Name}Fptr = nullptr;");
		}

		public IReadOnlyCollection<string> GetMessages() => _messages;

		private sealed class ParameterWriter : TypeVisitor
		{
			private static readonly Dictionary<string, string> CSharpToCppTypeLookup = new()
			{
				{ "sbyte", "int8_t" },
				{ "byte",  "uint8_t" },
				{ "short", "int16_t" },
				{ "ushort", "uint16_t" },
				{ "int", "int32_t" },
				{ "uint", "uint32_t" },
				{ "long", "int64_t" },
				{ "ulong", "uint64_t" },
				{ "nint", "void*" },
				{ "System.GCCollectionMode", "Coral::GCCollectionMode" }
			};

			private readonly NativeTypeTranslationList _nativeTypeTranslationList;

			private readonly StringBuilder _builder;

			public ParameterWriter(NativeTypeTranslationList nativeTypeTranslationList, StringBuilder builder)
			{
				_builder = builder;
				_nativeTypeTranslationList = nativeTypeTranslationList;
			}

			private string GetTypeName(IType type)
			{
				var typeDef = (type is ITypeDefinition) ? type as ITypeDefinition : type.GetDefinition();

				string typeName = string.Empty;

				if (typeDef != null)
					typeName = KnownTypeReference.GetCSharpNameByTypeCode(typeDef.KnownTypeCode) ?? type.FullName;
				else
					typeName = type.FullName;

				if (CSharpToCppTypeLookup.ContainsKey(typeName))
				{
					typeName = CSharpToCppTypeLookup[typeName];
				}

				if (_nativeTypeTranslationList._managedToNativeTypeLookup.ContainsKey(typeName))
				{
					typeName = _nativeTypeTranslationList._managedToNativeTypeLookup[typeName];
				}

				return typeName;
			}

			public override IType VisitOtherType(IType type)
			{
				var name = GetTypeName(type);
				_builder.Append($"{name}");
				return base.VisitOtherType(type);
			}

			public override IType VisitPointerType(PointerType type)
			{
				var elementType = type.ElementType;
				int pointerDepth = 1;

				while (elementType is PointerType pointerElementType)
				{
					elementType = pointerElementType.ElementType;
					pointerDepth++;
				}

				var name = GetTypeName(elementType);
				_builder.Append($"{name}");

				for (int i = 0; i < pointerDepth; i++)
					_builder.Append('*');

				return type;
			}

			public override IType VisitTypeDefinition(ITypeDefinition type)
			{
				var name = GetTypeName(type);
				_builder.Append($"{name}");
				return base.VisitTypeDefinition(type);
			}
		}

	}
}
