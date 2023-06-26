#pragma once

#include "Core.hpp"
#include "Assembly.hpp"

#include <unordered_map>

namespace Coral {

	using ErrorCallbackFn = void(*)(const CharType* InMessage);

	struct HostSettings
	{
		/// <summary>
		/// The file path to Coral.runtimeconfig.json (e.g C:\Dev\MyProject\ThirdParty\Coral)
		/// </summary>
		std::string_view CoralDirectory;
		
		ErrorCallbackFn ErrorCallback = nullptr;
	};

	class ObjectHandle
	{
	public:
		bool IsValid() const { return m_Handle != nullptr; }

	private:
		void* m_Handle;

		friend class HostInstance;
	};

	enum class ManagedType
	{
		Unknown = -1,

		SByte, Byte,
		Short, UShort,
		Int, UInt,
		Long, ULong,

		Float, Double,

		Bool,

		Pointer
	};

	class HostInstance
	{
	public:
		void Initialize(HostSettings InSettings);
		AssemblyLoadStatus LoadAssembly(std::string_view InFilePath, AssemblyHandle& OutHandle);

		void AddInternalCall(std::string_view InMethodName, void* InFunctionPtr);

		void UploadInternalCalls();

		template<typename... TArgs>
		ObjectHandle CreateInstance(std::string_view InTypeName, TArgs&&... InArguments)
		{
			constexpr size_t argumentCount = sizeof...(InArguments);

			ObjectHandle result;

			if constexpr (argumentCount > 0)
			{
				const void* argumentsArr[argumentCount];
				ManagedType argumentTypes[argumentCount];
				AddToArray<TArgs...>(argumentsArr, argumentTypes, std::forward<TArgs>(InArguments)..., std::make_index_sequence<argumentCount> {});
				result = CreateInstanceInternal(InTypeName, argumentsArr, argumentTypes, argumentCount);
			}
			else
			{
				result = CreateInstanceInternal(InTypeName, nullptr, nullptr, 0);
			}

			return result;
		}

		void DestroyInstance(ObjectHandle& InObjectHandle);

	private:
		void LoadHostFXR() const;
		void InitializeCoralManaged();
		void LoadCoralFunctions();

		void* LoadCoralManagedFunctionPtr(const std::filesystem::path& InAssemblyPath, const CharType* InTypeName, const CharType* InMethodName, const CharType* InDelegateType = CORAL_UNMANAGED_CALLERS_ONLY) const;

		template<typename TFunc>
		TFunc LoadCoralManagedFunctionPtr(const CharType* InTypeName, const CharType* InMethodName, const CharType* InDelegateType = CORAL_UNMANAGED_CALLERS_ONLY) const
		{
			return (TFunc)LoadCoralManagedFunctionPtr(m_CoralManagedAssemblyPath, InTypeName, InMethodName, InDelegateType);
		}

		template<typename TArg, size_t TIndex>
		void AddToArrayI(const void** InArgumentsArr, ManagedType* InArgumentTypesArr, TArg&& InArg) const
		{
			if constexpr (std::is_pointer_v<std::remove_reference_t<TArg>>)
			{
				InArgumentsArr[TIndex] = reinterpret_cast<const void*>(InArg);
				InArgumentTypesArr[TIndex] = ManagedType::Pointer;
			}
			else
			{
				InArgumentsArr[TIndex] = reinterpret_cast<const void*>(&InArg);

				if constexpr (std::same_as<TArg, uint8_t>)
					InArgumentTypesArr[TIndex] = ManagedType::Byte;
				else if constexpr (std::same_as<TArg, uint16_t>)
					InArgumentTypesArr[TIndex] = ManagedType::UShort;
				else if constexpr (std::same_as<TArg, uint32_t> || (std::same_as<TArg, unsigned long> && sizeof(TArg) == 4))
					InArgumentTypesArr[TIndex] = ManagedType::UInt;
				else if constexpr (std::same_as<TArg, uint64_t> || (std::same_as<TArg, unsigned long> && sizeof(TArg) == 8))
					InArgumentTypesArr[TIndex] = ManagedType::ULong;
				else if constexpr (std::same_as<TArg, char8_t>)
					InArgumentTypesArr[TIndex] = ManagedType::SByte;
				else if constexpr (std::same_as<TArg, int16_t>)
					InArgumentTypesArr[TIndex] = ManagedType::Short;
				else if constexpr (std::same_as<TArg, int32_t> || (std::same_as<TArg, long> && sizeof(TArg) == 4))
					InArgumentTypesArr[TIndex] = ManagedType::Int;
				else if constexpr (std::same_as<TArg, int64_t> || (std::same_as<TArg, long> && sizeof(TArg) == 8))
					InArgumentTypesArr[TIndex] = ManagedType::Long;
				else if constexpr (std::same_as<TArg, float>)
					InArgumentTypesArr[TIndex] = ManagedType::Float;
				else if constexpr (std::same_as<TArg, double>)
					InArgumentTypesArr[TIndex] = ManagedType::Double;
				else if constexpr (std::same_as<TArg, bool>)
					InArgumentTypesArr[TIndex] = ManagedType::Bool;
			}
		}

		template<typename... TArgs, size_t... TIndices>
		void AddToArray(const void** InArgumentsArr, ManagedType* InArgumentTypesArr, TArgs&&... InArgs, const std::index_sequence<TIndices...>&)
		{
			(AddToArrayI<TArgs, TIndices>(InArgumentsArr, InArgumentTypesArr, std::forward<TArgs>(InArgs)), ...);
		}

		ObjectHandle CreateInstanceInternal(std::string_view InTypeName, const void** InParameters, ManagedType* InParameterTypes, size_t InLength);

	public:
		struct InternalCall
		{
			const CharType* Name;
			void* NativeFunctionPtr;
		};

	private:
		HostSettings m_Settings;
		std::filesystem::path m_CoralManagedAssemblyPath;
		void* m_HostFXRContext = nullptr;
		bool m_Initialized = false;

	#if defined(CORAL_WIDE_CHARS)
		std::vector<std::wstring> m_InternalCallNameStorage;
	#else
		std::vector<std::string> m_InternalCallNameStorage;
	#endif
		std::vector<InternalCall*> m_InternalCalls;
	};

}
