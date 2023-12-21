/*!
* This file is a portion of Luna SDK.
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

namespace Luna
{
	Ref<MainThread> g_main_thread;
	IThread* g_main_thread_ref;
	opaque_t g_tls_thread;

	void thread_init()
	{
		g_main_thread = new_object<MainThread>();
		g_main_thread_ref = query_interface<IThread>(g_main_thread.object());
		g_main_thread->m_handle = OS::get_current_thread_handle();
		g_tls_thread = OS::tls_alloc(nullptr);
		OS::tls_set(g_tls_thread, g_main_thread_ref);
	}
	void thread_close()
	{
		OS::tls_set(g_tls_thread, nullptr);
		g_main_thread_ref = nullptr;
		g_main_thread = nullptr;
		OS::tls_free(g_tls_thread);
	}
	static void thread_entry(void* data)
	{
		Thread* th = (Thread*)data;
		IThread* i = query_interface<IThread>(th);
		OS::tls_set(g_tls_thread, i);
		th->m_entry(th->m_params);
	}
	LUNA_RUNTIME_API u32 get_processors_count()
	{
		return OS::get_num_processors();
	}
	LUNA_RUNTIME_API Ref<IThread> new_thread(void(*entry_func)(void* params), void* params, const c8* name, u32 stack_size)
	{
		luassert(entry_func);
		Ref<Thread> t = new_object<Thread>();
		t->m_entry = entry_func;
		t->m_params = params;
		t->m_handle = OS::new_thread(thread_entry, t.object(), name, stack_size);
		return t;
	}
	LUNA_RUNTIME_API IThread* get_current_thread()
	{
		IThread* th = (IThread*)OS::tls_get(g_tls_thread);
		return th;
	}
	LUNA_RUNTIME_API IThread* get_main_thread()
	{
		return g_main_thread_ref;
	}
	LUNA_RUNTIME_API void sleep(u32 time_milliseconds)
	{
		OS::sleep(time_milliseconds);
	}
	LUNA_RUNTIME_API void fast_sleep(u32 time_microseconds)
	{
		OS::fast_sleep(time_microseconds);
	}
	LUNA_RUNTIME_API void yield_current_thread()
	{
		OS::yield_current_thread();
	}
	LUNA_RUNTIME_API opaque_t tls_alloc(void (*destructor)(void*))
	{
		return OS::tls_alloc(destructor);
	}
	LUNA_RUNTIME_API void tls_free(opaque_t handle)
	{
		OS::tls_free(handle);
	}
	LUNA_RUNTIME_API void tls_set(opaque_t handle, void* ptr)
	{
		return OS::tls_set(handle, ptr);
	}
	LUNA_RUNTIME_API void* tls_get(opaque_t handle)
	{
		return OS::tls_get(handle);
	}
}
