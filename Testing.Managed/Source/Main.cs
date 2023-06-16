using System;
using System.Runtime.InteropServices;

using Coral;
using Coral.Interop;

namespace Testing {

	public class MyTestObject
	{
		public readonly int MyValue;

		public MyTestObject(int value, float fvalue, IntPtr str)
		{
			MyValue = value;
			Console.WriteLine(value);
			Console.WriteLine(fvalue);
			Console.WriteLine(Marshal.PtrToStringAuto(str));
		}
	}

	public class Test
	{
		[UnmanagedCallersOnly]
		public static void TestMain(UnmanagedString InString)
		{
			Console.WriteLine(InString);
		}
	}

}
