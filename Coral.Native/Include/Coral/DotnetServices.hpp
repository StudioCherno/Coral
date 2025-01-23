#pragma once

#include <string>

namespace Coral
{
	class DotnetServices
	{
	public:
		static bool RunMSBuild(const std::string& InSolutionPath, bool InBuildDebug = true);
	};
}
