using ICSharpCode.Decompiler.CSharp;

using System;
using System.Collections.Generic;

namespace Coral.Generator
{
	internal interface IFileGenerator
	{
		void GenerateFile(CSharpDecompiler decompiler);
		IReadOnlyCollection<string> GetMessages();
	}
}
