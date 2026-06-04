/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Runtime.cpp
* @author JXMaster
* @date 2020/12/10
*/
#include "../PlatformDefines.hpp"
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "../Runtime.hpp"
#include "NameImpl.hpp"
#include "ModuleImpl.hpp"
#include "Platform/Init.hpp"
#include "MemoryImpl.hpp"
#include "../Waitable.hpp"
#include "SignalImpl.hpp"
#include "MutexImpl.hpp"
#include "SemaphoreImpl.hpp"
#include "FileImpl.hpp"
#include "ThreadImpl.hpp"
#include "FiberImpl.hpp"
#include "CoroutineImpl.hpp"
#include "TypeInfoImpl.hpp"
#include "InterfaceImpl.hpp"
#include "RandomImpl.hpp"
#include "ReadWriteLockImpl.hpp"
#include "StdIOImpl.hpp"
#include "ProfilerImpl.hpp"
#include "Runtime.meta.generated.hpp"
namespace Luna
{
    bool error_init();
    void error_close();
    void object_close();
    void add_builtin_typeinfo();
    bool stack_allocator_init();

    void log_init();
    void log_close();
    void stack_allocator_close();

    void register_types_and_interfaces()
    {
        Meta::register_Runtime_types();
    }

    static bool g_initialized = false;

    LUNA_RUNTIME_API bool init()
    {
        if (g_initialized) return true;
        auto r = Platform::init();
        if(r != Platform::Result::success) [[unlikely]] return false;
        if(!profiler_init()) [[unlikely]]
        {
            Platform::close();
            return false;
        }
        if(!error_init()) [[unlikely]]
        {
            profiler_close();
            Platform::close();
            return false;
        }
        name_init();
        type_registry_init();
        add_builtin_typeinfo();
        register_types_and_interfaces();
        if(!thread_init()) [[unlikely]]
        {
            object_close();
            type_registry_close();
            name_close();
            error_close();
            profiler_close();
            Platform::close();
            return false;
        }
        if(!coroutine_init()) [[unlikely]]
        {
            thread_close();
            object_close();
            type_registry_close();
            name_close();
            error_close();
            profiler_close();
            Platform::close();
            return false;
        }
        if(!stack_allocator_init()) [[unlikely]]
        {
            coroutine_close();
            thread_close();
            object_close();
            type_registry_close();
            name_close();
            error_close();
            profiler_close();
            Platform::close();
            return false;
        }
        random_init();
        log_init();
        std_io_init();
        module_init();
        g_profiler_ready = true;
        g_initialized = true;
        return true;
    }
    LUNA_RUNTIME_API bool is_initialized()
    {
        return g_initialized;
    }
    LUNA_RUNTIME_API void close()
    {
        if (!g_initialized) return;
        module_close();
        g_profiler_ready = false;
        std_io_close();
        log_close();
        random_close();
        stack_allocator_close();
        coroutine_close();
        thread_close();
        object_close();
        type_registry_close();
        name_close();
        error_close();
        profiler_close();
        Platform::close();
        g_initialized = false;
    }
}
