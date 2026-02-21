/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Thread.cpp
* @author JXMaster
* @date 2020/7/30
*/
#include "../Thread.hpp"
#include <Luna/Runtime/HashMap.hpp>
#include "../../../SpinLock.hpp"
#include "../../../Unicode.hpp"
#include "Utils.hpp"
#include "../Memory.hpp"
#include "ErrCode.hpp"

namespace Luna
{
    namespace Platform
    {
        struct ThreadContext
        {
            thread_callback_func_t* m_func;
            void* m_params;
        };
        HANDLE g_main_thread_handle = NULL;
    }
}
DWORD WINAPI WinThreadEntry(LPVOID cookie)
{
    using namespace Luna;
    Platform::ThreadContext* ctx = (Platform::ThreadContext*)cookie;
    ctx->m_func(ctx->m_params);
    Luna::memdelete(ctx);
    return 0;
}
namespace Luna
{
    namespace Platform
    {
        Result thread_init()
        {
            g_main_thread_handle = ::OpenThread(THREAD_ALL_ACCESS, FALSE, ::GetCurrentThreadId());
            if(!g_main_thread_handle) return Result::bad_platform_call;
            return Result::success;
        }
        void thread_close()
        {
            ::CloseHandle(g_main_thread_handle);
            g_main_thread_handle = NULL;
        }
        Result new_thread(thread_callback_func_t* callback, void* params, const c8* name, usize stack_size, Thread& out_thread)
        {
            luassert(callback);
            ThreadContext* t = Luna::memnew<ThreadContext>();
            t->m_func = callback;
            t->m_params = params;
            DWORD tid;
            HANDLE h = ::CreateThread(NULL, stack_size, &WinThreadEntry, t, CREATE_SUSPENDED, &tid);
            if (!h)
            {
                DWORD err = ::GetLastError();
                Luna::memdelete(t);
                return translate_last_error(err);
            }
            if (name)
            {
                wchar_t* buf = utf8_to_wchar_buffered(name);
                auto r = SetThreadDescription(h, buf);
                memfree(buf);
                if (FAILED(r))
                {
                    lupanic();
                }
            }
            ::ResumeThread(h);
            out_thread.m_handle = h;
            return Result::success;
        }
        Result set_thread_priority(Thread& thread, ThreadPriority priority)
        {
            BOOL r;
            switch (priority)
            {
            case ThreadPriority::low:
                r = ::SetThreadPriority(thread.m_handle, THREAD_PRIORITY_LOWEST);
                break;
            case ThreadPriority::normal:
                r = ::SetThreadPriority(thread.m_handle, THREAD_PRIORITY_NORMAL);
                break;
            case ThreadPriority::high:
                r = ::SetThreadPriority(thread.m_handle, THREAD_PRIORITY_HIGHEST);
                break;
            case ThreadPriority::critical:
                r = ::SetThreadPriority(thread.m_handle, THREAD_PRIORITY_TIME_CRITICAL);
                break;
            }
            if (!r)
            {
                DWORD err = ::GetLastError();
                return translate_last_error(err);
            }
            return Result::success;
        }
        void wait_thread(Thread& thread)
        {
            if (::WaitForSingleObject(thread.m_handle, INFINITE) != WAIT_OBJECT_0)
            {
                lupanic_msg_always("WaitForSingleObject failed for thread object");
            }
        }
        bool try_wait_thread(Thread& thread)
        {
            DWORD r = ::WaitForSingleObject(thread.m_handle, 0);
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
        void detach_thread(Thread& thread)
        {
            ::CloseHandle(thread.m_handle);
        }
        usize get_current_thread_id()
        {
            return (usize)::GetCurrentThreadId();
        }
        void get_main_thread(Thread& out_thread)
        {
            out_thread.m_handle = g_main_thread_handle;
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
                processor_pause();
                processor_pause();
                processor_pause();
                processor_pause();
                ::QueryPerformanceCounter(&currentTime);
            }
        }
        void yield_current_thread()
        {
            SwitchToThread();
        }
        Result tls_alloc(opaque_t& out_handle)
        {
            DWORD index = TlsAlloc();
            if (index == TLS_OUT_OF_INDEXES)
            {
                DWORD err = GetLastError();
                return translate_last_error(err);
            }
            out_handle = (opaque_t)(usize)index;
            return Result::success;
        }
        void tls_free(opaque_t handle)
        {
            DWORD index = (DWORD)(usize)handle;
            TlsFree(index);
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
        u32 get_num_processors()
        {
            SYSTEM_INFO si;
            memzero(&si, sizeof(SYSTEM_INFO));
            ::GetSystemInfo(&si);
            return si.dwNumberOfProcessors;
        }
    }
}