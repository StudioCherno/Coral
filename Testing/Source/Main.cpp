#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <chrono>
#include <functional>
#include <source_location>

#include <Coral/HostInstance.hpp>
#include <Coral/GC.hpp>
#include <Coral/Array.hpp>

void ExceptionCallback(std::string_view InMessage)
{
	std::cout << "Unhandled native exception: " << InMessage << std::endl;
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
Coral::TypeId TypeMarshalIcall(Coral::TypeId InTypeId)
{
	std::cout << InTypeId << std::endl;
	return InTypeId;
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

void RegisterTestInternalCalls(Coral::ManagedAssembly& InAssembly)
{
	InAssembly.AddInternalCall("Testing.Managed.Tests", "SByteMarshalIcall", &SByteMarshalIcall);
	InAssembly.AddInternalCall("Testing.Managed.Tests", "ByteMarshalIcall", &ByteMarshalIcall);
	InAssembly.AddInternalCall("Testing.Managed.Tests", "ShortMarshalIcall", &ShortMarshalIcall);
	InAssembly.AddInternalCall("Testing.Managed.Tests", "UShortMarshalIcall", &UShortMarshalIcall);
	InAssembly.AddInternalCall("Testing.Managed.Tests", "IntMarshalIcall", &IntMarshalIcall);
	InAssembly.AddInternalCall("Testing.Managed.Tests", "UIntMarshalIcall", &UIntMarshalIcall);
	InAssembly.AddInternalCall("Testing.Managed.Tests", "LongMarshalIcall", &LongMarshalIcall);
	InAssembly.AddInternalCall("Testing.Managed.Tests", "ULongMarshalIcall", &ULongMarshalIcall);
	InAssembly.AddInternalCall("Testing.Managed.Tests", "FloatMarshalIcall", &FloatMarshalIcall);
	InAssembly.AddInternalCall("Testing.Managed.Tests", "DoubleMarshalIcall", &DoubleMarshalIcall);
	InAssembly.AddInternalCall("Testing.Managed.Tests", "BoolMarshalIcall", &BoolMarshalIcall);
	InAssembly.AddInternalCall("Testing.Managed.Tests", "IntPtrMarshalIcall", &IntPtrMarshalIcall);
	InAssembly.AddInternalCall("Testing.Managed.Tests", "StringMarshalIcall", &StringMarshalIcall);
	InAssembly.AddInternalCall("Testing.Managed.Tests", "DummyStructMarshalIcall", &DummyStructMarshalIcall);
	InAssembly.AddInternalCall("Testing.Managed.Tests", "DummyStructPtrMarshalIcall", &DummyStructPtrMarshalIcall);
	InAssembly.AddInternalCall("Testing.Managed.Tests", "TypeMarshalIcall", &TypeMarshalIcall);
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

void RegisterMemberMethodTests(Coral::HostInstance& InHost, Coral::ManagedObject InObject)
{
	RegisterTest("SByteTest", [InObject]() mutable{ return InObject.InvokeMethod<char8_t, char8_t>("SByteTest", 10) == 20; });
	RegisterTest("ByteTest", [InObject]() mutable{ return InObject.InvokeMethod<uint8_t, uint8_t>("ByteTest", 10) == 20; });
	RegisterTest("ShortTest", [InObject]() mutable{ return InObject.InvokeMethod<int16_t, int16_t>("ShortTest", 10) == 20; });
	RegisterTest("UShortTest", [InObject]() mutable{ return InObject.InvokeMethod<uint16_t, uint16_t>("UShortTest", 10) == 20; });
	RegisterTest("IntTest", [InObject]() mutable{ return InObject.InvokeMethod<int32_t, int32_t>("IntTest", 10) == 20; });
	RegisterTest("UIntTest", [InObject]() mutable{ return InObject.InvokeMethod<uint32_t, uint32_t>("UIntTest", 10) == 20; });
	RegisterTest("LongTest", [InObject]() mutable{ return InObject.InvokeMethod<int64_t, int64_t>("LongTest", 10) == 20; });
	RegisterTest("ULongTest", [InObject]() mutable{ return InObject.InvokeMethod<uint64_t, uint64_t>("ULongTest", 10) == 20; });
	RegisterTest("FloatTest", [InObject]() mutable{ return InObject.InvokeMethod<float, float>("FloatTest", 10.0f) - 20.0f < 0.001f; });
	RegisterTest("DoubleTest", [InObject]() mutable{ return InObject.InvokeMethod<double, double>("DoubleTest", 10.0) - 20.0 < 0.001; });
	RegisterTest("BoolTest", [InObject]() mutable{ return InObject.InvokeMethod<Coral::Bool32, Coral::Bool32>("BoolTest", false); });
	RegisterTest("IntPtrTest", [InObject]() mutable{ int32_t v = 10; return *InObject.InvokeMethod<int32_t*, int32_t*>("IntPtrTest", &v) == 50; });
#if CORAL_WIDE_CHARS
	RegisterTest("StringTest", [InObject, &InHost]() mutable
	{
		const auto* str = InObject.InvokeMethod<const CharType*, const CharType*>("StringTest", CORAL_STR("Hello"));
		bool success = wcscmp(str, CORAL_STR("Hello, World!")) == 0;
		InHost.FreeString(str);
		return success;
	});
#else
	RegisterTest("SByteTest", [&](){ return strcmp(InObject.InvokeMethod<const CharType*, const CharType*>("SByteTest", CORAL_STR("Hello")), CORAL_STR("Hello, World!")) == 0; });
#endif
	
	// TODO(Peter): Struct Marshalling
	RegisterTest("DummyStructTest", [InObject]() mutable
	{
		DummyStruct value =
		{
			.X = 10,
			.Y = 10.0f,
			.Z = 10,
		};
		auto result = InObject.InvokeMethod<DummyStruct, DummyStruct&>("DummyStructTest", value);
		return result.X == 20 && result.Y - 20.0f < 0.001f && result.Z == 20;
	});
	RegisterTest("DummyStructPtrTest", [InObject]() mutable
	{
		DummyStruct value =
		{
			.X = 10,
			.Y = 10.0f,
			.Z = 10,
		};
		auto* result = InObject.InvokeMethod<DummyStruct*, DummyStruct*>("DummyStructPtrTest", &value);
		return result->X == 20 && result->Y - 20.0f < 0.001f && result->Z == 20;
	});
}

void RegisterFieldMarshalTests(Coral::HostInstance& InHost, Coral::ManagedObject InObject)
{
	RegisterTest("SByteFieldTest", [InObject]() mutable
	{
		auto value = InObject.GetFieldValue<char8_t>("SByteFieldTest");
		if (value != 10)
			return false;
		InObject.SetFieldValue<char8_t>("SByteFieldTest", 20);
		value = InObject.GetFieldValue<char8_t>("SByteFieldTest");
		return value == 20;
	});

	RegisterTest("ByteFieldTest", [InObject]() mutable
	{
		auto value = InObject.GetFieldValue<uint8_t>("ByteFieldTest");
		if (value != 10)
			return false;
		InObject.SetFieldValue<uint8_t>("ByteFieldTest", 20);
		value = InObject.GetFieldValue<uint8_t>("ByteFieldTest");
		return value == 20;
	});

	RegisterTest("ShortFieldTest", [InObject]() mutable
	{
		auto value = InObject.GetFieldValue<int16_t>("ShortFieldTest");
		if (value != 10)
			return false;
		InObject.SetFieldValue<int16_t>("ShortFieldTest", 20);
		value = InObject.GetFieldValue<int16_t>("ShortFieldTest");
		return value == 20;
	});

	RegisterTest("UShortFieldTest", [InObject]() mutable
	{
		auto value = InObject.GetFieldValue<uint16_t>("UShortFieldTest");
		if (value != 10)
			return false;
		InObject.SetFieldValue<uint16_t>("UShortFieldTest", 20);
		value = InObject.GetFieldValue<uint16_t>("UShortFieldTest");
		return value == 20;
	});

	RegisterTest("IntFieldTest", [InObject]() mutable
	{
		auto value = InObject.GetFieldValue<int32_t>("IntFieldTest");
		if (value != 10)
			return false;
		InObject.SetFieldValue<int32_t>("IntFieldTest", 20);
		value = InObject.GetFieldValue<int32_t>("IntFieldTest");
		return value == 20;
	});

	RegisterTest("UIntFieldTest", [InObject]() mutable
	{
		auto value = InObject.GetFieldValue<uint32_t>("UIntFieldTest");
		if (value != 10)
			return false;
		InObject.SetFieldValue<uint32_t>("UIntFieldTest", 20);
		value = InObject.GetFieldValue<uint32_t>("UIntFieldTest");
		return value == 20;
	});

	RegisterTest("LongFieldTest", [InObject]() mutable
	{
		auto value = InObject.GetFieldValue<int64_t>("LongFieldTest");
		if (value != 10)
			return false;
		InObject.SetFieldValue<int64_t>("LongFieldTest", 20);
		value = InObject.GetFieldValue<int64_t>("LongFieldTest");
		return value == 20;
	});

	RegisterTest("ULongFieldTest", [InObject]() mutable
	{
		auto value = InObject.GetFieldValue<uint64_t>("ULongFieldTest");
		if (value != 10)
			return false;
		InObject.SetFieldValue<uint64_t>("ULongFieldTest", 20);
		value = InObject.GetFieldValue<uint64_t>("ULongFieldTest");
		return value == 20;
	});

	RegisterTest("FloatFieldTest", [InObject]() mutable
	{
		auto value = InObject.GetFieldValue<float>("FloatFieldTest");
		if (value - 10.0f > 0.001f)
			return false;
		InObject.SetFieldValue<float>("FloatFieldTest", 20);
		value = InObject.GetFieldValue<float>("FloatFieldTest");
		return value - 20.0f < 0.001f;
	});

	RegisterTest("DoubleFieldTest", [InObject]() mutable
	{
		auto value = InObject.GetFieldValue<double>("DoubleFieldTest");
		if (value - 10.0 > 0.001)
			return false;
		InObject.SetFieldValue<double>("DoubleFieldTest", 20);
		value = InObject.GetFieldValue<double>("DoubleFieldTest");
		return value - 20.0 < 0.001;
	});
	
	RegisterTest("BoolFieldTest", [InObject]() mutable
	{
		auto value = InObject.GetFieldValue<Coral::Bool32>("BoolFieldTest");
		if (value != false)
			return false;
		InObject.SetFieldValue<Coral::Bool32>("BoolFieldTest", true);
		value = InObject.GetFieldValue<Coral::Bool32>("BoolFieldTest");
		return static_cast<bool>(value);
	});
#if CORAL_WIDE_CHARS
	RegisterTest("StringFieldTest", [InObject]() mutable
	{
		auto value = InObject.GetFieldValue<const CharType*>("StringFieldTest");
		if (wcscmp(value, CORAL_STR("Hello")) != 0)
			return false;
		InObject.SetFieldValue("StringFieldTest", CORAL_STR("Hello, World!"));
		value = InObject.GetFieldValue<const CharType*>("StringFieldTest");
		return wcscmp(value, CORAL_STR("Hello, World!")) == 0;
	});
#endif

	///// PROPERTIES ////

	RegisterTest("SBytePropertyTest", [InObject]() mutable
	{
		auto value = InObject.GetPropertyValue<char8_t>("SBytePropertyTest");
		if (value != 10)
			return false;
		InObject.SetPropertyValue<char8_t>("SBytePropertyTest", 20);
		value = InObject.GetPropertyValue<char8_t>("SBytePropertyTest");
		return value == 20;
	});

	RegisterTest("BytePropertyTest", [InObject]() mutable
	{
		auto value = InObject.GetPropertyValue<uint8_t>("BytePropertyTest");
		if (value != 10)
			return false;
		InObject.SetPropertyValue<uint8_t>("BytePropertyTest", 20);
		value = InObject.GetPropertyValue<uint8_t>("BytePropertyTest");
		return value == 20;
	});

	RegisterTest("ShortPropertyTest", [InObject]() mutable
	{
		auto value = InObject.GetPropertyValue<int16_t>("ShortPropertyTest");
		if (value != 10)
			return false;
		InObject.SetPropertyValue<int16_t>("ShortPropertyTest", 20);
		value = InObject.GetPropertyValue<int16_t>("ShortPropertyTest");
		return value == 20;
	});

	RegisterTest("UShortPropertyTest", [InObject]() mutable
	{
		auto value = InObject.GetPropertyValue<uint16_t>("UShortPropertyTest");
		if (value != 10)
			return false;
		InObject.SetPropertyValue<uint16_t>("UShortPropertyTest", 20);
		value = InObject.GetPropertyValue<uint16_t>("UShortPropertyTest");
		return value == 20;
	});

	RegisterTest("IntPropertyTest", [InObject]() mutable
	{
		auto value = InObject.GetPropertyValue<int32_t>("IntPropertyTest");
		if (value != 10)
			return false;
		InObject.SetPropertyValue<int32_t>("IntPropertyTest", 20);
		value = InObject.GetPropertyValue<int32_t>("IntPropertyTest");
		return value == 20;
	});

	RegisterTest("UIntPropertyTest", [InObject]() mutable
	{
		auto value = InObject.GetPropertyValue<uint32_t>("UIntPropertyTest");
		if (value != 10)
			return false;
		InObject.SetPropertyValue<uint32_t>("UIntPropertyTest", 20);
		value = InObject.GetPropertyValue<uint32_t>("UIntPropertyTest");
		return value == 20;
	});

	RegisterTest("LongPropertyTest", [InObject]() mutable
	{
		auto value = InObject.GetPropertyValue<int64_t>("LongPropertyTest");
		if (value != 10)
			return false;
		InObject.SetPropertyValue<int64_t>("LongPropertyTest", 20);
		value = InObject.GetPropertyValue<int64_t>("LongPropertyTest");
		return value == 20;
	});

	RegisterTest("ULongPropertyTest", [InObject]() mutable
	{
		auto value = InObject.GetPropertyValue<uint64_t>("ULongPropertyTest");
		if (value != 10)
			return false;
		InObject.SetPropertyValue<uint64_t>("ULongPropertyTest", 20);
		value = InObject.GetPropertyValue<uint64_t>("ULongPropertyTest");
		return value == 20;
	});

	RegisterTest("FloatPropertyTest", [InObject]() mutable
	{
		auto value = InObject.GetPropertyValue<float>("FloatPropertyTest");
		if (value - 10.0f > 0.001f)
			return false;
		InObject.SetPropertyValue<float>("FloatPropertyTest", 20);
		value = InObject.GetPropertyValue<float>("FloatPropertyTest");
		return value - 20.0f < 0.001f;
	});

	RegisterTest("DoublePropertyTest", [InObject]() mutable
	{
		auto value = InObject.GetPropertyValue<double>("DoublePropertyTest");
		if (value - 10.0 > 0.001)
			return false;
		InObject.SetPropertyValue<double>("DoublePropertyTest", 20);
		value = InObject.GetPropertyValue<double>("DoublePropertyTest");
		return value - 20.0 < 0.001;
	});
	
	RegisterTest("BoolPropertyTest", [InObject]() mutable
	{
		auto value = InObject.GetPropertyValue<Coral::Bool32>("BoolPropertyTest");
		if (value != false)
			return false;
		InObject.SetPropertyValue<Coral::Bool32>("BoolPropertyTest", true);
		value = InObject.GetPropertyValue<Coral::Bool32>("BoolPropertyTest");
		return static_cast<bool>(value);
	});
#if CORAL_WIDE_CHARS
	RegisterTest("StringPropertyTest", [InObject]() mutable
	{
		auto value = InObject.GetPropertyValue<const CharType*>("StringPropertyTest");
		if (wcscmp(value, CORAL_STR("Hello")) != 0)
			return false;
		InObject.SetPropertyValue("StringPropertyTest", CORAL_STR("Hello, World!"));
		value = InObject.GetPropertyValue<const CharType*>("StringPropertyTest");
		return wcscmp(value, CORAL_STR("Hello, World!")) == 0;
	});
#endif
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
			std::cerr << "[" << i + 1 << " / " << tests.size() << " (" << test.Name << "): Failed\n"; 
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
		.CoralDirectory = coralDir.c_str(),
		.ExceptionCallback = ExceptionCallback
	};
	Coral::HostInstance hostInstance;
	hostInstance.Initialize(settings);

	auto loadContext = hostInstance.CreateAssemblyLoadContext("TestContext");

	auto assemblyPath = std::filesystem::path("F:/Coral/Build") / ConfigName / "Testing.Managed.dll";
	auto assembly = loadContext.LoadAssembly(assemblyPath.string().c_str());

	const auto& assemblyTypes = assembly.GetTypes();

	RegisterTestInternalCalls(assembly);
	assembly.UploadInternalCalls();
	auto typeId = assembly.GetTypeId("Testing.Managed.Tests, Testing.Managed");
	std::cout << typeId << std::endl;

	Coral::ManagedObject objectHandle = hostInstance.CreateInstance("Testing.Managed.Tests, Testing.Managed");
	objectHandle.InvokeMethod("RunManagedTests");
	hostInstance.DestroyInstance(objectHandle);

	auto fieldTestObject = hostInstance.CreateInstance("Testing.Managed.FieldMarshalTest, Testing.Managed");

	{
		auto array = fieldTestObject.GetFieldValue<Coral::Array<int32_t>>("IntArrayTest");

		for (auto value : array)
			std::cout << value << std::endl;

		array[0] = 999;

		fieldTestObject.SetFieldValue("IntArrayTest", array);

		auto array2 = fieldTestObject.GetFieldValue<Coral::Array<int32_t>>("IntArrayTest");
	
		for (auto value : array2)
			std::cout << value << std::endl;

		std::cout << "Printing in C#" << std::endl;

		Coral::Array<float> floats(5);
		floats[0] = 50.0f;
		floats[1] = 40.0f;
		floats[2] = 30.0f;
		floats[3] = 20.0f;
		floats[4] = 10.0f;
		fieldTestObject.InvokeMethod("ArrayParamTest", floats);

		auto returnedFloats = fieldTestObject.InvokeMethod<Coral::Array<float>>("ArrayReturnTest");

		for (int i = 0; i < returnedFloats.Length(); i++)
			std::cout << returnedFloats[i] << std::endl;
	}

	{
		auto array = fieldTestObject.GetPropertyValue<Coral::Array<int32_t>>("IntArrayProp");

		for (auto value : array)
			std::cout << value << std::endl;

		array[0] = 999;

		fieldTestObject.SetPropertyValue("IntArrayProp", array);

		auto array2 = fieldTestObject.GetPropertyValue<Coral::Array<int32_t>>("IntArrayProp");

		for (auto value : array2)
			std::cout << value << std::endl;
	}

	auto& objectType = fieldTestObject.GetType();
	auto& objectBaseType = objectType.GetBaseType();
	const auto& fields = objectType.GetFields();
	bool f = objectType.IsAssignableTo(objectType.GetBaseType());

	const auto& methods = objectType.GetMethods();

	auto object = hostInstance.CreateInstance("Testing.Managed.MemberMethodTest, Testing.Managed");

	RegisterMemberMethodTests(hostInstance, object);
	RegisterFieldMarshalTests(hostInstance, fieldTestObject);
	RunTests();

	hostInstance.DestroyInstance(object);
	hostInstance.DestroyInstance(fieldTestObject);
	hostInstance.UnloadAssemblyLoadContext(loadContext);
	Coral::GC::Collect();

	return 0;
}
