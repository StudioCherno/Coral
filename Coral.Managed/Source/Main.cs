using System;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Loader;
using Coral.Managed.Interop;

namespace Coral.Managed
{
	
	internal static class ManagedHost
	{
		private static unsafe delegate*<UnmanagedString, void> s_ExceptionCallback;

		[UnmanagedCallersOnly]
		private static void Initialize()
		{
			var assemblyLoadContexts = AssemblyLoadContext.All;
			
			foreach (var alc in assemblyLoadContexts)
			{
				Console.WriteLine($"Name: {alc.Name}");
				Console.WriteLine($"Assemblies: {alc.Assemblies.Count()}");
				foreach (var assembly in alc.Assemblies)
				{
					Console.WriteLine($"\tName: {assembly.FullName}");
				}
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe void SetExceptionCallback(delegate*<UnmanagedString, void> InCallback)
		{
			s_ExceptionCallback = InCallback;
		}

		internal struct ReflectionType
		{
			public UnmanagedString FullName;
			public UnmanagedString Name;
			public UnmanagedString Namespace;
			public UnmanagedString BaseTypeName;
			public UnmanagedString AssemblyQualifiedName;
		}

		internal enum TypeVisibility
		{
			Public,
			Private,
			Protected,
			Internal,
			ProtectedPublic,
			PrivateProtected
		}
		
		internal static ReflectionType? BuildReflectionType(Type? InType)
		{
			if (InType == null)
				return null;
			
			var reflectionType = new ReflectionType
			{
				FullName = UnmanagedString.FromString(InType.FullName),
				Name = UnmanagedString.FromString(InType.Name),
				Namespace = UnmanagedString.FromString(InType.Namespace),
				AssemblyQualifiedName = UnmanagedString.FromString(InType.AssemblyQualifiedName)
			};

			if (InType.BaseType != null)
			{
				reflectionType.BaseTypeName = UnmanagedString.FromString(InType.BaseType.FullName);
			}
			
			Console.WriteLine($"Building ReflectionType...");
			Console.WriteLine($"FullName: {reflectionType.FullName}");
			Console.WriteLine($"Name: {reflectionType.Name}");
			Console.WriteLine($"Namespace: {reflectionType.Namespace}");
			Console.WriteLine($"BaseTypeName: {reflectionType.BaseTypeName}");
			Console.WriteLine($"AssemblyQualifiedName: {reflectionType.AssemblyQualifiedName}");
			Console.WriteLine($"Done Building ReflectionType...");

			return reflectionType;
		}

		[UnmanagedCallersOnly]
		private static unsafe Bool32 GetReflectionType(UnmanagedString InTypeName, ReflectionType* OutReflectionType)
		{
			try
			{
				var type = TypeHelper.FindType(InTypeName);
				var reflectionType = BuildReflectionType(type);

				if (reflectionType == null)
					return false;

				*OutReflectionType = reflectionType.Value;
				return true;
			}
			catch (Exception ex)
			{
				HandleException(ex);
				return false;
			}
		}

		[UnmanagedCallersOnly]
		private static Bool32 IsTypeAssignableTo(UnmanagedString InTypeName, UnmanagedString InOtherTypeName)
		{
			try
			{
				var type = TypeHelper.FindType(InTypeName);
				var otherType = TypeHelper.FindType(InOtherTypeName);
				return type != null && type.IsAssignableTo(otherType);
			}
			catch (Exception e)
			{
				HandleException(e);
				return false;
			}
		}

		[UnmanagedCallersOnly]
		private static Bool32 IsTypeAssignableFrom(UnmanagedString InTypeName, UnmanagedString InOtherTypeName)
		{
			try
			{
				var type = TypeHelper.FindType(InTypeName);
				var otherType = TypeHelper.FindType(InOtherTypeName);
				return type != null && type.IsAssignableFrom(otherType);
			}
			catch (Exception e)
			{
				HandleException(e);
				return false;
			}
		}

		[UnmanagedCallersOnly]
		private static unsafe Bool32 GetReflectionTypeFromObject(IntPtr InObjectHandle, ReflectionType* OutReflectionType)
		{
			try
			{
				var target = GCHandle.FromIntPtr(InObjectHandle).Target;
				var reflectionType = BuildReflectionType(target.GetType());

				if (reflectionType == null)
					return false;

				*OutReflectionType = reflectionType.Value;
				return true;
			}
			catch (Exception e)
			{
				HandleException(e);
				return false;
			}
		}

		private struct MethodInfo
		{
			public UnmanagedString Name;
			public TypeVisibility Visibility;
		}

		[UnmanagedCallersOnly]
		private static unsafe void GetTypeMethods(UnmanagedString InTypeName, int* InMethodCount, MethodInfo* InMethodArray)
		{
			try
			{
				var type = TypeHelper.FindType(InTypeName);

				if (type == null)
					return;

				var methods = type.GetMethods();

				if (methods == null || methods.Length == 0)
				{
					*InMethodCount = 0;
					return;
				}


			}
			catch (Exception e)
			{
				HandleException(e);
			}
		}

		internal struct ManagedField
		{
			public UnmanagedString Name;
			public TypeVisibility Visibility;
		}

		[UnmanagedCallersOnly]
		private static unsafe void QueryObjectFields(UnmanagedString InTypeName, ManagedField* InFieldsArray, int* OutFieldCount)
		{
			var type = TypeHelper.FindType(InTypeName);

			if (type == null)
			{
				Console.WriteLine("Invalid type");
				*OutFieldCount = 0;
				return;
			}

			var fields = type.GetFields();
			*OutFieldCount = fields.Length;

			if (InFieldsArray == null)
				return;
			
			for (int i = 0; i < fields.Length; i++)
			{
				var field = fields[i];
				var managedField = new ManagedField();
				
				managedField.Name = UnmanagedString.FromString(field.Name);
				
				if (field.IsPublic) managedField.Visibility = TypeVisibility.Public;
				else if (field.IsPrivate) managedField.Visibility = TypeVisibility.Private;
				else if (field.IsFamily) managedField.Visibility = TypeVisibility.Protected;
				else if (field.IsAssembly) managedField.Visibility = TypeVisibility.Internal;
				else if (field.IsFamilyOrAssembly) managedField.Visibility = TypeVisibility.ProtectedPublic;
				else if (field.IsFamilyAndAssembly) managedField.Visibility = TypeVisibility.PrivateProtected;

				InFieldsArray[i] = managedField;
			}
		}

		internal static void HandleException(Exception InException)
		{
			unsafe
			{
				if (s_ExceptionCallback == null)
					return;

				var message = UnmanagedString.FromString(InException.ToString());
				s_ExceptionCallback(message);
				message.Free();
			}
		}

	}
}
