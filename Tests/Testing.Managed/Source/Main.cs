using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;

using Coral.Managed.Interop;

namespace Testing.Managed {

	public class InstanceTest
	{
		public float X = 50.0f;

		public float Stuff()
		{
			return X * 10.0f;
		}
	}

	public class Tests
	{
		internal static unsafe delegate*<sbyte, sbyte> SByteMarshalIcall;
		internal static unsafe delegate*<byte, byte> ByteMarshalIcall;
		internal static unsafe delegate*<short, short> ShortMarshalIcall;
		internal static unsafe delegate*<ushort, ushort> UShortMarshalIcall;
		internal static unsafe delegate*<int, int> IntMarshalIcall;
		internal static unsafe delegate*<uint, uint> UIntMarshalIcall;
		internal static unsafe delegate*<long, long> LongMarshalIcall;
		internal static unsafe delegate*<ulong, ulong> ULongMarshalIcall;
		internal static unsafe delegate*<float, float> FloatMarshalIcall;
		internal static unsafe delegate*<double, double> DoubleMarshalIcall;
		internal static unsafe delegate*<bool, bool> BoolMarshalIcall;
		internal static unsafe delegate*<IntPtr, IntPtr> IntPtrMarshalIcall;
		internal static unsafe delegate*<NativeString, NativeString> StringMarshalIcall;
		internal static unsafe delegate*<NativeString, void> StringMarshalIcall2;
		internal static unsafe delegate*<ReflectionType, bool> TypeMarshalIcall;
		internal static unsafe delegate*<NativeArray<float>> FloatArrayIcall;
		internal static unsafe delegate*<NativeArray<int>> EmptyArrayIcall;
		internal static unsafe delegate*<NativeInstance<InstanceTest>> NativeInstanceIcall;

		internal struct DummyStruct
		{
			public int X;
			public float Y;
			public int Z;
		}
		internal static unsafe delegate*<DummyStruct, DummyStruct> DummyStructMarshalIcall;
		internal static unsafe delegate*<DummyStruct*, DummyStruct*> DummyStructPtrMarshalIcall;
		
		public static void StaticMethodTest(float value)
		{
			Console.WriteLine(value);
		}

		public static void StaticMethodTest(int value)
		{
			Console.WriteLine(value);
		}

		[Test]
		public bool SByteMarshalTest()
		{
			unsafe { return SByteMarshalIcall(10) == 20; }
		}
		
		[Test]
		public bool ByteMarshalTest()
		{
			unsafe { return ByteMarshalIcall(10) == 20; }
		}
		
		[Test]
		public bool ShortMarshalTest()
		{
			unsafe { return ShortMarshalIcall(10) == 20; }
		}
		
		[Test]
		public bool UShortMarshalTest()
		{
			unsafe { return UShortMarshalIcall(10) == 20; }
		}
		
		[Test]
		public bool IntMarshalTest()
		{
			unsafe { return IntMarshalIcall(10) == 20; }
		}

		[Test]
		public bool UIntMarshalTest()
		{
			unsafe { return UIntMarshalIcall(10) == 20; }
		}
		
		[Test]
		public bool LongMarshalTest()
		{
			unsafe { return LongMarshalIcall(10) == 20; }
		}
		
		[Test]
		public bool ULongMarshalTest()
		{
			unsafe { return ULongMarshalIcall(10) == 20; }
		}
		
		[Test]
		public bool FloatMarshalTest()
		{
			unsafe { return Math.Abs(FloatMarshalIcall(10.0f) - 20.0f) < 0.001f; }
		}
		
		[Test]
		public bool DoubleMarshalTest()
		{
			unsafe { return Math.Abs(DoubleMarshalIcall(10.0) - 20.0) < 0.001; }
		}
		
		[Test]
		public bool BoolMarshalTest()
		{
			unsafe { return BoolMarshalIcall(false); }
		}

		[Test]
		public bool EmptyArrayTest()
		{
			unsafe
			{
				using var arr = EmptyArrayIcall();
				foreach (var item in arr)
				{
					Console.WriteLine(item);
				}
			}

			return true;
		}

		[Test]
		public bool FloatArrayTest()
		{
			float[] requiredValues = new[]{ 5.0f, 10.0f, 15.0f, 50.0f };

			unsafe
			{
				using var arr = FloatArrayIcall();
				for (int i = 0; i < arr.Length; i++)
				{
					if (requiredValues[i] != arr[i])
						return false;
				}
			}

			return true;
		}

		[Test]
		public bool IntPtrMarshalTest()
		{
			IntPtr data = Marshal.AllocCoTaskMem(Marshal.SizeOf<int>());
			Marshal.WriteInt32(data, 50);
			bool success;
			unsafe { success = Marshal.ReadInt32(IntPtrMarshalIcall(data), 0) == 100; }
			Marshal.FreeCoTaskMem(data);
			return success;
		}

		[Test]
		public bool StringMarshalTest()
		{
			unsafe { return StringMarshalIcall("Hello") == "Hello"; }
		}

		[Test]
		public bool StringMarshalTest2()
		{
			unsafe { StringMarshalIcall2("Hello, World!"); }
			return true;
		}

		[Test]
		public bool DummyStructMarshalTest()
		{
			DummyStruct s = new()
			{
				X = 10,
				Y = 15.0f,
				Z = 100
			};

			unsafe
			{
				var newS = DummyStructMarshalIcall(s);
				return newS.X == 20 && Math.Abs(newS.Y) - 30.0f < 0.001f && newS.Z == 200;
			}
		}

		[Test]
		public bool DummyStructPtrMarshalTest()
		{
			DummyStruct s = new()
			{
				X = 10,
				Y = 15.0f,
				Z = 100
			};

			unsafe
			{
				var newS = DummyStructPtrMarshalIcall(&s);
				return newS->X == 20 && Math.Abs(newS->Y) - 30.0f < 0.001f && newS->Z == 200;
			}
		}

		[Test]
		public bool TypeMarshalTest()
		{
			var t = typeof(Tests);
			unsafe { return TypeMarshalIcall(t); }
		}

		[Test]
		public bool NativeInstanceTest()
		{
			InstanceTest? instanceTest;
			unsafe { instanceTest = NativeInstanceIcall(); }
			return instanceTest?.X == 500.0f;
		}

		public void RunManagedTests()
		{
			CollectTests();
			
			foreach (var test in s_Tests)
				test.Run();
			
			Console.WriteLine($"[Test]: Done. {s_PassedTests} passed, {s_Tests.Count - s_PassedTests} failed.");
		}

		private void CollectTests()
		{
			var methods = GetType().GetMethods().Where(methodInfo => methodInfo.GetCustomAttributes(typeof(TestAttribute), false).Any());
			foreach (var method in methods)
				s_Tests.Add(new TestContainer(method.Name, s_Tests.Count + 1, () => (bool)method.Invoke(this, null)));
		}
		
		[AttributeUsage(AttributeTargets.Method)]
		private class TestAttribute : Attribute {}
		
		private class TestContainer
		{
			private readonly string m_Name;
			private readonly int m_TestIndex;
			private readonly Func<bool> m_Func;
			
			internal TestContainer(string InName, int InTestIndex, Func<bool> InFunction)
			{
				m_Name = InName;
				m_TestIndex = InTestIndex;
				m_Func = InFunction;
			}

			public void Run()
			{
				bool result = m_Func();
				if (result)
				{
					Console.ForegroundColor = ConsoleColor.Green;
					Console.WriteLine($"[{m_TestIndex} / {s_Tests.Count} ({m_Name})]: Passed");
					s_PassedTests++;
				}
				else
				{
					Console.ForegroundColor = ConsoleColor.DarkRed;
					Console.WriteLine($"[{m_TestIndex} / {s_Tests.Count} ({m_Name})]: Failed");
				}
			}
		}

		private static List<TestContainer> s_Tests;
		private static int s_PassedTests;

		public Tests()
		{
			s_Tests = new List<TestContainer>();
		}

	}

}
