/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Main.cpp
* @author JXMaster
* @date 2026/4/21
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#include <Luna/Window/AppMain.hpp>
#include <coreclr_delegates.h>
#include <hostfxr.h>
#include <nethost.h>

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#ifdef LUNA_PLATFORM_WINDOWS
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

#ifdef LUNA_PLATFORM_MACOS
#include <mach-o/dyld.h>
#endif

#ifdef LUNA_PLATFORM_LINUX
#include <limits.h>
#include <unistd.h>
#endif

namespace
{
#ifdef LUNA_PLATFORM_WINDOWS
    constexpr const char_t* c_managed_entry_type = L"Luna.Samples.ManagedHostApp.ManagedEntry, ManagedHostApp";
    constexpr const char_t* c_managed_entry_method = L"Run";
#else
    constexpr const char_t* c_managed_entry_type = "Luna.Samples.ManagedHostApp.ManagedEntry, ManagedHostApp";
    constexpr const char_t* c_managed_entry_method = "Run";
#endif

    struct ManagedHostArguments
    {
        int32_t count;
        const char** values;
    };

    struct HostOptions
    {
        std::filesystem::path runtime_config_path;
        std::filesystem::path assembly_path;
        std::vector<std::string> managed_args;
    };

#ifdef LUNA_PLATFORM_WINDOWS
    std::filesystem::path get_executable_directory()
    {
        wchar_t buffer[MAX_PATH] = {};
        const auto length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
        return std::filesystem::path(std::wstring(buffer, length)).parent_path();
    }
#elif defined(LUNA_PLATFORM_MACOS)
    std::filesystem::path get_executable_directory()
    {
        uint32_t size = 0;
        _NSGetExecutablePath(nullptr, &size);
        std::string buffer(size, '\0');
        if(_NSGetExecutablePath(buffer.data(), &size) != 0)
        {
            return std::filesystem::current_path();
        }
        return std::filesystem::weakly_canonical(std::filesystem::path(buffer.c_str())).parent_path();
    }
#elif defined(LUNA_PLATFORM_LINUX)
    std::filesystem::path get_executable_directory()
    {
        char buffer[PATH_MAX] = {};
        const auto length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if(length <= 0)
        {
            return std::filesystem::current_path();
        }
        return std::filesystem::path(std::string(buffer, static_cast<size_t>(length))).parent_path();
    }
#else
    std::filesystem::path get_executable_directory()
    {
        return std::filesystem::current_path();
    }
#endif

    std::basic_string<char_t> to_host_path(const std::filesystem::path& path)
    {
#ifdef LUNA_PLATFORM_WINDOWS
        return path.wstring();
#else
        return path.string();
#endif
    }

    HostOptions parse_options(int argc, const char* argv[])
    {
        HostOptions options;
        const auto executable_directory = get_executable_directory();
        options.runtime_config_path = executable_directory / "ManagedHostApp.runtimeconfig.json";
        options.assembly_path = executable_directory / "ManagedHostApp.dll";

        for(int i = 1; i < argc; ++i)
        {
            const std::string arg = argv[i];
            if(arg == "--runtimeconfig" && i + 1 < argc)
            {
                options.runtime_config_path = argv[++i];
            }
            else if(arg == "--assembly" && i + 1 < argc)
            {
                options.assembly_path = argv[++i];
            }
            else
            {
                options.managed_args.push_back(arg);
            }
        }
        return options;
    }

    void* load_library(const std::filesystem::path& path)
    {
#ifdef LUNA_PLATFORM_WINDOWS
        return LoadLibraryW(path.wstring().c_str());
#else
        return dlopen(path.string().c_str(), RTLD_LAZY | RTLD_LOCAL);
#endif
    }

    void* get_export(void* library, const char* name)
    {
#ifdef LUNA_PLATFORM_WINDOWS
        return reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(library), name));
#else
        return dlsym(library, name);
#endif
    }

    template <typename T>
    bool load_hostfxr_export(void* hostfxr, const char* name, T& out_func)
    {
        auto symbol = get_export(hostfxr, name);
        if(!symbol)
        {
            std::cerr << "Failed to load hostfxr export: " << name << std::endl;
            return false;
        }
        out_func = reinterpret_cast<T>(symbol);
        return true;
    }

    bool load_hostfxr(
        hostfxr_initialize_for_runtime_config_fn& out_init,
        hostfxr_get_runtime_delegate_fn& out_get_delegate,
        hostfxr_close_fn& out_close)
    {
        char_t hostfxr_path[4096] = {};
        size_t hostfxr_path_size = sizeof(hostfxr_path) / sizeof(char_t);
        const auto rc = get_hostfxr_path(hostfxr_path, &hostfxr_path_size, nullptr);
        if(rc != 0)
        {
            std::cerr << "get_hostfxr_path failed: 0x" << std::hex << rc << std::dec << std::endl;
            return false;
        }

        auto hostfxr = load_library(std::filesystem::path(hostfxr_path));
        if(!hostfxr)
        {
            std::cerr << "Failed to load hostfxr." << std::endl;
            return false;
        }

        return load_hostfxr_export(hostfxr, "hostfxr_initialize_for_runtime_config", out_init) &&
            load_hostfxr_export(hostfxr, "hostfxr_get_runtime_delegate", out_get_delegate) &&
            load_hostfxr_export(hostfxr, "hostfxr_close", out_close);
    }

    int run_managed_app(const HostOptions& options)
    {
        const auto runtime_config_path = std::filesystem::absolute(options.runtime_config_path);
        const auto assembly_path = std::filesystem::absolute(options.assembly_path);

        if(!std::filesystem::exists(runtime_config_path))
        {
            std::cerr << "Managed runtime config not found: " << runtime_config_path << std::endl;
            return -1;
        }
        if(!std::filesystem::exists(assembly_path))
        {
            std::cerr << "Managed assembly not found: " << assembly_path << std::endl;
            return -1;
        }

        hostfxr_initialize_for_runtime_config_fn init = nullptr;
        hostfxr_get_runtime_delegate_fn get_delegate = nullptr;
        hostfxr_close_fn close = nullptr;
        if(!load_hostfxr(init, get_delegate, close))
        {
            return -1;
        }

        hostfxr_handle host_context = nullptr;
        auto runtime_config_host_path = to_host_path(runtime_config_path);
        auto rc = init(runtime_config_host_path.c_str(), nullptr, &host_context);
        if(rc != 0 || !host_context)
        {
            std::cerr << "hostfxr_initialize_for_runtime_config failed: 0x" << std::hex << rc << std::dec << std::endl;
            return -1;
        }

        void* load_assembly_and_get_function_pointer = nullptr;
        rc = get_delegate(host_context, hdt_load_assembly_and_get_function_pointer, &load_assembly_and_get_function_pointer);
        close(host_context);
        if(rc != 0 || !load_assembly_and_get_function_pointer)
        {
            std::cerr << "hostfxr_get_runtime_delegate failed: 0x" << std::hex << rc << std::dec << std::endl;
            return -1;
        }

        component_entry_point_fn managed_entry = nullptr;
        auto assembly_host_path = to_host_path(assembly_path);
        rc = reinterpret_cast<load_assembly_and_get_function_pointer_fn>(load_assembly_and_get_function_pointer)(
            assembly_host_path.c_str(),
            c_managed_entry_type,
            c_managed_entry_method,
            UNMANAGEDCALLERSONLY_METHOD,
            nullptr,
            reinterpret_cast<void**>(&managed_entry));
        if(rc != 0 || !managed_entry)
        {
            std::cerr << "load_assembly_and_get_function_pointer failed: 0x" << std::hex << rc << std::dec << std::endl;
            return -1;
        }

        std::vector<const char*> managed_arg_values;
        managed_arg_values.reserve(options.managed_args.size());
        for(const auto& arg : options.managed_args)
        {
            managed_arg_values.push_back(arg.c_str());
        }
        ManagedHostArguments args
        {
            static_cast<int32_t>(managed_arg_values.size()),
            managed_arg_values.empty() ? nullptr : managed_arg_values.data()
        };
        return managed_entry(&args, sizeof(args));
    }
}

int luna_main(int argc, const char* argv[])
{
    return run_managed_app(parse_options(argc, argv));
}
