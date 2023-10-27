using Coral.Managed.Interop;

using System;

namespace Testing.Managed;

public class DummyClass
{
	public float X;
}

public struct DummyStruct
{
	public float X;
}

public class FieldMarshalTest
{

	public sbyte SByteFieldTest = 10;
	public byte ByteFieldTest = 10;
	public short ShortFieldTest = 10;
	public ushort UShortFieldTest = 10;
	public int IntFieldTest = 10;
	public uint UIntFieldTest = 10;
	public long LongFieldTest = 10;
	public ulong ULongFieldTest = 10;
	public float FloatFieldTest = 10.0f;
	public double DoubleFieldTest = 10.0;
	public bool BoolFieldTest = false;
	public string StringFieldTest = "Hello";
	public DummyClass DummyClassTest;
	public DummyStruct DummyStructTest;

	public sbyte SBytePropertyTest { get; set; } = 10;
	public byte BytePropertyTest { get; set; } = 10;
	public short ShortPropertyTest { get; set; } = 10;
	public ushort UShortPropertyTest { get; set; } = 10;
	public int IntPropertyTest { get; set; } = 10;
	public uint UIntPropertyTest { get; set; } = 10;
	public long LongPropertyTest { get; set; } = 10;
	public ulong ULongPropertyTest { get; set; } = 10;
	public float FloatPropertyTest { get; set; } = 10.0f;
	public double DoublePropertyTest { get; set; } = 10.0;
	public bool BoolPropertyTest { get; set; } = false;
	public string StringPropertyTest { get; set; } = "Hello";

	public int[] IntArrayTest = new[]{ 5, 2, 1, 64 };

	public int[] IntArrayProp { get; set; } = new int[]{ 6, 10, 16, 24 };

	public void ArrayParamTest(NativeArray<float> InArray)
	{
		foreach (var f in InArray)
			Console.WriteLine(f);
	}

	public float[] ArrayReturnTest()
	{
		return new float[]{ 10.0f, 20.0f, 30.0f, 40.0f, 50.0f };
	}

	public void TestClassAndStruct()
	{
		Console.WriteLine(DummyClassTest.X);
		Console.WriteLine(DummyStructTest.X);
	}

	[Dummy(SomeValue = 1000.0f)]
	public float AttributeFieldTest = 50.0f;

	[Dummy(SomeValue = 10000.0f)]
	public float AttributePropertyTest { get; private set; } = 50.0f;

}