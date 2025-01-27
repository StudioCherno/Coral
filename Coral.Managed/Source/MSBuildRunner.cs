//using Microsoft.Build.Evaluation;
//using Microsoft.Build.Execution;
//using Microsoft.Build.Framework;
//using Microsoft.Build.Utilities;

using System.Runtime.InteropServices;
using System.Collections.Generic;

using Coral.Managed.Interop;

namespace Coral.Managed;

public static class MSBuildRunner
{
	[UnmanagedCallersOnly]
	internal static unsafe void Run(NativeString InSolutionPath, Bool32 InBuildDebug, Bool32* OutBuildResult)
	{
		/*
		var logger = new NativeLogger();

		var projectCollection = new ProjectCollection();

		var buildParamters = new BuildParameters(projectCollection);
		buildParamters.Loggers = new List<Microsoft.Build.Framework.ILogger>() { logger };

		var globalProperty = new Dictionary<string, string?>();
		globalProperty.Add("Configuration", InBuildDebug ? "Debug" : "Release");

		BuildManager.DefaultBuildManager.ResetCaches();
		var buildRequest = new BuildRequestData(InSolutionPath.ToString() ?? ".", globalProperty, null, new string[] { "Build" }, null);

		var buildResult = BuildManager.DefaultBuildManager.Build(buildParamters, buildRequest);

		*OutBuildResult = buildResult.OverallResult == BuildResultCode.Failure;
		 */
	}
}
