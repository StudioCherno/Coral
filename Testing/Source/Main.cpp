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
	Coral::HostSettings settings =
	{
		.CoralDirectory = coralDir.c_str()
	};
	Coral::HostInstance hostInstance;
	hostInstance.Initialize(settings);

	Coral::AssemblyHandle testingHandle;
	hostInstance.LoadAssembly(CORAL_STR("F:/Coral/Build/Debug/Testing.Managed.dll"), testingHandle);

	// TODO(Peter): hostInstance.AddInternalCall(testingHandle, "Coral.ManagedHost:Dummy", &Dummy)
	hostInstance.AddInternalCall(CORAL_STR("Coral.ManagedHost+Dummy, Coral.Managed"), &Dummy);
	hostInstance.AddInternalCall(CORAL_STR("Coral.ManagedHost+Dummy, Coral.Managed"), &Dummy);
	hostInstance.UploadInternalCalls();

	Coral::ObjectHandle objectHandle = hostInstance.CreateInstance(CORAL_STR("Testing.MyTestObject, Testing.Managed"), 50, 100.0f, (const CharType*)CORAL_STR("Hello, World?"));
	//hostInstance.CallMethod(objectHandle, "MyInstanceMethod", 5.0f, 10.0f, myOtherObjectHandle);
	hostInstance.DestroyInstance(objectHandle);

	return 0;
}
