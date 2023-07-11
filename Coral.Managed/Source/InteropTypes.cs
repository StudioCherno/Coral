using System;
using System.Runtime.InteropServices;

namespace Coral.Managed.Interop
{
	[StructLayout(LayoutKind.Sequential)]
	public readonly struct UnmanagedArray
	{
		private readonly IntPtr m_NativeArray;
		private readonly int m_NativeLength;

		public int Length => m_NativeLength;

		public T[] ToArray<T>() where T : struct
		{
			try
			{
				if (m_NativeArray == IntPtr.Zero || m_NativeLength == 0)
					return Array.Empty<T>();

				var result = new T[m_NativeLength];

				for (int i = 0; i < m_NativeLength; i++)
				{
					IntPtr elementPtr = Marshal.ReadIntPtr(m_NativeArray, i * Marshal.SizeOf<nint>());
					result[i] = Marshal.PtrToStructure<T>(elementPtr);
				}

				return result;
			}
			catch (Exception ex)
			{
				ManagedHost.HandleException(ex);
				return Array.Empty<T>();
			}
		}

		public bool IsEmpty() => m_NativeArray == IntPtr.Zero || Length == 0;
		
		public IntPtr[] ToIntPtrArray()
		{
			try
			{
				if (m_NativeArray == IntPtr.Zero || m_NativeLength == 0)
					return Array.Empty<IntPtr>();

				IntPtr[] result = new IntPtr[m_NativeLength];

				for (int i = 0; i < m_NativeLength; i++)
					result[i] = Marshal.ReadIntPtr(m_NativeArray, i * Marshal.SizeOf<nint>());

				return result;
			}
			catch (Exception ex)
			{
				ManagedHost.HandleException(ex);
				return Array.Empty<IntPtr>();
			}
		}
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct UnmanagedString : IEquatable<UnmanagedString>
	{
		private IntPtr m_NativeString;

		public bool IsNull() => m_NativeString == IntPtr.Zero;

		public override string? ToString() => m_NativeString != IntPtr.Zero ? Marshal.PtrToStringAuto(m_NativeString) : string.Empty;

		public static UnmanagedString FromString(string? InValue)
		{
			return new UnmanagedString()
			{
				m_NativeString = Marshal.StringToCoTaskMemAuto(InValue)
			};
		}
		
		public static UnmanagedString Null()
		{
			return new UnmanagedString(){ m_NativeString = IntPtr.Zero };
		}

		[UnmanagedCallersOnly]
		internal static void FreeUnmanaged(UnmanagedString InString)
		{
			InString.Free();
		}

		public void Free() => Marshal.FreeCoTaskMem(m_NativeString);

		public override bool Equals(object? obj) => obj is UnmanagedString other && Equals(other);
		public bool Equals(UnmanagedString other) => m_NativeString == other.m_NativeString;
		public override int GetHashCode() => m_NativeString.GetHashCode();

		public static bool operator ==(UnmanagedString left, UnmanagedString right) => left.Equals(right);
		public static bool operator !=(UnmanagedString left, UnmanagedString right) => !(left == right);

		public static implicit operator string?(UnmanagedString InUnmanagedString) => InUnmanagedString.ToString();
	}

	public struct Bool32
	{
		public readonly uint m_Value;
		public static implicit operator bool(Bool32 InBool32) => InBool32.m_Value > 0;
	}
}