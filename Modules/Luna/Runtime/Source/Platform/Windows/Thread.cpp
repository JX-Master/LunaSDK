/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Thread.cpp
* @author JXMaster
* @date 2020/7/30
*/
#include "../../OS.hpp"
#include "../../../Platform/Windows/MiniWin.hpp"
#include <Luna/Runtime/HashMap.hpp>
#include "../../../SpinLock.hpp"
#include "../../../Unicode.hpp"
#include "Utils.hpp"

namespace Luna
{
	namespace OS
	{
		using tls_destructor = void(void*);
		Unconstructed<HashMap<DWORD, tls_destructor*, hash<DWORD>, equal_to<DWORD>, OSAllocator>> g_allocated_tls;
		SpinLock g_allocated_tls_mtx;
		struct Thread
		{
			thread_callback_func_t* m_func;
			void* m_params;
		};
	}
}
DWORD WINAPI WinThreadEntry(LPVOID cookie)
{
	using namespace Luna;
	using namespace Luna::OS;
	Thread* ctx = (Thread*)cookie;
	ctx->m_func(ctx->m_params);
	// Clean up all tls.
	g_allocated_tls_mtx.lock();
	auto& tls = g_allocated_tls.get();
	for (auto& i : tls)
	{
		void* p = OS::tls_get(opaque_t((usize)i.first));
		if (p)
		{
			OS::tls_set(opaque_t((usize)i.first), nullptr);
			i.second(p);
		}
	}
	g_allocated_tls_mtx.unlock();
	Luna::memdelete(ctx);
	return 0;
}
namespace Luna
{
	namespace OS
	{
		void thread_init()
		{
			g_allocated_tls.construct();
		}
		void thread_close()
		{
			g_allocated_tls.destruct();
		}
		opaque_t new_thread(thread_callback_func_t* callback, void* params, const c8* name, usize stack_size)
		{
			luassert(callback);
			Thread* t = Luna::memnew<Thread>();
			t->m_func = callback;
			t->m_params = params;
			DWORD tid;
			HANDLE h = ::CreateThread(NULL, stack_size, &WinThreadEntry, t, CREATE_SUSPENDED, &tid);
			if (!h)
			{
				DWORD dw = ::GetLastError();
				Luna::memdelete(t);
				lupanic_msg_always("CreateThread failed.");
			}
			if (name)
			{
				wchar_t* buf = utf8_to_wchar_buffered(name);
				auto r = SetThreadDescription(h, buf);
				if (FAILED(r))
				{
					lupanic_msg_always("SetThreadDescription failed.");
				}
				memfree(buf);
			}
			::ResumeThread(h);
			return h;
		}
		void set_thread_priority(opaque_t thread, ThreadPriority priority)
		{
			BOOL r;
			switch (priority)
			{
			case ThreadPriority::low:
				r = ::SetThreadPriority((HANDLE)thread, THREAD_PRIORITY_LOWEST);
				break;
			case ThreadPriority::normal:
				r = ::SetThreadPriority((HANDLE)thread, THREAD_PRIORITY_NORMAL);
				break;
			case ThreadPriority::high:
				r = ::SetThreadPriority((HANDLE)thread, THREAD_PRIORITY_HIGHEST);
				break;
			case ThreadPriority::critical:
				r = ::SetThreadPriority((HANDLE)thread, THREAD_PRIORITY_TIME_CRITICAL);
				break;
			}
			if (!r)
			{
				lupanic_msg_always("SetThreadPriority failed for thread object");
			}
		}
		void wait_thread(opaque_t thread)
		{
			if (::WaitForSingleObject((HANDLE)thread, INFINITE) != WAIT_OBJECT_0)
			{
				lupanic_msg_always("WaitForSingleObject failed for thread object");
			}
		}
		bool try_wait_thread(opaque_t thread)
		{
			DWORD r = ::WaitForSingleObject((HANDLE)thread, 0);
			if (r == WAIT_OBJECT_0)
			{
				return true;
			}
			if (r != WAIT_TIMEOUT)
			{
				lupanic_msg_always("WaitForSingleObject failed for thread object");
			}
			return false;
		}
		void detach_thread(opaque_t thread)
		{
			::CloseHandle((HANDLE)thread);
		}
		opaque_t get_current_thread_handle()
		{
			return opaque_t(::GetCurrentThread());
		}
		void sleep(u32 time_milliseconds)
		{
			::Sleep(time_milliseconds);
		}
		void fast_sleep(u32 time_microseconds)
		{
			LARGE_INTEGER currentTime;
			::QueryPerformanceCounter(&currentTime);
			LARGE_INTEGER ticksPerSnd;
			::QueryPerformanceFrequency(&ticksPerSnd);
			u64 endTime = currentTime.QuadPart + ticksPerSnd.QuadPart * time_microseconds / 1000000;
			if (time_microseconds > 4000) //4ms
			{
				::Sleep(time_microseconds / 1000);
			}
			::QueryPerformanceCounter(&currentTime);
			while ((u64)(currentTime.QuadPart) < endTime)
			{
				::SwitchToThread();
				::SwitchToThread();
				::SwitchToThread();
				::SwitchToThread();
			}
		}
		void yield_current_thread()
		{
			SwitchToThread();
		}
		opaque_t tls_alloc(void (*destructor)(void*))
		{
			DWORD index = TlsAlloc();
			if (index == TLS_OUT_OF_INDEXES)
			{
				lupanic_msg_always("TlsAlloc failed with TLS_OUT_OF_INDEXES.");
			}
			if (destructor)
			{
				g_allocated_tls_mtx.lock();
				g_allocated_tls.get().insert(make_pair(index, destructor));
				g_allocated_tls_mtx.unlock();
			}
			return opaque_t((usize)index);
		}
		void tls_free(opaque_t handle)
		{
			if (TlsFree((DWORD)(usize)(usize)handle))
			{
				g_allocated_tls_mtx.lock();
				g_allocated_tls.get().erase((DWORD)(usize)handle);
				g_allocated_tls_mtx.unlock();
			}
		}
		void tls_set(opaque_t handle, void* ptr)
		{
			if (!TlsSetValue((DWORD)(usize)handle, ptr))
			{
				lupanic_msg_always("TlsSetValue failed.");
			}
		}
		void* tls_get(opaque_t handle)
		{
			return ::TlsGetValue((DWORD)(usize)handle);
		}
	}
}