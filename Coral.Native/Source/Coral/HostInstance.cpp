#include "HostInstance.hpp"
#include "Verify.hpp"
#include "HostFXRErrorCodes.hpp"

namespace Coral {

	hostfxr_set_error_writer_fn SetHostFXRErrorWriter = nullptr;
	hostfxr_initialize_for_runtime_config_fn InitHostFXRForRuntimeConfig = nullptr;
	hostfxr_get_runtime_delegate_fn GetRuntimeDelegate = nullptr;
	hostfxr_close_fn CloseHostFXR = nullptr;

	load_assembly_and_get_function_pointer_fn LoadAssemblyFnptr = nullptr;

	ErrorCallbackFn ErrorCallback = nullptr;

	struct UnmanagedArray
	{
		void* Ptr;
		int32_t Length;
	};

	using SetInternalCallsFn = void (*)(UnmanagedArray*);
	SetInternalCallsFn SetInternalCalls = nullptr;

	enum class EAssemblyLoadStatus
	{
		Success,
		FileNotFound,
		FileLoadFailure,
		InvalidFilePath,
		InvalidAssembly,
		UnknownError
	};

	using LoadManagedAssemblyFn = EAssemblyLoadStatus(*)(const CharType*);
	LoadManagedAssemblyFn LoadManagedAssembly = nullptr;

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
		if (m_Initialized)
		{
			// TODO: Error here
			return;
		}

		LoadHostFXR();

		// Setup settings
		m_Settings = std::move(InSettings);

		if (!m_Settings.ErrorCallback)
			m_Settings.ErrorCallback = DefaultErrorCallback;
		ErrorCallback = m_Settings.ErrorCallback;

		SetHostFXRErrorWriter([](const CharType* InMessage)
		{
			ErrorCallback(InMessage);
		});

		m_CoralManagedAssemblyPath = std::filesystem::path(m_Settings.CoralDirectory) / "Coral.Managed.dll";

		InitializeCoralManaged();

		m_Initialized = true;
	}

	void HostInstance::LoadAssembly(const CharType* InFilePath)
	{
		auto status = LoadManagedAssembly(InFilePath);
		CORAL_VERIFY(status == EAssemblyLoadStatus::Success);
	}

	void HostInstance::AddInternalCall(const CharType* InMethodName, void* InFunctionPtr)
	{
		CORAL_VERIFY(InFunctionPtr != nullptr);

		auto* internalCall = new InternalCall();
		internalCall->Name = InMethodName;
		internalCall->NativeFunctionPtr = InFunctionPtr;
		m_InternalCalls.emplace_back(std::move(internalCall));
	}

	void HostInstance::UploadInternalCalls()
	{
		UnmanagedArray arr = { m_InternalCalls.data(), (int32_t)m_InternalCalls.size() };
		SetInternalCalls(&arr);
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
		SetHostFXRErrorWriter = LoadFunctionPtr<hostfxr_set_error_writer_fn>(libraryHandle, "hostfxr_set_error_writer");
		InitHostFXRForRuntimeConfig = LoadFunctionPtr<hostfxr_initialize_for_runtime_config_fn>(libraryHandle, "hostfxr_initialize_for_runtime_config");
		GetRuntimeDelegate = LoadFunctionPtr<hostfxr_get_runtime_delegate_fn>(libraryHandle, "hostfxr_get_runtime_delegate");
		CloseHostFXR = LoadFunctionPtr<hostfxr_close_fn>(libraryHandle, "hostfxr_close");
	}

	void DummyFunc()
	{
		std::cout << "Hellllooo" << std::endl;
	}

	void HostInstance::InitializeCoralManaged()
	{
		// Fetch load_assembly_and_get_function_pointer_fn from CoreCLR
		{
			auto runtimeConfigPath = std::filesystem::path(m_Settings.CoralDirectory) / "Coral.Managed.runtimeconfig.json";
			int status = InitHostFXRForRuntimeConfig(runtimeConfigPath.c_str(), nullptr, &m_HostFXRContext);
			CORAL_VERIFY(status == StatusCode::Success && m_HostFXRContext != nullptr);

			status = GetRuntimeDelegate(m_HostFXRContext, hdt_load_assembly_and_get_function_pointer, (void**)&LoadAssemblyFnptr);
			CORAL_VERIFY(status == StatusCode::Success && LoadAssemblyFnptr != nullptr);
		}

		using InitializeFn = int(*)(void*);
		InitializeFn coralManagedEntryPoint = nullptr;
		coralManagedEntryPoint = LoadCoralManagedFunctionPtr<InitializeFn>(CORAL_STR("Coral.ManagedHost, Coral.Managed"), CORAL_STR("Initialize"));
		LoadManagedAssembly = LoadCoralManagedFunctionPtr<LoadManagedAssemblyFn>(CORAL_STR("Coral.AssemblyLoader, Coral.Managed"), CORAL_STR("LoadAssembly"));

		struct DummyData
		{
			float x = 10.0f;
			const CharType* Str = CORAL_STR("Hello from native code!");
		} dummyData;

		coralManagedEntryPoint(&dummyData);

		SetInternalCalls = LoadCoralManagedFunctionPtr<SetInternalCallsFn>(CORAL_STR("Coral.ManagedHost, Coral.Managed"), CORAL_STR("SetInternalCalls"));
	}

	void* HostInstance::LoadCoralManagedFunctionPtr(const std::filesystem::path& InAssemblyPath, const CharType* InTypeName, const CharType* InMethodName, const CharType* InDelegateType) const
	{
		void* funcPtr = nullptr;
		int status = LoadAssemblyFnptr(InAssemblyPath.c_str(), InTypeName, InMethodName, InDelegateType, nullptr, &funcPtr);
		CORAL_VERIFY(status == StatusCode::Success && funcPtr != nullptr);
		return funcPtr;
	}

}
