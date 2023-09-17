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
	ExceptionCallbackFn ExceptionCallback = nullptr;

	struct ObjectCreateInfo
	{
		const CharType* TypeName;
		bool IsWeakRef;
		UnmanagedArray Parameters;
	};

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
		auto name = StringHelper::ConvertUtf8ToWide(InName);
		AssemblyLoadContext alc;
		alc.m_ContextId = s_ManagedFunctions.CreateAssemblyLoadContextFptr(name.c_str());
		alc.m_Host = this;
		return alc;
	}

	void HostInstance::UnloadAssemblyLoadContext(AssemblyLoadContext& InLoadContext)
	{
		s_ManagedFunctions.UnloadAssemblyLoadContextFptr(InLoadContext.m_ContextId);
		InLoadContext.m_ContextId = -1;
		InLoadContext.m_LoadedAssemblies.clear();
	}

	ManagedObject HostInstance::CreateInstanceInternal(std::string_view InTypeName, const void** InParameters, size_t InLength)
	{
		auto typeName = StringHelper::ConvertUtf8ToWide(InTypeName);

		ObjectCreateInfo createInfo =
		{
			.TypeName = typeName.c_str(),
			.IsWeakRef = false,
			.Parameters = { InParameters, int32_t(InLength) },
		};

		auto result = s_ManagedFunctions.CreateObjectFptr(&createInfo);
		result.m_Host = this;
		return result;
	}

	void HostInstance::DestroyInstance(ManagedObject& InObjectHandle)
	{
		if (!InObjectHandle.m_Handle)
			return;
		
		s_ManagedFunctions.DestroyObjectFptr(InObjectHandle.m_Handle);
		InObjectHandle.m_Handle = nullptr;
	}

	void HostInstance::FreeString(const CharType* InString)
	{
		s_ManagedFunctions.FreeManagedStringFptr(InString);
	}

	ReflectionType& HostInstance::GetReflectionType(const CSString& InTypeName)
	{
		size_t id = std::hash<std::string>()(InTypeName.ToString());

		if (!m_ReflectionTypes.contains(id))
		{
			ReflectionType reflectionType;
			s_ManagedFunctions.GetReflectionTypeFptr(InTypeName.Data(), &reflectionType);
			reflectionType.m_Host = this;
			m_ReflectionTypes[id] = std::move(reflectionType);
		}
		
		return m_ReflectionTypes.at(id);
	}
	
	ReflectionType& HostInstance::GetReflectionType(ManagedObject InObject)
	{
		size_t id = std::hash<std::string>()(InObject.m_FullName.ToString());

		if (!m_ReflectionTypes.contains(id))
		{
			ReflectionType reflectionType;
			s_ManagedFunctions.GetReflectionTypeFromObjectFptr(InObject.m_Handle, &reflectionType);
			reflectionType.m_Host = this;
			m_ReflectionTypes[id] = std::move(reflectionType);
		}
		
		return m_ReflectionTypes.at(id);
	}

	const std::vector<ManagedField>& HostInstance::GetFields(const CSString& InTypeName)
	{
		size_t id = std::hash<std::string>()(InTypeName.ToString());

		if (!m_Fields.contains(id))
		{
			int32_t fieldCount;
			s_ManagedFunctions.GetFieldsFptr(InTypeName.Data(), nullptr, &fieldCount);
			m_Fields[id].resize(fieldCount);
			s_ManagedFunctions.GetFieldsFptr(InTypeName.Data(), m_Fields[id].data(), &fieldCount);
		}

		return m_Fields.at(id);
	}

	const std::vector<MethodInfo>& HostInstance::GetMethods(const CSString& InTypeName)
	{
		size_t id = std::hash<const CharType*>()(InTypeName.Data());

		if (!m_Methods.contains(id))
		{
			int32_t methodCount;
			s_ManagedFunctions.GetTypeMethodsFptr(InTypeName.Data(), nullptr, &methodCount);
			m_Methods[id].resize(methodCount);
			s_ManagedFunctions.GetTypeMethodsFptr(InTypeName.Data(), m_Methods[id].data(), &methodCount);
		}

		return m_Methods.at(id);
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

		s_ManagedFunctions.SetExceptionCallbackFptr([](const CharType* InMessage)
		{
			if (!ExceptionCallback)
				return;

			auto message = StringHelper::ConvertWideToUtf8(InMessage);
			ExceptionCallback(message);
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
		s_ManagedFunctions.QueryAssemblyTypesFptr = LoadCoralManagedFunctionPtr<QueryAssemblyTypesFn>(CORAL_STR("Coral.Managed.AssemblyLoader, Coral.Managed"), CORAL_STR("QueryAssemblyTypes"));
		s_ManagedFunctions.GetReflectionTypeFptr = LoadCoralManagedFunctionPtr<GetReflectionTypeFn>(CORAL_STR("Coral.Managed.ManagedHost, Coral.Managed"), CORAL_STR("GetReflectionType"));
		s_ManagedFunctions.GetReflectionTypeFromObjectFptr = LoadCoralManagedFunctionPtr<GetReflectionTypeFromObjectFn>(CORAL_STR("Coral.Managed.ManagedHost, Coral.Managed"), CORAL_STR("GetReflectionTypeFromObject"));
		s_ManagedFunctions.GetFieldsFptr = LoadCoralManagedFunctionPtr<GetFieldsFn>(CORAL_STR("Coral.Managed.ManagedHost, Coral.Managed"), CORAL_STR("QueryObjectFields"));
		s_ManagedFunctions.GetTypeMethodsFptr = LoadCoralManagedFunctionPtr<GetTypeMethodsFn>(CORAL_STR("Coral.Managed.ManagedHost, Coral.Managed"), CORAL_STR("GetTypeMethods"));
		s_ManagedFunctions.GetTypeIdFptr = LoadCoralManagedFunctionPtr<GetTypeIdFn>(CORAL_STR("Coral.Managed.ManagedHost, Coral.Managed"), CORAL_STR("GetTypeId"));
		s_ManagedFunctions.SetInternalCallsFptr = LoadCoralManagedFunctionPtr<SetInternalCallsFn>(CORAL_STR("Coral.Managed.Interop.InternalCallsManager, Coral.Managed"), CORAL_STR("SetInternalCalls"));
		s_ManagedFunctions.FreeManagedStringFptr = LoadCoralManagedFunctionPtr<FreeManagedStringFn>(CORAL_STR("Coral.Managed.Interop.UnmanagedString, Coral.Managed"), CORAL_STR("FreeUnmanaged"));
		s_ManagedFunctions.CreateObjectFptr = LoadCoralManagedFunctionPtr<CreateObjectFn>(CORAL_STR("Coral.Managed.ManagedObject, Coral.Managed"), CORAL_STR("CreateObject"));
		s_ManagedFunctions.InvokeMethodFptr = LoadCoralManagedFunctionPtr<InvokeMethodFn>(CORAL_STR("Coral.Managed.ManagedObject, Coral.Managed"), CORAL_STR("InvokeMethod"));
		s_ManagedFunctions.InvokeMethodRetFptr = LoadCoralManagedFunctionPtr<InvokeMethodRetFn>(CORAL_STR("Coral.Managed.ManagedObject, Coral.Managed"), CORAL_STR("InvokeMethodRet"));
		s_ManagedFunctions.SetFieldValueFptr = LoadCoralManagedFunctionPtr<SetFieldValueFn>(CORAL_STR("Coral.Managed.ManagedObject, Coral.Managed"), CORAL_STR("SetFieldValue"));
		s_ManagedFunctions.GetFieldValueFptr = LoadCoralManagedFunctionPtr<GetFieldValueFn>(CORAL_STR("Coral.Managed.ManagedObject, Coral.Managed"), CORAL_STR("GetFieldValue"));
		s_ManagedFunctions.SetPropertyValueFptr = LoadCoralManagedFunctionPtr<SetFieldValueFn>(CORAL_STR("Coral.Managed.ManagedObject, Coral.Managed"), CORAL_STR("SetPropertyValue"));
		s_ManagedFunctions.GetPropertyValueFptr = LoadCoralManagedFunctionPtr<GetFieldValueFn>(CORAL_STR("Coral.Managed.ManagedObject, Coral.Managed"), CORAL_STR("GetPropertyValue"));
		s_ManagedFunctions.DestroyObjectFptr = LoadCoralManagedFunctionPtr<DestroyObjectFn>(CORAL_STR("Coral.Managed.ManagedObject, Coral.Managed"), CORAL_STR("DestroyObject"));
		s_ManagedFunctions.IsTypeAssignableTo = LoadCoralManagedFunctionPtr<IsAssignableToFn>(CORAL_STR("Coral.Managed.ManagedHost, Coral.Managed"), CORAL_STR("IsTypeAssignableTo"));
		s_ManagedFunctions.IsTypeAssignableFrom = LoadCoralManagedFunctionPtr<IsAssignableFromFn>(CORAL_STR("Coral.Managed.ManagedHost, Coral.Managed"), CORAL_STR("IsTypeAssignableFrom"));
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
