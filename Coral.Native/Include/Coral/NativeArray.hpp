#pragma once

#include "Memory.hpp"

namespace Coral {

	template<typename TValue>
	class NativeArray
	{
	public:
		NativeArray() = default;

		NativeArray(int32_t InLength)
		{
			m_Ptr = static_cast<TValue*>(Memory::AllocHGlobal(InLength * sizeof(TValue)));
			m_Length = InLength;
		}

		NativeArray(const std::vector<TValue>& InValues)
		{
			m_Ptr = static_cast<TValue*>(Memory::AllocHGlobal(InValues.size() * sizeof(TValue)));
			m_Length = static_cast<int32_t>(InValues.size());
			memcpy(m_Ptr, InValues.data(), InValues.size() * sizeof(TValue));
		}

		NativeArray(const NativeArray& InOther)
		{
			m_Ptr = static_cast<TValue*>(Memory::AllocHGlobal(InOther.m_Length * sizeof(TValue)));
			m_Length = InOther.m_Length;
			memcpy(m_Ptr, InOther.m_Ptr, InOther.m_Length * sizeof(TValue));
		}

		NativeArray(NativeArray&& InOther) noexcept
		{
			m_Ptr = InOther.m_Ptr;
			m_Length = InOther.m_Length;
			InOther.m_Ptr = nullptr;
			InOther.m_Length = 0;
		}

		~NativeArray()
		{
			Memory::FreeHGlobal(m_Ptr);
		}

		NativeArray& operator=(const NativeArray& InOther)
		{
			Memory::FreeHGlobal(m_Ptr);

			m_Ptr = static_cast<TValue*>(Memory::AllocHGlobal(InOther.m_Length * sizeof(TValue)));
			m_Length = InOther.m_Length;
			memcpy(m_Ptr, InOther.m_Ptr, InOther.m_Length * sizeof(TValue));
		}
		
		NativeArray& operator=(NativeArray&& InOther) noexcept
		{
			Memory::FreeHGlobal(m_Ptr);

			m_Ptr = InOther.m_Ptr;
			m_Length = InOther.m_Length;
			InOther.m_Ptr = nullptr;
			InOther.m_Length = 0;
		}

		bool IsEmpty() const { return m_Length == 0 || m_Ptr == nullptr; }

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
		Bool32 m_IsDisposed = false;
	};

}
