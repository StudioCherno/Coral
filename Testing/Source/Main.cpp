#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include <Coral/HostInstance.hpp>

uint32_t x = 0;

void Dummy()
{
	for (uint32_t i = 0; i < 10; i++)
		x += i;
}

int32_t ReturnDummy()
{
	return 50;
}

void RunTest(Coral::HostInstance& InHostInstance, const std::filesystem::path& InFilePath)
{
	Coral::AssemblyHandle testingHandle;
	auto status = InHostInstance.LoadAssembly(InFilePath.string().c_str(), testingHandle);

	InHostInstance.AddInternalCall("Testing.Managed.InternalCalls+Dummy, Testing.Managed", &Dummy);
	InHostInstance.UploadInternalCalls();

	Coral::ObjectHandle objectHandle = InHostInstance.CreateInstance("Testing.Managed.MyTestObject, Testing.Managed", 5);
	InHostInstance.DestroyInstance(objectHandle);

	InHostInstance.UnloadAssemblyLoadContext(testingHandle);
}

int main()
{
#ifdef CORAL_TESTING_DEBUG
	const char* ConfigName = "Debug";
#else
	const char* ConfigName = "Release";
#endif

	auto coralDir = (std::filesystem::current_path().parent_path() / "Build" / ConfigName).string();
	Coral::HostSettings settings =
	{
		.CoralDirectory = coralDir.c_str()
	};
	Coral::HostInstance hostInstance;
	hostInstance.Initialize(settings);

	auto assemblyPath = std::filesystem::path("F:/Coral/Build") / ConfigName / "Testing.Managed.dll";

	RunTest(hostInstance, assemblyPath);
	RunTest(hostInstance, assemblyPath);

	return 0;
}
