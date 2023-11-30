using ICSharpCode.Decompiler.DebugInfo;
using ICSharpCode.Decompiler.Metadata;
using ICSharpCode.Decompiler.Util;

using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.IO;
using System.Reflection.Metadata;
using System.Reflection.PortableExecutable;
using System.Runtime.InteropServices;

using SequencePoint = ICSharpCode.Decompiler.DebugInfo.SequencePoint;

namespace Coral.Generator
{
	/// <summary>
	/// Code retrieved from https://github.com/icsharpcode/ILSpy/tree/71c3aaf4976a59d63770eda9efca1116d1bca818/ICSharpCode.ILSpyX/PdbProvider
	/// </summary>

	class PortableDebugInfoProvider : IDebugInfoProvider
	{
		string? pdbFileName;
		string moduleFileName;
		readonly MetadataReaderProvider provider;
		bool hasError;

		internal bool IsEmbedded => pdbFileName == null;

		public PortableDebugInfoProvider(string moduleFileName, MetadataReaderProvider provider,
			string? pdbFileName = null)
		{
			this.moduleFileName = moduleFileName ?? throw new ArgumentNullException(nameof(moduleFileName));
			this.provider = provider ?? throw new ArgumentNullException(nameof(provider));
			this.pdbFileName = pdbFileName;
		}

		public string Description
		{
			get
			{
				if (pdbFileName == null)
				{
					if (hasError)
						return "Error while loading the PDB stream embedded in this assembly";
					return "Embedded in this assembly";
				}
				else
				{
					if (hasError)
						return $"Error while loading portable PDB: {pdbFileName}";
					return $"Loaded from portable PDB: {pdbFileName}";
				}
			}
		}

		internal MetadataReader? GetMetadataReader()
		{
			try
			{
				hasError = false;
				return provider.GetMetadataReader();
			}
			catch (BadImageFormatException)
			{
				hasError = true;
				return null;
			}
			catch (IOException)
			{
				hasError = true;
				return null;
			}
		}

		public string SourceFileName => pdbFileName ?? moduleFileName;

		public IList<SequencePoint> GetSequencePoints(MethodDefinitionHandle method)
		{
			var metadata = GetMetadataReader();
			if (metadata == null)
				return EmptyList<SequencePoint>.Instance;
			var debugInfo = metadata.GetMethodDebugInformation(method);
			var sequencePoints = new List<SequencePoint>();

			foreach (var point in debugInfo.GetSequencePoints())
			{
				string documentFileName;

				if (!point.Document.IsNil)
				{
					var document = metadata.GetDocument(point.Document);
					documentFileName = metadata.GetString(document.Name);
				}
				else
				{
					documentFileName = "";
				}

				sequencePoints.Add(new SequencePoint()
				{
					Offset = point.Offset,
					StartLine = point.StartLine,
					StartColumn = point.StartColumn,
					EndLine = point.EndLine,
					EndColumn = point.EndColumn,
					DocumentUrl = documentFileName
				});
			}

			return sequencePoints;
		}

		public IList<Variable> GetVariables(MethodDefinitionHandle method)
		{
			var metadata = GetMetadataReader();
			var variables = new List<Variable>();
			if (metadata == null)
				return variables;

			foreach (var h in metadata.GetLocalScopes(method))
			{
				var scope = metadata.GetLocalScope(h);
				foreach (var v in scope.GetLocalVariables())
				{
					var var = metadata.GetLocalVariable(v);
					variables.Add(new Variable(var.Index, metadata.GetString(var.Name)));
				}
			}

			return variables;
		}

		public bool TryGetName(MethodDefinitionHandle method, int index, [NotNullWhen(true)] out string? name)
		{
			var metadata = GetMetadataReader();
			name = null;
			if (metadata == null)
				return false;

			foreach (var h in metadata.GetLocalScopes(method))
			{
				var scope = metadata.GetLocalScope(h);

				foreach (var v in scope.GetLocalVariables())
				{
					var var = metadata.GetLocalVariable(v);

					if (var.Index == index)
					{
						name = metadata.GetString(var.Name);
						return true;
					}
				}
			}
			return false;
		}
	}

	public static class DebugUtils
	{
		public static IDebugInfoProvider? LoadSymbols(PEFile module)
		{
			try
			{
				// try to open portable pdb file/embedded pdb info:
				if (TryOpenPortablePdb(module, out var provider, out var pdbFileName))
				{
					return new PortableDebugInfoProvider(module.FileName, provider, pdbFileName);
				}
				else
				{
					return null;
				}
			}
			catch (Exception ex) when (ex is BadImageFormatException || ex is COMException)
			{
				// Ignore PDB load errors
			}

			return null;
		}

		public static IDebugInfoProvider? FromFile(PEFile module, string pdbFileName)
		{
			if (string.IsNullOrEmpty(pdbFileName))
				return null;

			Stream? stream = OpenStream(pdbFileName);

			if (stream == null)
				return null;

			stream.Position = 0;
			var provider = MetadataReaderProvider.FromPortablePdbStream(stream);
			return new PortableDebugInfoProvider(module.FileName, provider, pdbFileName);
		}

		const string LegacyPDBPrefix = "Microsoft C/C++ MSF 7.00";
		static readonly byte[] buffer = new byte[LegacyPDBPrefix.Length];

		static bool TryOpenPortablePdb(PEFile module,
			[NotNullWhen(true)] out MetadataReaderProvider? provider,
			[NotNullWhen(true)] out string? pdbFileName)
		{
			provider = null;
			pdbFileName = null;
			var reader = module.Reader;
			foreach (var entry in reader.ReadDebugDirectory())
			{
				if (entry.IsPortableCodeView)
				{
					return reader.TryOpenAssociatedPortablePdb(module.FileName, OpenStream,
						out provider, out pdbFileName);
				}
				if (entry.Type == DebugDirectoryEntryType.CodeView)
				{
					string pdbDirectory = Path.GetDirectoryName(module.FileName)!;
					pdbFileName = Path.Combine(
						pdbDirectory, Path.GetFileNameWithoutExtension(module.FileName) + ".pdb");
					Stream? stream = OpenStream(pdbFileName);
					if (stream != null)
					{
						if (stream.Read(buffer, 0, buffer.Length) == LegacyPDBPrefix.Length
							&& System.Text.Encoding.ASCII.GetString(buffer) == LegacyPDBPrefix)
						{
							return false;
						}
						stream.Position = 0;
						provider = MetadataReaderProvider.FromPortablePdbStream(stream);
						return true;
					}
				}
			}
			return false;
		}

		static Stream? OpenStream(string fileName)
		{
			if (!File.Exists(fileName))
				return null;
			var memory = new MemoryStream();
			using (var stream = File.OpenRead(fileName))
				stream.CopyTo(memory);
			memory.Position = 0;
			return memory;
		}
	}
}