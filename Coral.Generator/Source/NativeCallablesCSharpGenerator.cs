using ICSharpCode.Decompiler.CSharp;
using ICSharpCode.Decompiler.TypeSystem;

using System.CodeDom.Compiler;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace Coral.Generator
{
	internal class NativeCallablesCSharpGenerator : IFileGenerator
	{
		private readonly ConcurrentQueue<string> _messages = new ConcurrentQueue<string>();
		private readonly string _outputDir;
		private readonly NativeCallableMethodList _methodList;

		internal NativeCallablesCSharpGenerator(string outputDir, NativeCallableMethodList methodList)
		{
			_outputDir = outputDir;
			_methodList = methodList;
		}

		public void GenerateFile(CSharpDecompiler decompiler)
		{
			var module = decompiler.TypeSystem.MainModule;

			string filepath = Path.Combine(_outputDir, "NativeCallables.generated.cs");

			using var streamWriter = new StreamWriter(filepath);
			var writer = new IndentedTextWriter(streamWriter);
			writer.WriteLine("using System;");
			writer.WriteLine("using System.Runtime.InteropServices;");
			writer.WriteLine();
			writer.WriteLine($"namespace {module.AssemblyName}");
			writer.WriteLine("{");
			writer.WriteLine();
			writer.Indent += 1;

			writer.WriteLine("[StructLayout(LayoutKind.Sequential)]");
			writer.WriteLine("internal unsafe struct NativeCallables");
			writer.WriteLine("{");
			writer.Indent += 1;

			foreach (var method in _methodList.Methods)
			{
				WriteFunctionPointerDeclaration(writer, method);
			}

			writer.WriteLine();
			writer.WriteLine("[UnmanagedCallersOnly]");
			writer.WriteLine("internal static NativeCallables Get()");
			writer.WriteLine("{");
			writer.Indent += 1;

			writer.WriteLine("return new()");
			writer.WriteLine("{");
			writer.Indent += 1;

			foreach (var method in _methodList.Methods)
			{
				WriteFunctionPointerAssignment(writer, method);
			}

			writer.Indent -= 1;
			writer.WriteLine("};");

			writer.Indent -= 1;
			writer.WriteLine("}");

			writer.Indent -= 1;
			writer.WriteLine("}");

			writer.Indent -= 1;
			writer.WriteLine("}");
		}

		private void WriteFunctionPointerDeclaration(TextWriter writer, IMethod method)
		{
			var parameterList = new StringBuilder();
			var parameterWriter = new ParameterWriter(parameterList);

			foreach (var parameter in method.Parameters)
			{
				parameter.Type.AcceptVisitor(parameterWriter);
				parameterList.Append(", ");
			}

			method.ReturnType.AcceptVisitor(parameterWriter);

			writer.WriteLine($"private delegate*<{parameterList}> {method.Name}Fptr;");
		}

		private void WriteFunctionPointerAssignment(TextWriter writer, IMethod method)
		{
			writer.WriteLine($"{method.Name}Fptr = &{method.FullName},");
		}

		public IReadOnlyCollection<string> GetMessages() => _messages;

		private sealed class ParameterWriter : TypeVisitor
		{

			private readonly StringBuilder _builder;

			public ParameterWriter(StringBuilder builder)
			{
				_builder = builder;
			}

			private string GetTypeName(IType type)
			{
				var typeDef = (type is ITypeDefinition) ? type as ITypeDefinition : type.GetDefinition();

				if (typeDef != null)
					return KnownTypeReference.GetCSharpNameByTypeCode(typeDef.KnownTypeCode) ?? type.FullName;

				return type.FullName;
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
