#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <chrono>
#include <functional>
#include <ranges>

#include <Coral/HostInstance.hpp>
#include <Coral/GC.hpp>
#include <Coral/Array.hpp>
#include <Coral/Attribute.hpp>

void ExceptionCallback(std::string_view InMessage)
{
	std::cout << "Unhandled native exception: " << InMessage << std::endl;
}

struct MyVec3
{
	float X;
	float Y;
	float Z;
};

void VectorAddIcall(MyVec3* InVec0, const MyVec3* InVec1)
{
	std::cout << "VectorAddIcall" << std::endl;
	InVec0->X += InVec1->X;
	InVec0->Y += InVec1->Y;
	InVec0->Z += InVec1->Z;
}

void PrintStringIcall(Coral::String InString)
{
	std::cout << std::string(InString) << std::endl;
}

void NativeArrayIcall(Coral::Array<float> InValues)
{
	std::cout << "NativeArrayIcall" << std::endl;
	for (auto value : InValues)
	{
		std::cout << value << std::endl;
	}
}

Coral::Array<float> ArrayReturnIcall()
{
	std::cout << "ArrayReturnIcall" << std::endl;
	return Coral::Array<float>::New({ 10.0f, 5000.0f, 1000.0f });
}

int main(int argc, char** argv)
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

	auto loadContext = hostInstance.CreateAssemblyLoadContext("ExampleContext");

	auto assemblyPath = exeDir / "Example.Managed.dll";
	auto& assembly = loadContext.LoadAssembly(assemblyPath.string());

	assembly.AddInternalCall("Example.Managed.ExampleClass", "VectorAddIcall",   reinterpret_cast<void*>(&VectorAddIcall));
	assembly.AddInternalCall("Example.Managed.ExampleClass", "PrintStringIcall", reinterpret_cast<void*>(&PrintStringIcall));
	assembly.AddInternalCall("Example.Managed.ExampleClass", "NativeArrayIcall", reinterpret_cast<void*>(&NativeArrayIcall));
	assembly.AddInternalCall("Example.Managed.ExampleClass", "ArrayReturnIcall", reinterpret_cast<void*>(&ArrayReturnIcall));
	assembly.UploadInternalCalls();

	// Get a reference to the ExampleClass type
	auto& exampleType = assembly.GetType("Example.Managed.ExampleClass");

	// Call the static method "StaticMethod" with value 50
	exampleType.InvokeStaticMethod("StaticMethod", 50.0f);

	// Get a reference to the CustomAttribute type
	auto& customAttributeType = assembly.GetType("Example.Managed.CustomAttribute");

	// Get a list of all attributes on the exampleType class
	auto exampleTypeAttribs = exampleType.GetAttributes();
	for (auto& attribute : exampleTypeAttribs)
	{
		if (attribute.GetType() == customAttributeType)
		{
			// Get the value of "Value" from the CustomAttribute attribute
			std::cout << "CustomAttribute: " << attribute.GetFieldValue<float>("Value") << std::endl;
		}
	}

	// Create an instance of type Example.Managed.ExampleClass and pass 50 to the constructor
	auto exampleInstance = exampleType.CreateInstance(50);

	// Invoke the method named "MemberMethod" with a MyVec3 argument (doesn't return anything)
	exampleInstance.InvokeMethod("Void MemberMethod(MyVec3)", MyVec3 { 10.0f, 10.0f, 10.0f });

	// Invokes the setter on PublicProp with the value 10 (will be multiplied by 2 in C#)
	exampleInstance.SetPropertyValue("PublicProp", 10);

	// Get the value of PublicProp as an int
	std::cout << exampleInstance.GetPropertyValue<int32_t>("PublicProp") << std::endl;

	// Sets the value of the private field "myPrivateValue" with the value 10 (will NOT be multiplied by 2 in C#)
	exampleInstance.SetFieldValue("myPrivateValue", 10);

	// Get the value of myPrivateValue as an int
	std::cout << exampleInstance.GetFieldValue<int32_t>("myPrivateValue") << std::endl;

	// Invokes StringDemo method which will in turn invoke PrintStringIcall with a string parameter
	exampleInstance.InvokeMethod("StringDemo");

	// Invokes ArrayDemo method which will in turn invoke NativeArrayIcall and pass the values we give here
	// and also invoke ArrayReturnIcall
	auto arr = Coral::Array<float>::New({ 5.0f, 0.0f, 10.0f, -50.0f });
	exampleInstance.InvokeMethod("ArrayDemo", arr);
	Coral::Array<float>::Free(arr);

	return 0;
}
