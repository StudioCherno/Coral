#include "HostInstance.hpp"
#include "Verify.hpp"
#include "HostFXRErrorCodes.hpp"

namespace Coral {

	hostfxr_set_error_writer_fn SetHostFXRErrorWriter = nullptr;
	hostfxr_initialize_for_runtime_config_fn InitHostFXRForRuntimeConfig = nullptr;
	hostfxr_get_runtime_delegate_fn GetRuntimeDelegate = nullptr;
	hostfxr_close_fn CloseHostFXR = nullptr;

	load_assembly_and_get_function_pointer_fn LoadAssembly = nullptr;

	ErrorCallbackFn ErrorCallback = nullptr;

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

	void HostInstance::AddInternalCall(const CharType* InMethodName, void* InFunctionPtr)
	{
		CORAL_VERIFY(InFunctionPtr != nullptr);

		auto& icallData = m_InternalCalls.emplace_back();
		icallData.MethodName = InMethodName;
		icallData.NativeFuncPtr = InFunctionPtr;
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

			status = GetRuntimeDelegate(m_HostFXRContext, hdt_load_assembly_and_get_function_pointer, (void**)&LoadAssembly);
			CORAL_VERIFY(status == StatusCode::Success && LoadAssembly != nullptr);
		}

		using InitializeFn = int(*)(void*);
		InitializeFn coralManagedEntryPoint = nullptr;
		coralManagedEntryPoint = LoadCoralManagedFunctionPtr<InitializeFn>(CORAL_STR("Coral.ManagedHost, Coral.Managed"), CORAL_STR("Initialize"));

		struct DummyData
		{
			float x = 10.0f;
			const CharType* Str = CORAL_STR("Hello from native code!");
		} dummyData;

		coralManagedEntryPoint(&dummyData);

		struct InternalCall
		{
			const CharType* Name;
			void* NativeFunctionPtr;
		};

		struct InternalCallsList
		{
			InternalCall** InternalCalls = nullptr;
			int32_t NumInternalCalls = 0;
		};

		InternalCall* call = new InternalCall { CORAL_STR("Test"), &DummyFunc };
		InternalCall* call2 = new InternalCall { CORAL_STR("Test2"), &DummyFunc };
		InternalCallsList list;
		list.InternalCalls = new InternalCall*[] { call, call2 };
		list.NumInternalCalls = 2;

		using SetInternalCallsFn = void(*)(InternalCallsList*);
		SetInternalCallsFn setInternalCalls = nullptr;
		setInternalCalls = LoadCoralManagedFunctionPtr<SetInternalCallsFn>(CORAL_STR("Coral.ManagedHost, Coral.Managed"), CORAL_STR("SetInternalCalls"));
		setInternalCalls(&list);
	}

	void* HostInstance::LoadCoralManagedFunctionPtr(const std::filesystem::path& InAssemblyPath, const CharType* InTypeName, const CharType* InMethodName) const
	{
		void* funcPtr = nullptr;
		int status = LoadAssembly(InAssemblyPath.c_str(), InTypeName, InMethodName, UNMANAGEDCALLERSONLY_METHOD, nullptr, &funcPtr);
		CORAL_VERIFY(status == StatusCode::Success && funcPtr != nullptr);
		return funcPtr;
	}

}
