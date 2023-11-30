using ICSharpCode.Decompiler;
using ICSharpCode.Decompiler.CSharp;
using ICSharpCode.Decompiler.CSharp.ProjectDecompiler;
using ICSharpCode.Decompiler.Metadata;

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;

namespace Coral.Generator
{
	public class Program
	{
		public void Run(string csOutputPath, string cppOutputPath, CSharpDecompiler decompiler)
		{
			var sw = Stopwatch.StartNew();
			var nativeTypeList = new NativeTypeTranslationList(decompiler);
			sw.Stop();
			Console.WriteLine($"Generating managed type to native type translation list took {sw.ElapsedMilliseconds}ms");

			sw = Stopwatch.StartNew();
			var methodList = new NativeCallableMethodList(decompiler);
			sw.Stop();
			Console.WriteLine($"Generating method list took {sw.ElapsedMilliseconds}ms");

			List<IFileGenerator> generators = [
				new NativeCallablesCSharpGenerator(csOutputPath, methodList),
				new NativeCallablesCPPGenerator(cppOutputPath, methodList, nativeTypeList)
			];

			foreach (var generator in generators)
			{
				var generatorName = generator.GetType().Name;
				Console.WriteLine($"Running generator: {generatorName}");

				sw = Stopwatch.StartNew();
				generator.GenerateFile(decompiler);
				sw.Stop();
				
				foreach (var message in generator.GetMessages())
					Console.WriteLine($"[{generatorName}]: {message}");

				Console.WriteLine($"{generatorName} took {sw.ElapsedMilliseconds}ms");
			}
		}

		public static void Main(string[] args)
		{
			if (args.Length == 0)
			{
				Console.WriteLine("No arguments specified, exiting...");
				return;
			}

			string csOutputDir = Path.Combine(Environment.CurrentDirectory, "GeneratedFiles");
			string cppOutputDir = Path.Combine(Environment.CurrentDirectory, "GeneratedFiles");

			if (args.Length > 1)
			{
				for (int i = 1; i < args.Length; i += 2)
				{
					if (i + 1 >= args.Length)
					{
						Console.WriteLine("Invalid number of arguments passed");
						return;
					}

					if (args[i] == "--cs-source-dir")
					{
						csOutputDir = args[i + 1];
					}
					else if (args[i] == "--cpp-source-dir")
					{
						cppOutputDir = args[i + 1];
					}
				}
			}

			var file = new PEFile(args[0]);
			var resolver = new UniversalAssemblyResolver(args[0], false, file.Metadata.DetectTargetFrameworkId());

			var decompilerSettings = new DecompilerSettings()
			{
				ThrowOnAssemblyResolveErrors = false,
				RemoveDeadCode = false,
				RemoveDeadStores = false,
				UseSdkStyleProjectFormat = WholeProjectDecompiler.CanUseSdkStyleProjectFormat(file),
				UseNestedDirectoriesForNamespaces = false,
			};

			var decompiler = new CSharpDecompiler(args[0], resolver, decompilerSettings)
			{
				DebugInfoProvider = DebugUtils.LoadSymbols(file)
			};

			new Program().Run(csOutputDir, cppOutputDir, decompiler);
		}
	}
}
