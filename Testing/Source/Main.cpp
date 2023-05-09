#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include <Coral/HostInstance.hpp>

int main()
{
#ifdef CORAL_TESTING_DEBUG
	constexpr std::wstring ConfigName = L"Debug";
#else
	constexpr std::wstring ConfigName = L"Release";
#endif

	auto coralDir = std::filesystem::current_path().parent_path() / "Build" / ConfigName;
	Coral::HostSettings settings = {
		.CoralDirectory = coralDir.c_str()
	};
	Coral::HostInstance hostInstance;
	hostInstance.Initialize(settings);

	return 0;
}
