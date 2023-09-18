using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.InteropServices;

namespace Coral.Managed.Interop;

public class NativeArrayEnumerator<T> : IEnumerator<T>
{
	private readonly T[] m_Elements;
	private int m_Index = -1;

	public NativeArrayEnumerator(T[] elements)
	{
		m_Elements = elements;
	}

	public bool MoveNext()
	{
		m_Index++;
		return m_Index < m_Elements.Length;
	}

	void IEnumerator.Reset() => m_Index = -1;
	void IDisposable.Dispose()
	{
		m_Index = -1;
		GC.SuppressFinalize(this);
	}

	object IEnumerator.Current => Current!;

	public T Current
	{
		get
		{
			try
			{
				return m_Elements[m_Index];
			}
			catch (IndexOutOfRangeException)
			{
				throw new InvalidOperationException();
			}
		}
	}

}

[StructLayout(LayoutKind.Sequential)]
public struct NativeArray<T> : IDisposable, IEnumerable<T>
{
	private readonly IntPtr m_NativeArray;
	private readonly int m_NativeLength;

	private bool m_IsDisposed;

	public int Length => m_NativeLength;

	public NativeArray(int InLength)
	{
		m_NativeArray = Marshal.AllocHGlobal(InLength * Marshal.SizeOf<T>());
		m_NativeLength = InLength;
	}

	public NativeArray([DisallowNull] T?[] InValues)
	{
		m_NativeArray = Marshal.AllocHGlobal(InValues.Length * Marshal.SizeOf<T>());
		m_NativeLength = InValues.Length;

		for (int i = 0; i < m_NativeLength; i++)
		{
			var elem = InValues[i];

			if (elem == null)
				continue;

			Marshal.StructureToPtr(elem, IntPtr.Add(m_NativeArray, i * Marshal.SizeOf<T>()), false);
		}
	}

	internal NativeArray(IntPtr InArray, int InLength)
	{
		m_NativeArray = InArray;
		m_NativeLength = InLength;
	}

	public T[] ToArray()
	{
		Span<T> data;
		unsafe { data = new Span<T>(m_NativeArray.ToPointer(), m_NativeLength); }
		return data.ToArray();
	}

	public Span<T> ToSpan()
	{
		unsafe { return new Span<T>(m_NativeArray.ToPointer(), m_NativeLength); }
	}

	public ReadOnlySpan<T> ToReadOnlySpan() => ToSpan();

	public void Dispose()
	{
		if (!m_IsDisposed)
		{
			Marshal.FreeHGlobal(m_NativeArray);
			m_IsDisposed = true;
		}

		GC.SuppressFinalize(this);
	}

	public IEnumerator<T> GetEnumerator() => new NativeArrayEnumerator<T>(this);
	IEnumerator IEnumerable.GetEnumerator() => new NativeArrayEnumerator<T>(this);

	public T? this[int InIndex]
	{
		get => Marshal.PtrToStructure<T>(IntPtr.Add(m_NativeArray, InIndex * Marshal.SizeOf<T>()));
		set => Marshal.StructureToPtr<T>(value, IntPtr.Add(m_NativeArray, InIndex * Marshal.SizeOf<T>()), false);
	}

	public static implicit operator T[](NativeArray<T> InArray) => InArray.ToArray();

}

[StructLayout(LayoutKind.Sequential)]
public struct UnmanagedString : IEquatable<UnmanagedString>
{
	internal IntPtr m_NativeString;

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
	public uint Value { get; set; }

	public static implicit operator Bool32(bool InValue) => new() { Value = InValue ? 1u : 0u };
	public static implicit operator bool(Bool32 InBool32) => InBool32.Value > 0;
}
