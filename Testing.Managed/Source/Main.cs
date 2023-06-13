using System;
using System.Runtime.InteropServices;

using Coral;
using Coral.Interop;

namespace Testing {

	public class MyTestObject
	{
		public readonly int MyValue;

		public MyTestObject()
		{
			MyValue = 50;
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
