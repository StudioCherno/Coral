#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <chrono>
#include <functional>
#include <ranges>

#include <Coral/HostInstance.hpp>
#include <Coral/DotnetServices.hpp>
#include <Coral/GC.hpp>
#include <Coral/Array.hpp>
#include <Coral/Attribute.hpp>

static Coral::Type g_TestsType;

static void ExceptionCallback(std::string_view InMessage)
{
	std::cout << "\033[1;31m " << "Unhandled native exception: " << InMessage << "\033[0m\n";
}

static int8_t SByteMarshalIcall(int8_t InValue) { return InValue * 2; }
static uint8_t ByteMarshalIcall(uint8_t InValue) { return InValue * 2; }
static int16_t ShortMarshalIcall(int16_t InValue) { return InValue * 2; }
static uint16_t UShortMarshalIcall(uint16_t InValue) { return InValue * 2; }
static int32_t IntMarshalIcall(int32_t InValue) { return InValue * 2; }
static uint32_t UIntMarshalIcall(uint32_t InValue) { return InValue * 2; }
static int64_t LongMarshalIcall(int64_t InValue) { return InValue * 2; }
static uint64_t ULongMarshalIcall(uint64_t InValue) { return InValue * 2; }
static float FloatMarshalIcall(float InValue) { return InValue * 2.0f; }
static double DoubleMarshalIcall(double InValue) { return InValue * 2.0; }
static bool BoolMarshalIcall(bool InValue)
{
	std::cout << "C++: " << (uint32_t)InValue << std::endl;
	return !InValue;
}
static int32_t* IntPtrMarshalIcall(int32_t* InValue)
{
	*InValue *= 2;
	return InValue;
}
static Coral::String StringMarshalIcall(Coral::String InStr)
{
	return InStr;
}
static void StringMarshalIcall2(Coral::String InStr)
{
	std::cout << std::string(InStr) << std::endl;
}
static bool TypeMarshalIcall(Coral::ReflectionType InReflectionType)
{
	Coral::Type& type = InReflectionType;
	return type == g_TestsType;
}

struct DummyStruct
{
	int32_t X;
	float Y;
	int32_t Z;
};
static DummyStruct DummyStructMarshalIcall(DummyStruct InStruct)
{
	InStruct.X *= 2;
	InStruct.Y *= 2.0f;
	InStruct.Z *= 2;
	return InStruct;
}

static DummyStruct* DummyStructPtrMarshalIcall(DummyStruct* InStruct)
{
	InStruct->X *= 2;
	InStruct->Y *= 2.0f;
	InStruct->Z *= 2;
	return InStruct;
}

static Coral::Array<int32_t> EmptyArrayIcall()
{
	std::vector<int32_t> empty;
	return Coral::Array<int32_t>::New(empty);
}

static Coral::Array<float> FloatArrayIcall()
{
	std::vector<float> floats = { 5.0f, 10.0f, 15.0f, 50.0f };
	return Coral::Array<float>::New(floats);
}

static Coral::ManagedObject instance;
static Coral::ManagedObject NativeInstanceIcall()
{
	return instance;
}

static void RegisterTestInternalCalls(Coral::ManagedAssembly& InAssembly)
{
	InAssembly.AddInternalCall("Testing.Managed.Tests", "SByteMarshalIcall", reinterpret_cast<void*>(&SByteMarshalIcall));
	InAssembly.AddInternalCall("Testing.Managed.Tests", "ByteMarshalIcall", reinterpret_cast<void*>(&ByteMarshalIcall));
	InAssembly.AddInternalCall("Testing.Managed.Tests", "ShortMarshalIcall", reinterpret_cast<void*>(&ShortMarshalIcall));
	InAssembly.AddInternalCall("Testing.Managed.Tests", "UShortMarshalIcall", reinterpret_cast<void*>(&UShortMarshalIcall));
	InAssembly.AddInternalCall("Testing.Managed.Tests", "IntMarshalIcall", reinterpret_cast<void*>(&IntMarshalIcall));
	InAssembly.AddInternalCall("Testing.Managed.Tests", "UIntMarshalIcall", reinterpret_cast<void*>(&UIntMarshalIcall));
	InAssembly.AddInternalCall("Testing.Managed.Tests", "LongMarshalIcall", reinterpret_cast<void*>(&LongMarshalIcall));
	InAssembly.AddInternalCall("Testing.Managed.Tests", "ULongMarshalIcall", reinterpret_cast<void*>(&ULongMarshalIcall));
	InAssembly.AddInternalCall("Testing.Managed.Tests", "FloatMarshalIcall", reinterpret_cast<void*>(&FloatMarshalIcall));
	InAssembly.AddInternalCall("Testing.Managed.Tests", "DoubleMarshalIcall", reinterpret_cast<void*>(&DoubleMarshalIcall));
	InAssembly.AddInternalCall("Testing.Managed.Tests", "BoolMarshalIcall", reinterpret_cast<void*>(&BoolMarshalIcall));
	InAssembly.AddInternalCall("Testing.Managed.Tests", "IntPtrMarshalIcall", reinterpret_cast<void*>(&IntPtrMarshalIcall));
	InAssembly.AddInternalCall("Testing.Managed.Tests", "StringMarshalIcall", reinterpret_cast<void*>(&StringMarshalIcall));
	InAssembly.AddInternalCall("Testing.Managed.Tests", "StringMarshalIcall2", reinterpret_cast<void*>(&StringMarshalIcall2));
	InAssembly.AddInternalCall("Testing.Managed.Tests", "DummyStructMarshalIcall", reinterpret_cast<void*>(&DummyStructMarshalIcall));
	InAssembly.AddInternalCall("Testing.Managed.Tests", "DummyStructPtrMarshalIcall", reinterpret_cast<void*>(&DummyStructPtrMarshalIcall));
	InAssembly.AddInternalCall("Testing.Managed.Tests", "TypeMarshalIcall", reinterpret_cast<void*>(&TypeMarshalIcall));
	InAssembly.AddInternalCall("Testing.Managed.Tests", "EmptyArrayIcall", reinterpret_cast<void*>(&EmptyArrayIcall));
	InAssembly.AddInternalCall("Testing.Managed.Tests", "FloatArrayIcall", reinterpret_cast<void*>(&FloatArrayIcall));
	InAssembly.AddInternalCall("Testing.Managed.Tests", "NativeInstanceIcall", reinterpret_cast<void*>(&NativeInstanceIcall));
}

struct Test
{
	std::string Name;
	std::function<bool()> Func;
};
static std::vector<Test> tests;

static void RegisterTest(std::string_view InName, std::function<bool()> InFunc)
{
	tests.push_back(Test{ std::string(InName), std::move(InFunc) });
}

static void RegisterMemberMethodTests(Coral::ManagedObject& InObject)
{
	RegisterTest("SByteTest", [&InObject]() mutable{ return InObject.InvokeMethod<int8_t, int8_t>("SByteTest", 10) == 20; });
	RegisterTest("ByteTest", [&InObject]() mutable{ return InObject.InvokeMethod<uint8_t, uint8_t>("ByteTest", 10) == 20; });
	RegisterTest("ShortTest", [&InObject]() mutable{ return InObject.InvokeMethod<int16_t, int16_t>("ShortTest", 10) == 20; });
	RegisterTest("UShortTest", [&InObject]() mutable{ return InObject.InvokeMethod<uint16_t, uint16_t>("UShortTest", 10) == 20; });
	RegisterTest("IntTest", [&InObject]() mutable{ return InObject.InvokeMethod<int32_t, int32_t>("IntTest", 10) == 20; });
	RegisterTest("UIntTest", [&InObject]() mutable{ return InObject.InvokeMethod<uint32_t, uint32_t>("UIntTest", 10) == 20; });
	RegisterTest("LongTest", [&InObject]() mutable{ return InObject.InvokeMethod<int64_t, int64_t>("LongTest", 10) == 20; });
	RegisterTest("ULongTest", [&InObject]() mutable{ return InObject.InvokeMethod<uint64_t, uint64_t>("ULongTest", 10) == 20; });
	RegisterTest("FloatTest", [&InObject]() mutable{ return InObject.InvokeMethod<float, float>("FloatTest", 10.0f) - 20.0f < 0.001f; });
	RegisterTest("DoubleTest", [&InObject]() mutable{ return InObject.InvokeMethod<double, double>("DoubleTest", 10.0) - 20.0 < 0.001; });
	RegisterTest("BoolTest", [&InObject]() mutable { return InObject.InvokeMethod<Coral::Bool32, Coral::Bool32>("BoolTest", false); });
	RegisterTest("IntPtrTest", [&InObject]() mutable{ int32_t v = 10; return *InObject.InvokeMethod<int32_t*, int32_t*>("IntPtrTest", &v) == 50; });
	RegisterTest("StringTest", [&InObject]() mutable
	{
		Coral::ScopedString str = InObject.InvokeMethod<Coral::String, Coral::String>("StringTest", Coral::String::New("Hello"));
		return str == "Hello, World!";
	});
	
	RegisterTest("DummyStructTest", [&InObject]() mutable
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
	RegisterTest("DummyStructPtrTest", [&InObject]() mutable
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

	RegisterTest("OverloadTest", [&InObject]() mutable
	{
		return InObject.InvokeMethod<int32_t, int32_t>("Int32 OverloadTest(Int32)", 50) == 1050;
	});

	RegisterTest("OverloadTest", [&InObject]() mutable
	{
		return InObject.InvokeMethod<float, float>("OverloadTest", 5) == 15.0f;
	});
}

static void RegisterFieldMarshalTests(Coral::ManagedObject& InObject)
{
	RegisterTest("SByteFieldTest", [&InObject]() mutable
	{
		auto value = InObject.GetFieldValue<int8_t>("SByteFieldTest");
		if (value != 10)
			return false;
		InObject.SetFieldValue<int8_t>("SByteFieldTest", 20);
		value = InObject.GetFieldValue<int8_t>("SByteFieldTest");
		return value == 20;
	});

	RegisterTest("ByteFieldTest", [&InObject]() mutable
	{
		auto value = InObject.GetFieldValue<uint8_t>("ByteFieldTest");
		if (value != 10)
			return false;
		InObject.SetFieldValue<uint8_t>("ByteFieldTest", 20);
		value = InObject.GetFieldValue<uint8_t>("ByteFieldTest");
		return value == 20;
	});

	RegisterTest("ShortFieldTest", [&InObject]() mutable
	{
		auto value = InObject.GetFieldValue<int16_t>("ShortFieldTest");
		if (value != 10)
			return false;
		InObject.SetFieldValue<int16_t>("ShortFieldTest", 20);
		value = InObject.GetFieldValue<int16_t>("ShortFieldTest");
		return value == 20;
	});

	RegisterTest("UShortFieldTest", [&InObject]() mutable
	{
		auto value = InObject.GetFieldValue<uint16_t>("UShortFieldTest");
		if (value != 10)
			return false;
		InObject.SetFieldValue<uint16_t>("UShortFieldTest", 20);
		value = InObject.GetFieldValue<uint16_t>("UShortFieldTest");
		return value == 20;
	});

	RegisterTest("IntFieldTest", [&InObject]() mutable
	{
		auto value = InObject.GetFieldValue<int32_t>("IntFieldTest");
		if (value != 10)
			return false;
		InObject.SetFieldValue<int32_t>("IntFieldTest", 20);
		value = InObject.GetFieldValue<int32_t>("IntFieldTest");
		return value == 20;
	});

	RegisterTest("UIntFieldTest", [&InObject]() mutable
	{
		auto value = InObject.GetFieldValue<uint32_t>("UIntFieldTest");
		if (value != 10)
			return false;
		InObject.SetFieldValue<uint32_t>("UIntFieldTest", 20);
		value = InObject.GetFieldValue<uint32_t>("UIntFieldTest");
		return value == 20;
	});

	RegisterTest("LongFieldTest", [&InObject]() mutable
	{
		auto value = InObject.GetFieldValue<int64_t>("LongFieldTest");
		if (value != 10)
			return false;
		InObject.SetFieldValue<int64_t>("LongFieldTest", 20);
		value = InObject.GetFieldValue<int64_t>("LongFieldTest");
		return value == 20;
	});

	RegisterTest("ULongFieldTest", [&InObject]() mutable
	{
		auto value = InObject.GetFieldValue<uint64_t>("ULongFieldTest");
		if (value != 10)
			return false;
		InObject.SetFieldValue<uint64_t>("ULongFieldTest", 20);
		value = InObject.GetFieldValue<uint64_t>("ULongFieldTest");
		return value == 20;
	});

	RegisterTest("FloatFieldTest", [&InObject]() mutable
	{
		auto value = InObject.GetFieldValue<float>("FloatFieldTest");
		if (value - 10.0f > 0.001f)
			return false;
		InObject.SetFieldValue<float>("FloatFieldTest", 20);
		value = InObject.GetFieldValue<float>("FloatFieldTest");
		return value - 20.0f < 0.001f;
	});

	RegisterTest("DoubleFieldTest", [&InObject]() mutable
	{
		auto value = InObject.GetFieldValue<double>("DoubleFieldTest");
		if (value - 10.0 > 0.001)
			return false;
		InObject.SetFieldValue<double>("DoubleFieldTest", 20);
		value = InObject.GetFieldValue<double>("DoubleFieldTest");
		return value - 20.0 < 0.001;
	});
	
	RegisterTest("BoolFieldTest", [&InObject]() mutable
	{
		auto value = InObject.GetFieldValue<bool>("BoolFieldTest");
		if (value != false)
			return false;

		InObject.SetFieldValue<bool>("BoolFieldTest", true);
		value = InObject.GetFieldValue<bool>("BoolFieldTest");

		return static_cast<bool>(value);
	});
	RegisterTest("StringFieldTest", [&InObject]() mutable
	{
		auto value = InObject.GetFieldValue<std::string>("StringFieldTest");
		if (value != "Hello")
			return false;

		InObject.SetFieldValue<std::string>("StringFieldTest", "Hello, World!");
		value = InObject.GetFieldValue<std::string>("StringFieldTest");

		return value == "Hello, World!";
	});

	///// PROPERTIES ////

	RegisterTest("SBytePropertyTest", [&InObject]() mutable
	{
		auto value = InObject.GetPropertyValue<int8_t>("SBytePropertyTest");
		if (value != 10)
			return false;
		InObject.SetPropertyValue<int8_t>("SBytePropertyTest", 20);
		value = InObject.GetPropertyValue<int8_t>("SBytePropertyTest");
		return value == 20;
	});

	RegisterTest("BytePropertyTest", [&InObject]() mutable
	{
		auto value = InObject.GetPropertyValue<uint8_t>("BytePropertyTest");
		if (value != 10)
			return false;
		InObject.SetPropertyValue<uint8_t>("BytePropertyTest", 20);
		value = InObject.GetPropertyValue<uint8_t>("BytePropertyTest");
		return value == 20;
	});

	RegisterTest("ShortPropertyTest", [&InObject]() mutable
	{
		auto value = InObject.GetPropertyValue<int16_t>("ShortPropertyTest");
		if (value != 10)
			return false;
		InObject.SetPropertyValue<int16_t>("ShortPropertyTest", 20);
		value = InObject.GetPropertyValue<int16_t>("ShortPropertyTest");
		return value == 20;
	});

	RegisterTest("UShortPropertyTest", [&InObject]() mutable
	{
		auto value = InObject.GetPropertyValue<uint16_t>("UShortPropertyTest");
		if (value != 10)
			return false;
		InObject.SetPropertyValue<uint16_t>("UShortPropertyTest", 20);
		value = InObject.GetPropertyValue<uint16_t>("UShortPropertyTest");
		return value == 20;
	});

	RegisterTest("IntPropertyTest", [&InObject]() mutable
	{
		auto value = InObject.GetPropertyValue<int32_t>("IntPropertyTest");
		if (value != 10)
			return false;
		InObject.SetPropertyValue<int32_t>("IntPropertyTest", 20);
		value = InObject.GetPropertyValue<int32_t>("IntPropertyTest");
		return value == 20;
	});

	RegisterTest("UIntPropertyTest", [&InObject]() mutable
	{
		auto value = InObject.GetPropertyValue<uint32_t>("UIntPropertyTest");
		if (value != 10)
			return false;
		InObject.SetPropertyValue<uint32_t>("UIntPropertyTest", 20);
		value = InObject.GetPropertyValue<uint32_t>("UIntPropertyTest");
		return value == 20;
	});

	RegisterTest("LongPropertyTest", [&InObject]() mutable
	{
		auto value = InObject.GetPropertyValue<int64_t>("LongPropertyTest");
		if (value != 10)
			return false;
		InObject.SetPropertyValue<int64_t>("LongPropertyTest", 20);
		value = InObject.GetPropertyValue<int64_t>("LongPropertyTest");
		return value == 20;
	});

	RegisterTest("ULongPropertyTest", [&InObject]() mutable
	{
		auto value = InObject.GetPropertyValue<uint64_t>("ULongPropertyTest");
		if (value != 10)
			return false;
		InObject.SetPropertyValue<uint64_t>("ULongPropertyTest", 20);
		value = InObject.GetPropertyValue<uint64_t>("ULongPropertyTest");
		return value == 20;
	});

	RegisterTest("FloatPropertyTest", [&InObject]() mutable
	{
		auto value = InObject.GetPropertyValue<float>("FloatPropertyTest");
		if (value - 10.0f > 0.001f)
			return false;
		InObject.SetPropertyValue<float>("FloatPropertyTest", 20);
		value = InObject.GetPropertyValue<float>("FloatPropertyTest");
		return value - 20.0f < 0.001f;
	});

	RegisterTest("DoublePropertyTest", [&InObject]() mutable
	{
		auto value = InObject.GetPropertyValue<double>("DoublePropertyTest");
		if (value - 10.0 > 0.001)
			return false;
		InObject.SetPropertyValue<double>("DoublePropertyTest", 20);
		value = InObject.GetPropertyValue<double>("DoublePropertyTest");
		return value - 20.0 < 0.001;
	});
	
	RegisterTest("BoolPropertyTest", [&InObject]() mutable
	{
		auto value = InObject.GetPropertyValue<Coral::Bool32>("BoolPropertyTest");
		if (value != false)
			return false;
		InObject.SetPropertyValue<Coral::Bool32>("BoolPropertyTest", true);
		value = InObject.GetPropertyValue<Coral::Bool32>("BoolPropertyTest");
		return static_cast<bool>(value);
	});
	RegisterTest("StringPropertyTest", [&InObject]() mutable
	{
		Coral::ScopedString value = InObject.GetPropertyValue<Coral::String>("StringPropertyTest");
		if (value != std::string_view("Hello"))
			return false;
		InObject.SetPropertyValue("StringPropertyTest", Coral::String::New("Hello, World!"));
		value = InObject.GetPropertyValue<Coral::String>("StringPropertyTest");
		return value == "Hello, World!";
	});
}

static void RunTests()
{
	size_t passedTests = 0;
	for (size_t i = 0; i < tests.size(); i++)
	{
		const auto& test = tests[i];
		bool result = test.Func();
		if (result)
		{
			std::cout << "\033[1;32m[" << i + 1 << " / " << tests.size() << " (" << test.Name << "): Passed\033[0m\n";
			passedTests++;
		}
		else
		{
			std::cerr << "\033[1;31m[" << i + 1 << " / " << tests.size() << " (" << test.Name << "): Failed\033[0m\n"; 
		}
	}
	std::cout << "[NativeTest]: Done. " << passedTests << " passed, " << tests.size() - passedTests  << " failed.\n";
}

int main([[maybe_unused]] int argc, char** argv)
{
	auto exeDir = std::filesystem::path(argv[0]).parent_path();
	auto coralDir = exeDir.string();
	Coral::HostSettings settings =
	{
		.CoralDirectory = coralDir,
		.ExceptionCallback = ExceptionCallback
	};
	Coral::HostInstance hostInstance;
	hostInstance.Initialize(settings);

	//Coral::DotnetServices::RunMSBuild((exeDir.parent_path().parent_path() / "CoralManaged.sln").string());

	std::string testDllPath = exeDir.parent_path().string() + ":" + exeDir.parent_path().parent_path().string();
	auto loadContext = hostInstance.CreateAssemblyLoadContext("TestContext", testDllPath);

	auto assemblyPath = exeDir / "Testing.Managed.dll";
	auto& assembly = loadContext.LoadAssembly(assemblyPath.string());

	RegisterTestInternalCalls(assembly);
	assembly.UploadInternalCalls();

	auto& testsType = assembly.GetType("Testing.Managed.Tests");
	g_TestsType = testsType;
	testsType.InvokeStaticMethod("StaticMethodTest", 50.0f);
	testsType.InvokeStaticMethod("StaticMethodTest", 1000);

	auto& instanceTestType = assembly.GetType("Testing.Managed.InstanceTest");
	instance = instanceTestType.CreateInstance();
	instance.SetFieldValue("X", 500.0f);

	Coral::ManagedObject testsInstance = testsType.CreateInstance();
	testsInstance.InvokeMethod("RunManagedTests");
	testsInstance.Destroy();

	auto& fieldTestType = assembly.GetType("Testing.Managed.FieldMarshalTest");
	std::cout << fieldTestType.IsAssignableTo(fieldTestType) << std::endl;

	auto fieldTestObject = fieldTestType.CreateInstance();

	auto dummyClassInstance = assembly.GetType("Testing.Managed.DummyClass").CreateInstance();
	dummyClassInstance.SetFieldValue("X", 500.0f);

	struct DummyStruct
	{
		float X;
	} ds;
	ds.X = 50.0f;
	fieldTestObject.SetFieldValue("DummyClassTest", dummyClassInstance);
	fieldTestObject.SetFieldValue("DummyStructTest", ds);
	fieldTestObject.InvokeMethod("TestClassAndStruct");
	dummyClassInstance.Destroy();

	for (auto fieldInfo : fieldTestType.GetFields())
	{
		auto attributes = fieldInfo.GetAttributes();
		for (auto attrib : attributes)
		{
			auto& attribType = attrib.GetType();

			if (attribType.GetFullName() == "Testing.Managed.DummyAttribute")
				std::cout << attrib.GetFieldValue<float>("SomeValue") << std::endl;
		}
	}

	for (auto propertyInfo : fieldTestType.GetProperties())
	{
		auto attributes = propertyInfo.GetAttributes();
		for (auto attrib : attributes)
		{
			auto& attribType = attrib.GetType();

			if (attribType.GetFullName() == "Testing.Managed.DummyAttribute")
				std::cout << attrib.GetFieldValue<float>("SomeValue") << std::endl;
		}
	}
	
	auto& memberMethodTestType = assembly.GetType("Testing.Managed.MemberMethodTest");

	// for (auto methodInfo : memberMethodTestType.GetMethods())
	// {
	// 	auto& type = methodInfo.GetReturnType();
	// 	auto accessibility = methodInfo.GetAccessibility();
	// 	std::cout << methodInfo.GetName() << ", Returns: " << type.GetFullName() << std::endl;
	// 	const auto& parameterTypes = methodInfo.GetParameterTypes();
	// 	for (const auto& paramType : parameterTypes)
	// 	{
	// 		std::cout << "\t" << paramType->GetFullName() << std::endl;
	// 	}

	// 	auto attributes = methodInfo.GetAttributes();
	// 	for (auto attrib : attributes)
	// 	{
	// 		auto& attribType = attrib.GetType();

	// 		if (attribType.GetFullName() == "Testing.Managed.DummyAttribute")
	// 			std::cout << attrib.GetFieldValue<float>("SomeValue") << std::endl;
	// 	}
	// }

	auto memberMethodTest = memberMethodTestType.CreateInstance();

	RegisterFieldMarshalTests(fieldTestObject);
	RegisterMemberMethodTests(memberMethodTest);
	RunTests();

	memberMethodTest.Destroy();
	fieldTestObject.Destroy();

	auto& virtualMethodTestType1 = assembly.GetType("Testing.Managed.Override1");
	auto& virtualMethodTestType2 = assembly.GetType("Testing.Managed.Override2");

	auto instance1 = virtualMethodTestType1.CreateInstance();
	auto instance2 = virtualMethodTestType2.CreateInstance();

	instance1.InvokeMethod("TestMe");
	instance2.InvokeMethod("TestMe");

	instance.Destroy();
	instance1.Destroy();
	instance2.Destroy();

	auto loadContext2 = hostInstance.CreateAssemblyLoadContext("ALCTestMulti", testDllPath);
	auto& multiAssembly = loadContext2.LoadAssembly(assemblyPath.string());

	if (&multiAssembly.GetLocalType("Testing.Managed.DummyClass") != &assembly.GetLocalType("Testing.Managed.DummyClass"))
	{
		std::cout << "\033[1;32mMultiple instances of the same DLL seem to be working\033[0m" << std::endl;
	}
	else
	{
		std::cout << "\033[1;31mType cache is clashing between multiple instances of the same DLL\033[0m" << std::endl;
	}

	hostInstance.UnloadAssemblyLoadContext(loadContext);

	Coral::GC::Collect();

	loadContext = hostInstance.CreateAssemblyLoadContext("ALC2", testDllPath);
	auto& newAssembly = loadContext.LoadAssembly(assemblyPath.string());

	RegisterTestInternalCalls(newAssembly);
	newAssembly.UploadInternalCalls();

	auto& testsType2 = newAssembly.GetType("Testing.Managed.Tests");
	g_TestsType = testsType2;

	auto& instanceTestType2 = newAssembly.GetType("Testing.Managed.InstanceTest");
	instance = instanceTestType2.CreateInstance();
	instance.SetFieldValue("X", 500.0f);

	auto& multiInheritanceTestType = newAssembly.GetType("Testing.Managed.MultiInheritanceTest");
	std::cout << "Class: " << std::string(multiInheritanceTestType.GetFullName()) << std::endl;
	std::cout << "\tBase: " << std::string(multiInheritanceTestType.GetBaseType().GetFullName()) << std::endl;
	std::cout << "\tInterfaces:" << std::endl;

	const auto& interfaceTypes = multiInheritanceTestType.GetInterfaceTypes();
	for (const auto& type : interfaceTypes)
	{
		std::cout << "\t\t" << std::string(type->GetFullName()) << std::endl;
	}

	Coral::ManagedObject testsInstance2 = testsType2.CreateInstance();
	testsInstance2.InvokeMethod("RunManagedTests");
	testsInstance2.Destroy();
	instance.Destroy();

	return 0;
}
