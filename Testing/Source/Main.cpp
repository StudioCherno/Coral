#include <iostream>
#include <string>
#include <filesystem>
#include <chrono>

#include <Coral/HostInstance.hpp>
#include <Coral/GC.h>

uint32_t x = 0;

void Dummy()
{
	for (uint32_t i = 0; i < 10; i++)
		x += i;
}

void RunTest(Coral::HostInstance& InHostInstance, const std::filesystem::path& InFilePath)
{
	Coral::AssemblyHandle testingHandle;
	auto status = InHostInstance.LoadAssembly(InFilePath.string().c_str(), testingHandle);

	InHostInstance.AddInternalCall("Testing.Managed.InternalCalls+Dummy, Testing.Managed", &Dummy);
	InHostInstance.UploadInternalCalls();

	Coral::ObjectHandle objectHandle = InHostInstance.CreateInstance("Testing.Managed.MyTestObject, Testing.Managed", 5);
	auto start = std::chrono::steady_clock::now();
	for (uint32_t i = 0; i < 1000 * 1000; i++)
		InHostInstance.InvokeMethod(objectHandle, "InvokeThis");
	auto elapsed = std::chrono::steady_clock::now() - start;
	std::cout << "InvokeMethod Elapsed: " << std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count() / (1000.0f * 1000.0f) << std::endl;
	InHostInstance.DestroyInstance(objectHandle);

	InHostInstance.UnloadAssemblyLoadContext(testingHandle);
	Coral::GC::Collect();
}

void ExceptionCallback(const CharType* InMessage)
{
#if CORAL_WIDE_CHARS
	std::wcout << L"Unhandled native exception: " << InMessage << std::endl;
#else
	std::cout << "Unhandled native exception: " << InMessage << std::endl;
#endif
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
	hostInstance.SetExceptionCallback(ExceptionCallback);

	auto assemblyPath = std::filesystem::path("F:/Coral/Build") / ConfigName / "Testing.Managed.dll";

	RunTest(hostInstance, assemblyPath);
	RunTest(hostInstance, assemblyPath);

	return 0;
}
