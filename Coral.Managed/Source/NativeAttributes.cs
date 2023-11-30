using System;

namespace Coral.Managed
{
	[AttributeUsage(AttributeTargets.Method, AllowMultiple = false, Inherited = false)]
	public class NativeCallableAttribute : Attribute {}

	[AttributeUsage(AttributeTargets.Class | AttributeTargets.Enum | AttributeTargets.Struct, AllowMultiple = false, Inherited = false)]
	public class NativeTypeAttribute : Attribute
	{
		public NativeTypeAttribute(string InNativeTypeName) {}
	}

	[AttributeUsage(AttributeTargets.Parameter, AllowMultiple = false, Inherited = false)]
	public class NativeConstAttribute : Attribute
	{
		public NativeConstAttribute() { }
	}

}
