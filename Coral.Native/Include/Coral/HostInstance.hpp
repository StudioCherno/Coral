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
		void UnloadAssemblyLoadContext(AssemblyHandle InAssemblyHandle);

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

		template<typename... TArgs>
		void InvokeMethod(ObjectHandle InHandle, std::string_view InMethodName, TArgs&&... InParameters)
		{
			constexpr size_t parameterCount = sizeof...(InParameters);

			if constexpr (parameterCount > 0)
			{
				const void* parameterValues[parameterCount];
				ManagedType parameterTypes[parameterCount];
				AddToArray<TArgs...>(parameterValues, parameterTypes, std::forward<TArgs>(InParameters)..., std::make_index_sequence<parameterCount>{});
				InvokeMethodInternal(InHandle, InMethodName, parameterTypes, parameterValues, parameterCount);
			}
			else
			{
				InvokeMethodInternal(InHandle, InMethodName, nullptr, nullptr, 0);
			}
		}

		template<typename TRet, typename... TArgs>
		TRet InvokeMethodRet(ObjectHandle InHandle, std::string_view InMethodName, TArgs&&... InParameters)
		{
			constexpr size_t parameterCount = sizeof...(InParameters);

			TRet result;
			ManagedType resultType = GetManagedType<TRet>();
			
			if constexpr (parameterCount > 0)
			{
				const void* parameterValues[parameterCount];
				ManagedType parameterTypes[parameterCount];
				AddToArray<TArgs...>(parameterValues, parameterTypes, std::forward<TArgs>(InParameters)..., std::make_index_sequence<parameterCount>{});
				InvokeMethodRetInternal(InHandle, InMethodName, parameterTypes, parameterValues, parameterCount, &result, sizeof(TRet), resultType);
			}
			else
			{
				InvokeMethodRetInternal(InHandle, InMethodName, nullptr, nullptr, 0, &result, sizeof(TRet), resultType);
			}

			return result;
		}
		void DestroyInstance(ObjectHandle& InObjectHandle);

		using ExceptionCallbackFn = void(*)(const CharType*);
		void SetExceptionCallback(ExceptionCallbackFn InCallback);

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

		template<typename TArg>
		constexpr ManagedType GetManagedType() const
		{
			if constexpr (std::is_pointer_v<std::remove_reference_t<TArg>>)
				return ManagedType::Pointer;
			else if constexpr (std::same_as<TArg, uint8_t>)
				return ManagedType::Byte;
			else if constexpr (std::same_as<TArg, uint16_t>)
				return ManagedType::UShort;
			else if constexpr (std::same_as<TArg, uint32_t> || (std::same_as<TArg, unsigned long> && sizeof(TArg) == 4))
				return ManagedType::UInt;
			else if constexpr (std::same_as<TArg, uint64_t> || (std::same_as<TArg, unsigned long> && sizeof(TArg) == 8))
				return ManagedType::ULong;
			else if constexpr (std::same_as<TArg, char8_t>)
				return ManagedType::SByte;
			else if constexpr (std::same_as<TArg, int16_t>)
				return ManagedType::Short;
			else if constexpr (std::same_as<TArg, int32_t> || (std::same_as<TArg, long> && sizeof(TArg) == 4))
				return ManagedType::Int;
			else if constexpr (std::same_as<TArg, int64_t> || (std::same_as<TArg, long> && sizeof(TArg) == 8))
				return ManagedType::Long;
			else if constexpr (std::same_as<TArg, float>)
				return ManagedType::Float;
			else if constexpr (std::same_as<TArg, double>)
				return ManagedType::Double;
			else if constexpr (std::same_as<TArg, bool>)
				return ManagedType::Bool;
			else
				return ManagedType::Unknown;
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
				InArgumentTypesArr[TIndex] = GetManagedType<TArg>();
			}
		}

		template<typename... TArgs, size_t... TIndices>
		void AddToArray(const void** InArgumentsArr, ManagedType* InArgumentTypesArr, TArgs&&... InArgs, const std::index_sequence<TIndices...>&)
		{
			(AddToArrayI<TArgs, TIndices>(InArgumentsArr, InArgumentTypesArr, std::forward<TArgs>(InArgs)), ...);
		}

		ObjectHandle CreateInstanceInternal(std::string_view InTypeName, const void** InParameters, ManagedType* InParameterTypes, size_t InLength);
		void InvokeMethodInternal(ObjectHandle InObjectHandle, std::string_view InMethodName, ManagedType* InParameterTypes, const void** InParameters, size_t InLength);
		void InvokeMethodRetInternal(ObjectHandle InObjectHandle, std::string_view InMethodName, ManagedType* InParameterTypes, const void** InParameters, size_t InLength, void* InResultStorage, uint64_t InResultSize, ManagedType InResultType);

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
