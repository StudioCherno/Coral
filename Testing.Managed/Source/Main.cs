using System;
using System.Diagnostics;
using System.Runtime.InteropServices;

using Coral;
using Coral.Interop;

namespace Testing {

	public class MyTestObject
	{
		public MyTestObject(int s)
		{
			InternalCallsManager.Invoke<Test.Dummy>();
		}
	}

	public class Test
	{
		public delegate void Dummy();
		public delegate int ReturnIntDel();
		
		[UnmanagedCallersOnly]
		public static void TestMain(UnmanagedString InString)
		{
			Console.WriteLine(InString);
		}
	}

}
