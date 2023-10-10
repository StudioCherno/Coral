#include "HostInstance.hpp"
#include "Verify.hpp"
#include "HostFXRErrorCodes.hpp"
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
	ExceptionCallbackFn ExceptionCallback = nullptr;

	void DefaultErrorCallback(std::string_view InMessage)
	{
		std::cout << "[Coral.Native]: " << InMessage << std::endl;
	}

	bool HostInstance::Initialize(HostSettings InSettings)
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
			auto message = StringHelper::ConvertWideToUtf8(InMessage);
			ErrorCallback(message);
		});

		m_CoralManagedAssemblyPath = std::filesystem::path(m_Settings.CoralDirectory) / "Coral.Managed.dll";

		if (!std::filesystem::exists(m_CoralManagedAssemblyPath))
		{
			ErrorCallback("Failed to find Coral.Managed.dll");
			return false;
		}

		m_Initialized = InitializeCoralManaged();

		return m_Initialized;
	}

	void HostInstance::Shutdown()
	{
		s_CoreCLRFunctions.CloseHostFXR(m_HostFXRContext);
	}
	
	AssemblyLoadContext HostInstance::CreateAssemblyLoadContext(std::string_view InName)
	{
		auto name = Coral::NativeString::FromUTF8(InName);
		AssemblyLoadContext alc;
		alc.m_ContextId = s_ManagedFunctions.CreateAssemblyLoadContextFptr(name);
		alc.m_Host = this;
		return alc;
	}

	void HostInstance::UnloadAssemblyLoadContext(AssemblyLoadContext& InLoadContext)
	{
		s_ManagedFunctions.UnloadAssemblyLoadContextFptr(InLoadContext.m_ContextId);
		InLoadContext.m_ContextId = -1;
		InLoadContext.m_LoadedAssemblies.clear();
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
	
	bool HostInstance::InitializeCoralManaged()
	{
		// Fetch load_assembly_and_get_function_pointer_fn from CoreCLR
		{
			auto runtimeConfigPath = std::filesystem::path(m_Settings.CoralDirectory) / "Coral.Managed.runtimeconfig.json";

			if (!std::filesystem::exists(runtimeConfigPath))
			{
				ErrorCallback("Failed to find Coral.Managed.runtimeconfig.json");
				return false;
			}

			int status = s_CoreCLRFunctions.InitHostFXRForRuntimeConfig(runtimeConfigPath.c_str(), nullptr, &m_HostFXRContext);
			CORAL_VERIFY(status == StatusCode::Success || status == StatusCode::Success_HostAlreadyInitialized || status == StatusCode::Success_DifferentRuntimeProperties);
			CORAL_VERIFY(m_HostFXRContext != nullptr);

			status = s_CoreCLRFunctions.GetRuntimeDelegate(m_HostFXRContext, hdt_load_assembly_and_get_function_pointer, (void**)&s_CoreCLRFunctions.GetManagedFunctionPtr);
			CORAL_VERIFY(status == StatusCode::Success);
		}

		using InitializeFn = void(*)();
		InitializeFn coralManagedEntryPoint = nullptr;
		coralManagedEntryPoint = LoadCoralManagedFunctionPtr<InitializeFn>(CORAL_STR("Coral.Managed.ManagedHost, Coral.Managed"), CORAL_STR("Initialize"));

		LoadCoralFunctions();

		coralManagedEntryPoint();

		ExceptionCallback = m_Settings.ExceptionCallback;

		s_ManagedFunctions.SetExceptionCallbackFptr([](NativeString InMessage)
		{
			if (!ExceptionCallback)
				return;

			ExceptionCallback(InMessage.ToString());
		});

		return true;
	}

	void HostInstance::LoadCoralFunctions()
	{
		s_ManagedFunctions.CreateAssemblyLoadContextFptr = LoadCoralManagedFunctionPtr<CreateAssemblyLoadContextFn>(CORAL_STR("Coral.Managed.AssemblyLoader, Coral.Managed"), CORAL_STR("CreateAssemblyLoadContext"));
		s_ManagedFunctions.UnloadAssemblyLoadContextFptr = LoadCoralManagedFunctionPtr<UnloadAssemblyLoadContextFn>(CORAL_STR("Coral.Managed.AssemblyLoader, Coral.Managed"), CORAL_STR("UnloadAssemblyLoadContext"));
		s_ManagedFunctions.LoadManagedAssemblyFptr = LoadCoralManagedFunctionPtr<LoadManagedAssemblyFn>(CORAL_STR("Coral.Managed.AssemblyLoader, Coral.Managed"), CORAL_STR("LoadAssembly"));
		s_ManagedFunctions.UnloadAssemblyLoadContextFptr = LoadCoralManagedFunctionPtr<UnloadAssemblyLoadContextFn>(CORAL_STR("Coral.Managed.AssemblyLoader, Coral.Managed"), CORAL_STR("UnloadAssemblyLoadContext"));
		s_ManagedFunctions.GetLastLoadStatusFptr = LoadCoralManagedFunctionPtr<GetLastLoadStatusFn>(CORAL_STR("Coral.Managed.AssemblyLoader, Coral.Managed"), CORAL_STR("GetLastLoadStatus"));
		s_ManagedFunctions.GetAssemblyNameFptr = LoadCoralManagedFunctionPtr<GetAssemblyNameFn>(CORAL_STR("Coral.Managed.AssemblyLoader, Coral.Managed"), CORAL_STR("GetAssemblyName"));

		s_ManagedFunctions.GetAssemblyTypes = LoadCoralManagedFunctionPtr<GetAssemblyTypesFn>(CORAL_STR("Coral.Managed.TypeInterface, Coral.Managed"), CORAL_STR("GetAssemblyTypes"));
		s_ManagedFunctions.GetTypeIdFptr = LoadCoralManagedFunctionPtr<GetTypeIdFn>(CORAL_STR("Coral.Managed.TypeInterface, Coral.Managed"), CORAL_STR("GetTypeId"));
		s_ManagedFunctions.GetFullTypeNameFptr = LoadCoralManagedFunctionPtr<GetFullTypeNameFn>(CORAL_STR("Coral.Managed.TypeInterface, Coral.Managed"), CORAL_STR("GetFullTypeName"));
		s_ManagedFunctions.GetAssemblyQualifiedNameFptr = LoadCoralManagedFunctionPtr<GetAssemblyQualifiedNameFn>(CORAL_STR("Coral.Managed.TypeInterface, Coral.Managed"), CORAL_STR("GetAssemblyQualifiedName"));
		s_ManagedFunctions.GetBaseTypeFptr = LoadCoralManagedFunctionPtr<GetBaseTypeFn>(CORAL_STR("Coral.Managed.TypeInterface, Coral.Managed"), CORAL_STR("GetBaseType"));
		s_ManagedFunctions.IsTypeAssignableToFptr = LoadCoralManagedFunctionPtr<IsTypeAssignableToFn>(CORAL_STR("Coral.Managed.TypeInterface, Coral.Managed"), CORAL_STR("IsTypeAssignableTo"));
		s_ManagedFunctions.IsTypeAssignableFromFptr = LoadCoralManagedFunctionPtr<IsTypeAssignableFromFn>(CORAL_STR("Coral.Managed.TypeInterface, Coral.Managed"), CORAL_STR("IsTypeAssignableFrom"));
		s_ManagedFunctions.GetTypeMethodsFptr = LoadCoralManagedFunctionPtr<GetTypeMethodsFn>(CORAL_STR("Coral.Managed.TypeInterface, Coral.Managed"), CORAL_STR("GetTypeMethods"));
		s_ManagedFunctions.GetTypeFieldsFptr = LoadCoralManagedFunctionPtr<GetTypeFieldsFn>(CORAL_STR("Coral.Managed.TypeInterface, Coral.Managed"), CORAL_STR("GetTypeFields"));
		s_ManagedFunctions.GetTypePropertiesFptr = LoadCoralManagedFunctionPtr<GetTypePropertiesFn>(CORAL_STR("Coral.Managed.TypeInterface, Coral.Managed"), CORAL_STR("GetTypeProperties"));
		s_ManagedFunctions.GetTypeAttributesFptr = LoadCoralManagedFunctionPtr<GetTypeAttributesFn>(CORAL_STR("Coral.Managed.TypeInterface, Coral.Managed"), CORAL_STR("GetTypeAttributes"));
		
		s_ManagedFunctions.GetMethodInfoNameFptr = LoadCoralManagedFunctionPtr<GetMethodInfoNameFn>(CORAL_STR("Coral.Managed.TypeInterface, Coral.Managed"), CORAL_STR("GetMethodInfoName"));
		s_ManagedFunctions.GetMethodInfoReturnTypeFptr = LoadCoralManagedFunctionPtr<GetMethodInfoReturnTypeFn>(CORAL_STR("Coral.Managed.TypeInterface, Coral.Managed"), CORAL_STR("GetMethodInfoReturnType"));
		s_ManagedFunctions.GetMethodInfoParameterTypesFptr = LoadCoralManagedFunctionPtr<GetMethodInfoParameterTypesFn>(CORAL_STR("Coral.Managed.TypeInterface, Coral.Managed"), CORAL_STR("GetMethodInfoParameterTypes"));
		s_ManagedFunctions.GetMethodInfoAccessibilityFptr = LoadCoralManagedFunctionPtr<GetMethodInfoAccessibilityFn>(CORAL_STR("Coral.Managed.TypeInterface, Coral.Managed"), CORAL_STR("GetMethodInfoAccessibility"));
		s_ManagedFunctions.GetMethodInfoAttributesFptr = LoadCoralManagedFunctionPtr<GetMethodInfoAttributesFn>(CORAL_STR("Coral.Managed.TypeInterface, Coral.Managed"), CORAL_STR("GetMethodInfoAttributes"));

		s_ManagedFunctions.GetFieldInfoNameFptr = LoadCoralManagedFunctionPtr<GetFieldInfoNameFn>(CORAL_STR("Coral.Managed.TypeInterface, Coral.Managed"), CORAL_STR("GetFieldInfoName"));
		s_ManagedFunctions.GetFieldInfoTypeFptr = LoadCoralManagedFunctionPtr<GetFieldInfoTypeFn>(CORAL_STR("Coral.Managed.TypeInterface, Coral.Managed"), CORAL_STR("GetFieldInfoType"));
		s_ManagedFunctions.GetFieldInfoAccessibilityFptr = LoadCoralManagedFunctionPtr<GetFieldInfoAccessibilityFn>(CORAL_STR("Coral.Managed.TypeInterface, Coral.Managed"), CORAL_STR("GetFieldInfoAccessibility"));
		s_ManagedFunctions.GetFieldInfoAttributesFptr = LoadCoralManagedFunctionPtr<GetFieldInfoAttributesFn>(CORAL_STR("Coral.Managed.TypeInterface, Coral.Managed"), CORAL_STR("GetFieldInfoAttributes"));

		s_ManagedFunctions.GetPropertyInfoNameFptr = LoadCoralManagedFunctionPtr<GetPropertyInfoNameFn>(CORAL_STR("Coral.Managed.TypeInterface, Coral.Managed"), CORAL_STR("GetPropertyInfoName"));
		s_ManagedFunctions.GetPropertyInfoTypeFptr = LoadCoralManagedFunctionPtr<GetPropertyInfoTypeFn>(CORAL_STR("Coral.Managed.TypeInterface, Coral.Managed"), CORAL_STR("GetPropertyInfoType"));
		s_ManagedFunctions.GetPropertyInfoAttributesFptr = LoadCoralManagedFunctionPtr<GetPropertyInfoAttributesFn>(CORAL_STR("Coral.Managed.TypeInterface, Coral.Managed"), CORAL_STR("GetPropertyInfoAttributes"));

		s_ManagedFunctions.GetAttributeFieldValueFptr = LoadCoralManagedFunctionPtr<GetAttributeFieldValueFn>(CORAL_STR("Coral.Managed.TypeInterface, Coral.Managed"), CORAL_STR("GetAttributeFieldValue"));
		s_ManagedFunctions.GetAttributeTypeFptr = LoadCoralManagedFunctionPtr<GetAttributeTypeFn>(CORAL_STR("Coral.Managed.TypeInterface, Coral.Managed"), CORAL_STR("GetAttributeType"));

		s_ManagedFunctions.SetInternalCallsFptr = LoadCoralManagedFunctionPtr<SetInternalCallsFn>(CORAL_STR("Coral.Managed.Interop.InternalCallsManager, Coral.Managed"), CORAL_STR("SetInternalCalls"));
		s_ManagedFunctions.CreateObjectFptr = LoadCoralManagedFunctionPtr<CreateObjectFn>(CORAL_STR("Coral.Managed.ManagedObject, Coral.Managed"), CORAL_STR("CreateObject"));
		s_ManagedFunctions.InvokeMethodFptr = LoadCoralManagedFunctionPtr<InvokeMethodFn>(CORAL_STR("Coral.Managed.ManagedObject, Coral.Managed"), CORAL_STR("InvokeMethod"));
		s_ManagedFunctions.InvokeMethodRetFptr = LoadCoralManagedFunctionPtr<InvokeMethodRetFn>(CORAL_STR("Coral.Managed.ManagedObject, Coral.Managed"), CORAL_STR("InvokeMethodRet"));
		s_ManagedFunctions.SetFieldValueFptr = LoadCoralManagedFunctionPtr<SetFieldValueFn>(CORAL_STR("Coral.Managed.ManagedObject, Coral.Managed"), CORAL_STR("SetFieldValue"));
		s_ManagedFunctions.GetFieldValueFptr = LoadCoralManagedFunctionPtr<GetFieldValueFn>(CORAL_STR("Coral.Managed.ManagedObject, Coral.Managed"), CORAL_STR("GetFieldValue"));
		s_ManagedFunctions.SetPropertyValueFptr = LoadCoralManagedFunctionPtr<SetFieldValueFn>(CORAL_STR("Coral.Managed.ManagedObject, Coral.Managed"), CORAL_STR("SetPropertyValue"));
		s_ManagedFunctions.GetPropertyValueFptr = LoadCoralManagedFunctionPtr<GetFieldValueFn>(CORAL_STR("Coral.Managed.ManagedObject, Coral.Managed"), CORAL_STR("GetPropertyValue"));
		s_ManagedFunctions.DestroyObjectFptr = LoadCoralManagedFunctionPtr<DestroyObjectFn>(CORAL_STR("Coral.Managed.ManagedObject, Coral.Managed"), CORAL_STR("DestroyObject"));
		s_ManagedFunctions.SetExceptionCallbackFptr = LoadCoralManagedFunctionPtr<SetExceptionCallbackFn>(CORAL_STR("Coral.Managed.ManagedHost, Coral.Managed"), CORAL_STR("SetExceptionCallback"));
		s_ManagedFunctions.CollectGarbageFptr = LoadCoralManagedFunctionPtr<CollectGarbageFn>(CORAL_STR("Coral.Managed.GarbageCollector, Coral.Managed"), CORAL_STR("CollectGarbage"));
		s_ManagedFunctions.WaitForPendingFinalizersFptr = LoadCoralManagedFunctionPtr<WaitForPendingFinalizersFn>(CORAL_STR("Coral.Managed.GarbageCollector, Coral.Managed"), CORAL_STR("WaitForPendingFinalizers"));
	}

	void* HostInstance::LoadCoralManagedFunctionPtr(const std::filesystem::path& InAssemblyPath, const CharType* InTypeName, const CharType* InMethodName, const CharType* InDelegateType) const
	{
		void* funcPtr = nullptr;
		int status = s_CoreCLRFunctions.GetManagedFunctionPtr(InAssemblyPath.c_str(), InTypeName, InMethodName, InDelegateType, nullptr, &funcPtr);
		CORAL_VERIFY(status == StatusCode::Success && funcPtr != nullptr);
		return funcPtr;
	}
}
