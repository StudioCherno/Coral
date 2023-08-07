#pragma once

#include "Memory.hpp"

namespace Coral {

	template<typename TValue>
	class Array
	{
	public:
		Array() = default;

		Array(int32_t InLength)
		{
			m_Ptr = static_cast<TValue*>(Memory::AllocHGlobal(InLength * sizeof(TValue)));
			m_Length = InLength;
		}

		Array(const Array& InOther)
		{
			m_Ptr = static_cast<TValue*>(Memory::AllocHGlobal(InOther.m_Length * sizeof(TValue)));
			m_Length = InOther.m_Length;
			memcpy(m_Ptr, InOther.m_Ptr, InOther.m_Length * sizeof(TValue));
		}

		Array(Array&& InOther) noexcept
		{
			m_Ptr = InOther.m_Ptr;
			m_Length = InOther.m_Length;
			InOther.m_Ptr = nullptr;
			InOther.m_Length = 0;
		}

		~Array()
		{
			Memory::FreeHGlobal(m_Ptr);
		}

		Array& operator=(const Array& InOther)
		{
			Memory::FreeHGlobal(m_Ptr);

			m_Ptr = static_cast<TValue*>(Memory::AllocHGlobal(InOther.m_Length * sizeof(TValue)));
			m_Length = InOther.m_Length;
			memcpy(m_Ptr, InOther.m_Ptr, InOther.m_Length * sizeof(TValue));
		}
		
		Array& operator=(Array&& InOther) noexcept
		{
			Memory::FreeHGlobal(m_Ptr);

			m_Ptr = InOther.m_Ptr;
			m_Length = InOther.m_Length;
			InOther.m_Ptr = nullptr;
			InOther.m_Length = 0;
		}

		TValue& operator[](size_t InIndex) { return m_Ptr[InIndex]; }
		const TValue& operator[](size_t InIndex) const { return m_Ptr[InIndex]; }

		size_t Length() const { return m_Length; }

		TValue* begin() { return m_Ptr; }
		TValue* end() { return m_Ptr + m_Length; }

		const TValue* begin() const { return m_Ptr; }
		const TValue* end() const { return m_Ptr + m_Length; }

		const TValue* cbegin() const { return m_Ptr; }
		const TValue* cend() const { return m_Ptr + m_Length; }

	private:
		TValue* m_Ptr = nullptr;
		int32_t m_Length = 0;
	};

}
