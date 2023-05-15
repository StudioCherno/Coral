using System;
using System.Runtime.InteropServices;

using Coral;
using Coral.Interop;

namespace Testing {

	public class Test
	{
		[UnmanagedCallersOnly]
		public static void TestMain(UnmanagedString InString)
		{
			Console.WriteLine(InString);
		}
	}

}
