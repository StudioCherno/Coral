using Coral.Managed.Interop;

using System;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace Testing.Managed {

	internal static class InternalCalls
	{
		internal static unsafe delegate*<void> Dummy;
	}

	public class MyTestObject
	{
		public MyTestObject(int s)
		{
			var sw = Stopwatch.StartNew();
			unsafe { InternalCalls.Dummy(); }
			sw.Stop();
			Console.WriteLine($"Elapsed: {sw.Elapsed.TotalMicroseconds} microseconds");
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
