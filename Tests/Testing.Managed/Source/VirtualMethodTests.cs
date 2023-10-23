using System;

namespace Testing.Managed
{
	public class VirtualMethodTests
	{
		public virtual void TestMe() {}
	}

	public class Override1 : VirtualMethodTests
	{
		public override void TestMe()
		{
			Console.WriteLine("Override1");
		}
	}

	public class Override2 : VirtualMethodTests
	{
		public override void TestMe()
		{
			Console.WriteLine("Override2");
		}
	}
}
