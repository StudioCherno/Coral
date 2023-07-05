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
			const int iterations = 1000 * 1000;
			var sw = Stopwatch.StartNew();
			unsafe
			{
				for (int i = 0; i < iterations; i++)
					InternalCalls.Dummy();
			}
			sw.Stop();
			Console.WriteLine($"Elapsed (Avg): {sw.Elapsed.TotalMicroseconds / iterations} microseconds (Total Iterations: {iterations}, Total Time: {sw.Elapsed.TotalMilliseconds}ms)");
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
