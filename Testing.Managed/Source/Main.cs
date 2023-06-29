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
			Console.WriteLine(s);
			Console.WriteLine("Hello");

			Stopwatch sw = new Stopwatch();
			int callCount = 10000;
			float totalTime = 0.0f;
			for (int i = 0; i < callCount; i++)
			{
				sw.Start();
				InternalCallsManager.Invoke<Test.Dummy>();
				sw.Stop();
				totalTime += sw.Elapsed.Microseconds;
			}

			Console.WriteLine($"Calling Test.Dummy {callCount} times took an average of {totalTime / callCount} microseconds");

			Console.WriteLine(InternalCallsManager.Invoke<Test.ReturnIntDel, int>());
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
