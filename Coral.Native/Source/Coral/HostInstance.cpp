#include "HostInstance.hpp"
#include "Verify.hpp"
#include "HostFXRErrorCodes.hpp"
#include "StringHelper.hpp"
#include "TypeCache.hpp"

#include "NativeCallables.generated.hpp"

#if defined(CORAL_WINDOWS)
	#include <ShlObj_core.h>
#else
	#include <dlfcn.h>
#endif

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

	MessageCallbackFn MessageCallback = nullptr;
	MessageLevel MessageFilter;
	ExceptionCallbackFn ExceptionCallback = nullptr;

	void DefaultMessageCallback(std::string_view InMessage, MessageLevel InLevel)
	{
		const char* level = "";

		switch (InLevel)
		{
		case MessageLevel::Info:
			level = "Info";
			break;
		case MessageLevel::Warning:
			level = "Warn";
			break;
		case MessageLevel::Error:
			level = "Error";
			break;
		}

		std::cout << "[Coral](" << level << "): " << InMessage << std::endl;
	}

	bool HostInstance::Initialize(HostSettings InSettings)
	{
		CORAL_VERIFY(!m_Initialized);

		LoadHostFXR();

		// Setup settings
		m_Settings = std::move(InSettings);

		if (!m_Settings.MessageCallback)
			m_Settings.MessageCallback = DefaultMessageCallback;
		MessageCallback = m_Settings.MessageCallback;
		MessageFilter = m_Settings.MessageFilter;

		s_CoreCLRFunctions.SetHostFXRErrorWriter([](const CharType* InMessage)
		{
			auto message = StringHelper::ConvertWideToUtf8(InMessage);
			MessageCallback(message, MessageLevel::Error);
		});

		m_CoralManagedAssemblyPath = std::filesystem::path(m_Settings.CoralDirectory) / "Coral.Managed.dll";

		if (!std::filesystem::exists(m_CoralManagedAssemblyPath))
		{
			MessageCallback("Failed to find Coral.Managed.dll", MessageLevel::Error);
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
		ScopedString name = String::New(InName);
		AssemblyLoadContext alc;
		alc.m_ContextId = s_NativeCallables.CreateAssemblyLoadContextFptr(name);
		alc.m_Host = this;
		return alc;
	}

	void HostInstance::UnloadAssemblyLoadContext(AssemblyLoadContext& InLoadContext)
	{
		s_NativeCallables.UnloadAssemblyLoadContextFptr(InLoadContext.m_ContextId);
		InLoadContext.m_ContextId = -1;
		InLoadContext.m_LoadedAssemblies.Clear();
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

	std::filesystem::path GetHostFXRPath()
	{
#if defined(CORAL_WINDOWS)
		std::filesystem::path basePath = "";
		
		// Find the Program Files folder
		TCHAR pf[MAX_PATH];
		SHGetSpecialFolderPath(
		nullptr,
		pf,
		CSIDL_PROGRAM_FILES,
		FALSE);

		basePath = pf;
		basePath /= "dotnet/host/fxr/";

		auto searchPaths = std::array
		{
			basePath
		};

#elif defined(CORAL_LINUX)
		auto searchPaths = std::array
		{
			std::filesystem::path("/usr/lib/dotnet/host/fxr/"),
			std::filesystem::path("/usr/share/dotnet/host/fxr/"),
		};
#endif

		for (const auto& path : searchPaths)
		{
			if (!std::filesystem::exists(path))
				continue;

			for (const auto& dir : std::filesystem::recursive_directory_iterator(path))
			{
				if (!dir.is_directory())
					continue;

				auto dirPath = dir.path().string();

				if (dirPath.find(CORAL_DOTNET_TARGET_VERSION_MAJOR_STR) == std::string::npos)
					continue;

				auto res = dir / std::filesystem::path(CORAL_HOSTFXR_NAME);
				CORAL_VERIFY(std::filesystem::exists(res));
				return res;
			}
		}

		return "";
	}

	void HostInstance::LoadHostFXR() const
	{
		// Retrieve the file path to the CoreCLR library
		auto hostfxrPath = GetHostFXRPath();

		// Load the CoreCLR library
		void* libraryHandle = nullptr;

#ifdef CORAL_WINDOWS
	#ifdef CORAL_WIDE_CHARS
		libraryHandle = LoadLibraryW(hostfxrPath.c_str());
	#else
		libraryHandle = LoadLibraryA(hostfxrPath.string().c_str());
	#endif
#else
		libraryHandle = dlopen(hostfxrPath.string().data(), RTLD_NOW | RTLD_GLOBAL);
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
				MessageCallback("Failed to find Coral.Managed.runtimeconfig.json", MessageLevel::Error);
				return false;
			}

			int status = s_CoreCLRFunctions.InitHostFXRForRuntimeConfig(runtimeConfigPath.c_str(), nullptr, &m_HostFXRContext);
			CORAL_VERIFY(status == StatusCode::Success || status == StatusCode::Success_HostAlreadyInitialized || status == StatusCode::Success_DifferentRuntimeProperties);
			CORAL_VERIFY(m_HostFXRContext != nullptr);

			status = s_CoreCLRFunctions.GetRuntimeDelegate(m_HostFXRContext, hdt_load_assembly_and_get_function_pointer, (void**)&s_CoreCLRFunctions.GetManagedFunctionPtr);
			CORAL_VERIFY(status == StatusCode::Success);
		}

		using InitializeFn = void(*)(void(*)(String, MessageLevel), void(*)(String));
		InitializeFn coralManagedEntryPoint = nullptr;
		coralManagedEntryPoint = LoadCoralManagedFunctionPtr<InitializeFn>(CORAL_STR("Coral.Managed.ManagedHost, Coral.Managed"), CORAL_STR("Initialize"));

		LoadCoralFunctions();

		coralManagedEntryPoint([](String InMessage, MessageLevel InLevel)
		{
			if (MessageFilter & InLevel)
			{
				std::string message = InMessage;
				MessageCallback(message, InLevel);
			}
		},
		[](String InMessage)
		{
			std::string message = InMessage;
			if (!ExceptionCallback)
			{
				MessageCallback(message, MessageLevel::Error);
				return;
			}
			
			ExceptionCallback(message);
		});

		ExceptionCallback = m_Settings.ExceptionCallback;

		return true;
	}

	void HostInstance::LoadCoralFunctions()
	{
		s_NativeCallables = LoadCoralManagedFunctionPtr<NativeCallables(*)()>(CORAL_STR("Coral.Managed.NativeCallables, Coral.Managed"), CORAL_STR("Get"))();
	}

	void* HostInstance::LoadCoralManagedFunctionPtr(const std::filesystem::path& InAssemblyPath, const CharType* InTypeName, const CharType* InMethodName, const CharType* InDelegateType) const
	{
		void* funcPtr = nullptr;
		int status = s_CoreCLRFunctions.GetManagedFunctionPtr(InAssemblyPath.c_str(), InTypeName, InMethodName, InDelegateType, nullptr, &funcPtr);
		CORAL_VERIFY(status == StatusCode::Success && funcPtr != nullptr);
		return funcPtr;
	}
}
