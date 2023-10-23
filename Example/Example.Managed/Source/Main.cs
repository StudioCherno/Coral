using Coral.Managed.Interop;

using System;

namespace Example.Managed {

	[AttributeUsage(AttributeTargets.Class)]
	public sealed class CustomAttribute : Attribute
	{
		public float Value;
	}

	[Custom(Value = -2500.0f)]
	public class ExampleClass
	{

		public struct MyVec3
		{
			public float X;
			public float Y;
			public float Z;
		}

		internal static unsafe delegate*<MyVec3*, MyVec3*, void> VectorAddIcall;
		internal static unsafe delegate*<NativeString, void> PrintStringIcall;
		internal static unsafe delegate*<NativeArray<float>, void> NativeArrayIcall;
		internal static unsafe delegate*<NativeArray<float>> ArrayReturnIcall;

		private int myPrivateValue;

		public ExampleClass(int someValue)
		{
			Console.WriteLine($"Example({someValue})");
		}

		public static void StaticMethod(float value)
		{
			Console.WriteLine($"StaticMethod: {value}");
		}

		public void MemberMethod(MyVec3 vec3)
		{
			MyVec3 anotherVector = new()
			{
				X = 10,
				Y = 20,
				Z = 30
			};

			unsafe { VectorAddIcall(&vec3, &anotherVector); }

			Console.WriteLine($"X: {vec3.X}, Y: {vec3.Y}, Z: {vec3.Z}");
		}

		public void StringDemo()
		{
			NativeString str = "Hello, World?";
			unsafe { PrintStringIcall(str); }
		}

		public void ArrayDemo(float[] InArray)
		{
			NativeArray<float> arr = new(InArray);
			unsafe { NativeArrayIcall(arr); }

			unsafe
			{
				// We use "using" here so that nativeArr is automatically disposed of at
				// the end of this scope
				using var nativeArr = ArrayReturnIcall();

				foreach (var v in nativeArr)
					Console.WriteLine(v);
			}
		}

		public int PublicProp
		{
			get => myPrivateValue;
			set => myPrivateValue = value * 2;
		}

	}

}
