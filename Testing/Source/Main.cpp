#include <iostream>
#include <string>
#include <filesystem>
#include <chrono>

#include <Coral/HostInstance.hpp>
#include <Coral/GC.h>

void ExceptionCallback(const CharType* InMessage)
{
#if CORAL_WIDE_CHARS
	std::wcout << L"Unhandled native exception: " << InMessage << std::endl;
#else
	std::cout << "Unhandled native exception: " << InMessage << std::endl;
#endif
}

char8_t SByteMarshalIcall(char8_t InValue) { return InValue * 2; }
uint8_t ByteMarshalIcall(uint8_t InValue) { return InValue * 2; }
int16_t ShortMarshalIcall(int16_t InValue) { return InValue * 2; }
uint16_t UShortMarshalIcall(uint16_t InValue) { return InValue * 2; }
int32_t IntMarshalIcall(int32_t InValue) { return InValue * 2; }
uint32_t UIntMarshalIcall(uint32_t InValue) { return InValue * 2; }
int64_t LongMarshalIcall(int64_t InValue) { return InValue * 2; }
uint64_t ULongMarshalIcall(uint64_t InValue) { return InValue * 2; }
float FloatMarshalIcall(float InValue) { return InValue * 2.0f; }
double DoubleMarshalIcall(double InValue) { return InValue * 2.0; }
bool BoolMarshalIcall(bool InValue) { return !InValue; }
int32_t* IntPtrMarshalIcall(int32_t* InValue)
{
	*InValue *= 2;
	return InValue;
}

struct DummyStruct
{
	int32_t X;
	float Y;
	int32_t Z;
};
DummyStruct DummyStructMarshalIcall(DummyStruct InStruct)
{
	InStruct.X *= 2;
	InStruct.Y *= 2.0f;
	InStruct.Z *= 2;
	return InStruct;
}

DummyStruct* DummyStructPtrMarshalIcall(DummyStruct* InStruct)
{
	InStruct->X *= 2;
	InStruct->Y *= 2.0f;
	InStruct->Z *= 2;
	return InStruct;
}

void RegisterTestInternalCalls(Coral::HostInstance& InHost)
{
	InHost.AddInternalCall("Testing.Managed.Tests+SByteMarshalIcall, Testing.Managed", &SByteMarshalIcall);
	InHost.AddInternalCall("Testing.Managed.Tests+ByteMarshalIcall, Testing.Managed", &ByteMarshalIcall);
	InHost.AddInternalCall("Testing.Managed.Tests+ShortMarshalIcall, Testing.Managed", &ShortMarshalIcall);
	InHost.AddInternalCall("Testing.Managed.Tests+UShortMarshalIcall, Testing.Managed", &UShortMarshalIcall);
	InHost.AddInternalCall("Testing.Managed.Tests+IntMarshalIcall, Testing.Managed", &IntMarshalIcall);
	InHost.AddInternalCall("Testing.Managed.Tests+UIntMarshalIcall, Testing.Managed", &UIntMarshalIcall);
	InHost.AddInternalCall("Testing.Managed.Tests+LongMarshalIcall, Testing.Managed", &LongMarshalIcall);
	InHost.AddInternalCall("Testing.Managed.Tests+ULongMarshalIcall, Testing.Managed", &ULongMarshalIcall);
	InHost.AddInternalCall("Testing.Managed.Tests+FloatMarshalIcall, Testing.Managed", &FloatMarshalIcall);
	InHost.AddInternalCall("Testing.Managed.Tests+DoubleMarshalIcall, Testing.Managed", &DoubleMarshalIcall);
	InHost.AddInternalCall("Testing.Managed.Tests+BoolMarshalIcall, Testing.Managed", &BoolMarshalIcall);
	InHost.AddInternalCall("Testing.Managed.Tests+IntPtrMarshalIcall, Testing.Managed", &IntPtrMarshalIcall);
	InHost.AddInternalCall("Testing.Managed.Tests+DummyStructMarshalIcall, Testing.Managed", &DummyStructMarshalIcall);
	InHost.AddInternalCall("Testing.Managed.Tests+DummyStructPtrMarshalIcall, Testing.Managed", &DummyStructPtrMarshalIcall);
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

	Coral::AssemblyHandle testingHandle;
	auto status = hostInstance.LoadAssembly(assemblyPath.string().c_str(), testingHandle);

	RegisterTestInternalCalls(hostInstance);

	hostInstance.UploadInternalCalls();

	Coral::ObjectHandle objectHandle = hostInstance.CreateInstance("Testing.Managed.Tests, Testing.Managed");

	{
		hostInstance.InvokeMethod(objectHandle, "RunManagedTests");
	}

	hostInstance.DestroyInstance(objectHandle);

	hostInstance.UnloadAssemblyLoadContext(testingHandle);
	Coral::GC::Collect();

	return 0;
}
