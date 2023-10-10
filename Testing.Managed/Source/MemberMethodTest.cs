using System;
using System.Runtime.InteropServices;

namespace Testing.Managed;

[AttributeUsage(AttributeTargets.Method | AttributeTargets.Field | AttributeTargets.Property)]
public class DummyAttribute : Attribute
{
	public float SomeValue;
}

public class MemberMethodTest
{
	
	public struct DummyStruct
	{
		public int X;
		public float Y;
		public int Z;
	}
	
	public sbyte SByteTest(sbyte InValue)
	{
		InValue *= 2;
		return InValue;
	}
	
	public byte ByteTest(byte InValue)
	{
		InValue *= 2;
		return InValue;
	}
	
	public short ShortTest(short InValue)
	{
		InValue *= 2;
		return InValue;
	}
	
	public ushort UShortTest(ushort InValue)
	{
		InValue *= 2;
		return InValue;
	}
	
	public int IntTest(int InValue)
	{
		InValue *= 2;
		return InValue;
	}
	
	public uint UIntTest(uint InValue)
	{
		InValue *= 2;
		return InValue;
	}
	
	public long LongTest(long InValue)
	{
		InValue *= 2;
		return InValue;
	}
	
	public ulong ULongTest(ulong InValue)
	{
		InValue *= 2;
		return InValue;
	}
	
	public float FloatTest(float InValue)
	{
		InValue *= 2.0f;
		return InValue;
	}
	
	public double DoubleTest(double InValue)
	{
		InValue *= 2.0;
		return InValue;
	}
	
	public bool BoolTest(bool InValue)
	{
		InValue = !InValue;
		return InValue;
	}
	
	public IntPtr IntPtrTest(IntPtr InValue)
	{
		Marshal.WriteInt32(InValue, 50);
		return InValue;
	}
	
	public string StringTest(string InValue)
	{
		InValue += ", World!";
		return InValue;
	}

	public DummyStruct DummyStructTest(DummyStruct InValue)
	{
		InValue.X *= 2;
		InValue.Y *= 2.0f;
		InValue.Z *= 2;
		return InValue;
	}

	public int OverloadTest(int InValue)
	{
		return InValue + 1000;
	}

	public float OverloadTest(float InValue)
	{
		return InValue + 10.0f;
	}

	public unsafe DummyStruct* DummyStructPtrTest(DummyStruct* InValue)
	{
		InValue->X *= 2;
		InValue->Y *= 2.0f;
		InValue->Z *= 2;
		return InValue;
	}

	[Dummy(SomeValue = 10.0f)]
	public void SomeFunction(){}

}