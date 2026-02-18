/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Thread.cpp
* @author JXMaster
* @date 2020/12/10
*/
#include "../PlatformDefines.hpp"
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "Thread.hpp"
#include "Error.hpp"

namespace Luna
{
    Ref<MainThread> g_main_thread;
    IThread* g_main_thread_ref;
    opaque_t g_tls_thread;

    bool thread_init()
    {
        g_main_thread = new_object<MainThread>();
        g_main_thread_ref = query_interface<IThread>(g_main_thread.object());
        Platform::get_main_thread(g_main_thread->m_thread);
        auto r = Platform::tls_alloc(nullptr, g_tls_thread);
        if(r != Platform::Result::success) return false;
        Platform::tls_set(g_tls_thread, g_main_thread_ref);
        return true;
    }
    void thread_close()
    {
        Platform::tls_set(g_tls_thread, nullptr);
        g_main_thread_ref = nullptr;
        g_main_thread = nullptr;
        Platform::tls_free(g_tls_thread);
    }
    static void thread_entry(void* data)
    {
        Thread* th = (Thread*)data;
        IThread* i = query_interface<IThread>(th);
        Platform::tls_set(g_tls_thread, i);
        th->m_entry(th->m_params);
    }
    LUNA_RUNTIME_API u32 get_processors_count()
    {
        return Platform::get_num_processors();
    }
    LUNA_RUNTIME_API R<Ref<IThread>> new_thread(void(*entry_func)(void* params), void* params, const c8* name, u32 stack_size)
    {
        luassert(entry_func);
        Ref<Thread> t = new_object<Thread>();
        t->m_entry = entry_func;
        t->m_params = params;
        auto r = Platform::new_thread(thread_entry, t.object(), name, stack_size, t->m_thread);
        if(r != Platform::Result::success) return encode_platform_result(r).errcode();
        return Ref<IThread>(t);
    }
    LUNA_RUNTIME_API IThread* get_current_thread()
    {
        IThread* th = (IThread*)Platform::tls_get(g_tls_thread);
        return th;
    }
    LUNA_RUNTIME_API usize get_current_thread_id()
    {
        return Platform::get_current_thread_id();
    }
    LUNA_RUNTIME_API IThread* get_main_thread()
    {
        return g_main_thread_ref;
    }
    
    LUNA_RUNTIME_API void sleep(u32 time_milliseconds)
    {
        Platform::sleep(time_milliseconds);
    }
    LUNA_RUNTIME_API void fast_sleep(u32 time_microseconds)
    {
        Platform::fast_sleep(time_microseconds);
    }
    LUNA_RUNTIME_API void yield_current_thread()
    {
        Platform::yield_current_thread();
    }
    LUNA_RUNTIME_API R<opaque_t> tls_alloc(void (*destructor)(void*))
    {
        opaque_t handle;
        auto r = Platform::tls_alloc(destructor, handle);
        if(r != Platform::Result::success) return encode_platform_result(r).errcode();
        return handle;
    }
    LUNA_RUNTIME_API void tls_free(opaque_t handle)
    {
        Platform::tls_free(handle);
    }
    LUNA_RUNTIME_API void tls_set(opaque_t handle, void* ptr)
    {
        Platform::tls_set(handle, ptr);
    }
    LUNA_RUNTIME_API void* tls_get(opaque_t handle)
    {
        return Platform::tls_get(handle);
    }
}
