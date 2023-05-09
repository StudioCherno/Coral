using System;

namespace Coral {

	public class ManagedHost
	{
		public static int Initialize(IntPtr InArguments, int InArgumentsSize)
		{
			Console.WriteLine($"Hello! Arguments Size: {InArgumentsSize}");
			return 0;
		}
	}

}
