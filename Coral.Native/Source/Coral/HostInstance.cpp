#include "HostInstance.hpp"
#include "Verify.hpp"
#include "HostFXRErrorCodes.hpp"
#include "Interop.hpp"

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

	using SetInternalCallsFn = void (*)(UnmanagedArray*);
	SetInternalCallsFn SetInternalCalls = nullptr;

	using LoadManagedAssemblyFn = AssemblyLoadStatus(*)(uint16_t, const CharType*);
	LoadManagedAssemblyFn LoadManagedAssembly = nullptr;

	using GetStringFn = const CharType*(*)();
	GetStringFn GetString = nullptr;

	using FreeManagedStringFn = void(*)(const CharType*);
	FreeManagedStringFn FreeManagedString = nullptr;

	struct ObjectCreateInfo
	{
		const CharType* TypeName;
		bool IsWeakRef;
	};
	using CreateObjectFn = void*(*)(const ObjectCreateInfo*);
	CreateObjectFn CreateObject = nullptr;

	using DestroyObjectFn = void(*)(void*);
	DestroyObjectFn DestroyObject = nullptr;

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

	AssemblyLoadStatus HostInstance::LoadAssembly(const CharType* InFilePath, AssemblyHandle& OutHandle)
	{
		static uint16_t s_NextAssemblyID = 0;

		OutHandle.m_AssemblyID = s_NextAssemblyID++;
		AssemblyLoadStatus loadStatus = LoadManagedAssembly(OutHandle.m_AssemblyID, InFilePath);

		/*if (loadStatus == AssemblyLoadStatus::Success)
		{
			auto& assemblyData = m_LoadedAssemblies[OutHandle.m_AssemblyID];
		}*/

		return loadStatus;
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

	ObjectHandle HostInstance::CreateInstance(const CharType* InTypeName)
	{
		ObjectCreateInfo createInfo =
		{
			.TypeName = InTypeName,
			.IsWeakRef = false
		};

		ObjectHandle handle;
		handle.m_Handle = CreateObject(&createInfo);
		return handle;
	}

	void HostInstance::DestroyInstance(ObjectHandle& InObjectHandle)
	{
		DestroyObject(InObjectHandle.m_Handle);
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

	void DummyFunc()
	{
		std::cout << "Hellllooo" << std::endl;
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
		LoadManagedAssembly = LoadCoralManagedFunctionPtr<LoadManagedAssemblyFn>(CORAL_STR("Coral.AssemblyLoader, Coral.Managed"), CORAL_STR("LoadAssembly"));
		SetInternalCalls = LoadCoralManagedFunctionPtr<SetInternalCallsFn>(CORAL_STR("Coral.ManagedHost, Coral.Managed"), CORAL_STR("SetInternalCalls"));
		GetString = LoadCoralManagedFunctionPtr<GetStringFn>(CORAL_STR("Coral.ManagedHost, Coral.Managed"), CORAL_STR("GetString"));
		FreeManagedString = LoadCoralManagedFunctionPtr<FreeManagedStringFn>(CORAL_STR("Coral.Interop.UnmanagedString, Coral.Managed"), CORAL_STR("Free"));
		CreateObject = LoadCoralManagedFunctionPtr<CreateObjectFn>(CORAL_STR("Coral.ManagedHost, Coral.Managed"), CORAL_STR("CreateObject"));
		DestroyObject = LoadCoralManagedFunctionPtr<DestroyObjectFn>(CORAL_STR("Coral.ManagedHost, Coral.Managed"), CORAL_STR("DestroyObject"));

		auto msg = GetString();
		std::wcout << L"Message: " << msg << std::endl;
		FreeManagedString(msg);

		coralManagedEntryPoint();
	}

	void* HostInstance::LoadCoralManagedFunctionPtr(const std::filesystem::path& InAssemblyPath, const CharType* InTypeName, const CharType* InMethodName, const CharType* InDelegateType) const
	{
		void* funcPtr = nullptr;
		int status = s_CoreCLRFunctions.GetManagedFunctionPtr(InAssemblyPath.c_str(), InTypeName, InMethodName, InDelegateType, nullptr, &funcPtr);
		CORAL_VERIFY(status == StatusCode::Success && funcPtr != nullptr);
		return funcPtr;
	}

}
