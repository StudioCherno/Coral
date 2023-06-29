#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include <Coral/HostInstance.hpp>

void Dummy()
{
}

int32_t ReturnDummy()
{
	return 50;
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

	Coral::AssemblyHandle testingHandle;
	hostInstance.LoadAssembly("F:/Coral/Build/Debug/Testing.Managed.dll", testingHandle);

	hostInstance.AddInternalCall("Testing.Test+Dummy, Testing.Managed", &Dummy);
	hostInstance.AddInternalCall("Testing.Test+ReturnIntDel, Testing.Managed", &ReturnDummy);
	hostInstance.UploadInternalCalls();

	Coral::ObjectHandle objectHandle = hostInstance.CreateInstance("Testing.MyTestObject, Testing.Managed", 5);
	//hostInstance.CallMethod(objectHandle, "MyInstanceMethod", 5.0f, 10.0f, myOtherObjectHandle);
	hostInstance.DestroyInstance(objectHandle);

	return 0;
}
