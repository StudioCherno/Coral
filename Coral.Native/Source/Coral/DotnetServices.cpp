#include "Coral/DotnetServices.hpp"

#include "CoralManagedFunctions.hpp"

namespace Coral
{
	bool DotnetServices::RunMSBuild(const std::string& InSolutionPath, bool InBuildDebug)
	{
		String s = String::New(InSolutionPath);
		Bool32 result;

		s_ManagedFunctions.RunMSBuildFptr(s, InBuildDebug, &result);

		String::Free(s);

		return !!result;
	}
}
