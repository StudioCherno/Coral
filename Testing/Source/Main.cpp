#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <chrono>
#include <functional>
#include <source_location>

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
const CharType* StringMarshalIcall(const CharType* InStr)
{
	return InStr;
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
	InHost.AddInternalCall("Testing.Managed.Tests+StringMarshalIcall, Testing.Managed", &StringMarshalIcall);
	InHost.AddInternalCall("Testing.Managed.Tests+DummyStructMarshalIcall, Testing.Managed", &DummyStructMarshalIcall);
	InHost.AddInternalCall("Testing.Managed.Tests+DummyStructPtrMarshalIcall, Testing.Managed", &DummyStructPtrMarshalIcall);
}

struct Test
{
	std::string Name;
	std::function<bool()> Func;
};
std::vector<Test> tests;

void RegisterTest(std::string_view InName, std::function<bool()> InFunc)
{
	tests.emplace_back(std::string(InName), std::move(InFunc));
}

void RegisterMemeberMethodTests(Coral::HostInstance& InHost, Coral::ObjectHandle InObject)
{
	RegisterTest("SByteTest", [&](){ return InHost.InvokeMethodRet<char8_t, char8_t>(InObject, "SByteTest", 10) == 20; });
	RegisterTest("ByteTest", [&](){ return InHost.InvokeMethodRet<uint8_t, uint8_t>(InObject, "ByteTest", 10) == 20; });
	RegisterTest("ShortTest", [&](){ return InHost.InvokeMethodRet<int16_t, int16_t>(InObject, "ShortTest", 10) == 20; });
	RegisterTest("UShortTest", [&](){ return InHost.InvokeMethodRet<uint16_t, uint16_t>(InObject, "UShortTest", 10) == 20; });
	RegisterTest("IntTest", [&](){ return InHost.InvokeMethodRet<int32_t, int32_t>(InObject, "IntTest", 10) == 20; });
	RegisterTest("UIntTest", [&](){ return InHost.InvokeMethodRet<uint32_t, uint32_t>(InObject, "UIntTest", 10) == 20; });
	RegisterTest("LongTest", [&](){ return InHost.InvokeMethodRet<int64_t, int64_t>(InObject, "LongTest", 10) == 20; });
	RegisterTest("ULongTest", [&](){ return InHost.InvokeMethodRet<uint64_t, uint64_t>(InObject, "ULongTest", 10) == 20; });
	RegisterTest("FloatTest", [&](){ return InHost.InvokeMethodRet<float, float>(InObject, "FloatTest", 10.0f) - 20.0f < 0.001f; });
	RegisterTest("DoubleTest", [&](){ return InHost.InvokeMethodRet<double, double>(InObject, "DoubleTest", 10.0) - 20.0 < 0.001; });
	RegisterTest("BoolTest", [&](){ return InHost.InvokeMethodRet<bool, bool>(InObject, "BoolTest", false); });
	RegisterTest("IntPtrTest", [&](){ int32_t v = 10; return *InHost.InvokeMethodRet<int32_t*, int32_t*>(InObject, "IntPtrTest", &v) == 50; });
#if CORAL_WIDE_CHARS
	//RegisterTest("StringTest", [&](){ return wcscmp(InHost.InvokeMethodRet<const CharType*, const CharType*>(InObject, "StringTest", CORAL_STR("Hello")), CORAL_STR("Hello, World!")) == 0; });
#else
	//RegisterTest("SByteTest", [&](){ return strcmp(InHost.InvokeMethodRet<const CharType*, const CharType*>(InObject, "SByteTest", CORAL_STR("Hello")), CORAL_STR("Hello, World!")) == 0; });
#endif
	
	// TODO(Peter): Struct Marshalling
	//RegisterTest("SByteTest", [&](){ return InHost.InvokeMethodRet<char8_t, char8_t>(InObject, "SByteTest", 10) == 20; });
	//RegisterTest("SByteTest", [&](){ return InHost.InvokeMethodRet<char8_t, char8_t>(InObject, "SByteTest", 10) == 20; });
}

void RunTests()
{
	size_t passedTests = 0;
	for (size_t i = 0; i < tests.size(); i++)
	{
		const auto& test = tests[i];
		bool result = test.Func();
		if (result)
		{
			std::cout << "[" << i + 1 << " / " << tests.size() << " (" << test.Name << "): Passed\n";
			passedTests++;
		}
		else
		{
			std::cout << "[" << i + 1 << " / " << tests.size() << " (" << test.Name << "): Failed\n"; 
		}
	}
	std::cout << "[NativeTest]: Done. " << passedTests << " passed, " << tests.size() - passedTests  << " failed.";
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

	auto object = hostInstance.CreateInstance("Testing.Managed.MemberMethodTest, Testing.Managed");
	RegisterMemeberMethodTests(hostInstance, object);
	RunTests();
	hostInstance.DestroyInstance(object);

	hostInstance.UnloadAssemblyLoadContext(testingHandle);
	Coral::GC::Collect();

	return 0;
}
