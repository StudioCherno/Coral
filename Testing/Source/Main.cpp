#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include <Coral/HostInstance.hpp>

void Dummy()
{
	std::cout << "Dummy!" << std::endl;
}

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

	hostInstance.AddInternalCall(CORAL_STR("Test1"), &Dummy);
	hostInstance.AddInternalCall(CORAL_STR("Test2"), &Dummy);
	hostInstance.UploadInternalCalls();

	return 0;
}
