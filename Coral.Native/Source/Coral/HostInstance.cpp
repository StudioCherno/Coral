#include "HostInstance.hpp"
#include "Verify.hpp"
#include "HostFXRErrorCodes.hpp"
#include "Interop.hpp"
#include "CoralManagedFunctions.hpp"
#include "StringHelper.hpp"

namespace Coral {

	struct CoreCLRFunctions
	{
		hostfxr_set_error_writer_fn SetHostFXRErrorWriter = nullptr;
		hostfxr_initialize_for_runtime_config_fn InitHostFXRForRuntimeConfig = nullptr;
		hostfxr_get_runtime_delegate_fn GetRuntimeDelegate = nullptr;
		hostfxr_close_fn CloseHostFXR = nullptr;
		load_assembly_and_get_function_pointer_fn GetManagedFunctionPtr = nullptr;
	};
	static CoreCLRFunctions s_CoreCLRFunctions;

	ErrorCallbackFn ErrorCallback = nullptr;

	struct ObjectCreateInfo
	{
		const CharType* TypeName;
		bool IsWeakRef;
		const void** Parameters;
		ManagedType* ParameterTypes;
		int32_t Length;
	};

	void DefaultErrorCallback(const CharType* InMessage)
	{
#if CORAL_WIDE_CHARS
		std::wcout << L"[Coral.Native]: " << InMessage << std::endl;
#else
		std::cout << L"[Coral.Native]: " << InMessage << std::endl;
#endif
	}

	void HostInstance::Initialize(HostSettings InSettings)
	{
		CORAL_VERIFY(!m_Initialized);

		LoadHostFXR();

		// Setup settings
		m_Settings = std::move(InSettings);

		if (!m_Settings.ErrorCallback)
			m_Settings.ErrorCallback = DefaultErrorCallback;
		ErrorCallback = m_Settings.ErrorCallback;

		s_CoreCLRFunctions.SetHostFXRErrorWriter([](const CharType* InMessage)
		{
			ErrorCallback(InMessage);
		});

		m_CoralManagedAssemblyPath = std::filesystem::path(m_Settings.CoralDirectory) / "Coral.Managed.dll";

		InitializeCoralManaged();

		m_Initialized = true;
	}

	AssemblyLoadStatus HostInstance::LoadAssembly(std::string_view InFilePath, AssemblyHandle& OutHandle)
	{
		static uint16_t s_NextAssemblyID = 0;

		auto filepath = StringHelper::ConvertUtf8ToWide(InFilePath);

		OutHandle.m_AssemblyID = s_NextAssemblyID++;
		AssemblyLoadStatus loadStatus = s_ManagedFunctions.LoadManagedAssemblyFptr(OutHandle.m_AssemblyID, filepath.c_str());
		return loadStatus;
	}

	void HostInstance::AddInternalCall(std::string_view InMethodName, void* InFunctionPtr)
	{
		CORAL_VERIFY(InFunctionPtr != nullptr);

		const auto& methodName = m_InternalCallNameStorage.emplace_back(StringHelper::ConvertUtf8ToWide(InMethodName));

		auto* internalCall = new InternalCall();
		internalCall->Name = methodName.c_str();
		internalCall->NativeFunctionPtr = InFunctionPtr;
		m_InternalCalls.emplace_back(std::move(internalCall));
	}

	void HostInstance::UploadInternalCalls()
	{
		UnmanagedArray arr = { m_InternalCalls.data(), (int32_t)m_InternalCalls.size() };
		s_ManagedFunctions.SetInternalCallsFptr(&arr);
	}

	ObjectHandle HostInstance::CreateInstanceInternal(std::string_view InTypeName, const void** InParameters, ManagedType* InParameterTypes, size_t InLength)
	{
		auto typeName = StringHelper::ConvertUtf8ToWide(InTypeName);

		ObjectCreateInfo createInfo =
		{
			.TypeName = typeName.c_str(),
			.IsWeakRef = false,
			.Parameters = InParameters,
			.ParameterTypes = InParameterTypes,
			.Length = int32_t(InLength)
		};

		ObjectHandle handle;
		handle.m_Handle = s_ManagedFunctions.CreateObjectFptr(&createInfo);
		return handle;
	}

	void HostInstance::DestroyInstance(ObjectHandle& InObjectHandle)
	{
		s_ManagedFunctions.DestroyObjectFptr(InObjectHandle.m_Handle);
		InObjectHandle.m_Handle = nullptr;
	}

#ifdef _WIN32
	template <typename TFunc>
	TFunc LoadFunctionPtr(void* InLibraryHandle, const char* InFunctionName)
	{
		auto result = (TFunc)GetProcAddress((HMODULE)InLibraryHandle, InFunctionName);
		CORAL_VERIFY(result);
		return result;
	}
#else
	template <typename TFunc>
	TFunc LoadFunctionPtr(void* InLibraryHandle, const char* InFunctionName)
	{
		auto result = (TFunc)dlsym(InLibraryHandle, InFunctionName);
		CORAL_VERIFY(result);
		return result;
	}
#endif

	void HostInstance::LoadHostFXR() const
	{
		// Retrieve the file path to the CoreCLR library
		size_t pathBufferSize = 0;
		int status = get_hostfxr_path(nullptr, &pathBufferSize, nullptr);
		CORAL_VERIFY(status == StatusCode::HostApiBufferTooSmall);
		std::vector<CharType> pathBuffer;
		pathBuffer.resize(pathBufferSize);
		status = get_hostfxr_path(pathBuffer.data(), &pathBufferSize, nullptr);
		CORAL_VERIFY(status == StatusCode::Success);

		// Load the CoreCLR library
		void* libraryHandle = nullptr;

#ifdef _WIN32
	#ifdef _WCHAR_T_DEFINED
		libraryHandle = LoadLibraryW(pathBuffer.data());
	#else
		libraryHandle = LoadLibraryA(pathBuffer.data());
	#endif
#else
		libraryHandle = dlopen(pathBuffer.data());
#endif

		CORAL_VERIFY(libraryHandle != nullptr);

		// Load CoreCLR functions
		s_CoreCLRFunctions.SetHostFXRErrorWriter = LoadFunctionPtr<hostfxr_set_error_writer_fn>(libraryHandle, "hostfxr_set_error_writer");
		s_CoreCLRFunctions.InitHostFXRForRuntimeConfig = LoadFunctionPtr<hostfxr_initialize_for_runtime_config_fn>(libraryHandle, "hostfxr_initialize_for_runtime_config");
		s_CoreCLRFunctions.GetRuntimeDelegate = LoadFunctionPtr<hostfxr_get_runtime_delegate_fn>(libraryHandle, "hostfxr_get_runtime_delegate");
		s_CoreCLRFunctions.CloseHostFXR = LoadFunctionPtr<hostfxr_close_fn>(libraryHandle, "hostfxr_close");
	}
	
	void HostInstance::InitializeCoralManaged()
	{
		// Fetch load_assembly_and_get_function_pointer_fn from CoreCLR
		{
			auto runtimeConfigPath = std::filesystem::path(m_Settings.CoralDirectory) / "Coral.Managed.runtimeconfig.json";
			int status = s_CoreCLRFunctions.InitHostFXRForRuntimeConfig(runtimeConfigPath.c_str(), nullptr, &m_HostFXRContext);
			CORAL_VERIFY(status == StatusCode::Success && m_HostFXRContext != nullptr);

			status = s_CoreCLRFunctions.GetRuntimeDelegate(m_HostFXRContext, hdt_load_assembly_and_get_function_pointer, (void**)&s_CoreCLRFunctions.GetManagedFunctionPtr);
			CORAL_VERIFY(status == StatusCode::Success);
		}

		using InitializeFn = void(*)();
		InitializeFn coralManagedEntryPoint = nullptr;
		coralManagedEntryPoint = LoadCoralManagedFunctionPtr<InitializeFn>(CORAL_STR("Coral.ManagedHost, Coral.Managed"), CORAL_STR("Initialize"));

		LoadCoralFunctions();

		coralManagedEntryPoint();
	}

	void HostInstance::LoadCoralFunctions()
	{
		s_ManagedFunctions.LoadManagedAssemblyFptr = LoadCoralManagedFunctionPtr<LoadManagedAssemblyFn>(CORAL_STR("Coral.AssemblyLoader, Coral.Managed"), CORAL_STR("LoadAssembly"));
		s_ManagedFunctions.SetInternalCallsFptr = LoadCoralManagedFunctionPtr<SetInternalCallsFn>(CORAL_STR("Coral.Interop.InternalCallsManager, Coral.Managed"), CORAL_STR("SetInternalCalls"));
		s_ManagedFunctions.FreeManagedStringFptr = LoadCoralManagedFunctionPtr<FreeManagedStringFn>(CORAL_STR("Coral.Interop.UnmanagedString, Coral.Managed"), CORAL_STR("Free"));
		s_ManagedFunctions.CreateObjectFptr = LoadCoralManagedFunctionPtr<CreateObjectFn>(CORAL_STR("Coral.ManagedHost, Coral.Managed"), CORAL_STR("CreateObject"));
		s_ManagedFunctions.DestroyObjectFptr = LoadCoralManagedFunctionPtr<DestroyObjectFn>(CORAL_STR("Coral.ManagedHost, Coral.Managed"), CORAL_STR("DestroyObject"));
	}

	void* HostInstance::LoadCoralManagedFunctionPtr(const std::filesystem::path& InAssemblyPath, const CharType* InTypeName, const CharType* InMethodName, const CharType* InDelegateType) const
	{
		void* funcPtr = nullptr;
		int status = s_CoreCLRFunctions.GetManagedFunctionPtr(InAssemblyPath.c_str(), InTypeName, InMethodName, InDelegateType, nullptr, &funcPtr);
		CORAL_VERIFY(status == StatusCode::Success && funcPtr != nullptr);
		return funcPtr;
	}

}
