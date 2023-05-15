using System;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.InteropServices;

namespace Coral.Interop
{
	[StructLayout(LayoutKind.Sequential)]
	public struct UnmanagedArray
	{
		private IntPtr m_NativeArray;
		private int m_NativeLength;

		public T[] ToArray<T>() where T : struct
		{
			if (m_NativeArray == IntPtr.Zero || m_NativeLength == 0)
				return Array.Empty<T>();

			T[] result = new T[m_NativeLength];

			for (int i = 0; i < m_NativeLength; i++)
			{
				IntPtr elementPtr = Marshal.ReadIntPtr(m_NativeArray, i * Marshal.SizeOf<nint>());
				result[i] = Marshal.PtrToStructure<T>(elementPtr);
			}

			return result;
		}
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct UnmanagedString : IEquatable<UnmanagedString>
	{
		private IntPtr m_NativeString;

		public override string ToString()
		{
			if (m_NativeString == IntPtr.Zero)
				return null;

			return Marshal.PtrToStringAuto(m_NativeString);
		}

		public override bool Equals(object? obj) => obj is UnmanagedString other && Equals(other);
		public bool Equals(UnmanagedString other) => m_NativeString == other.m_NativeString;
		public override int GetHashCode() => m_NativeString.GetHashCode();

		public static bool operator==(UnmanagedString left, UnmanagedString right) => left.Equals(right);
		public static bool operator!=(UnmanagedString left, UnmanagedString right) => !(left == right);

		public static implicit operator string(UnmanagedString InUnmanagedString) => InUnmanagedString.ToString();
	}

}
