/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Thread.cpp
* @author JXMaster
* @date 2020/9/28
*/
#include "../Thread.hpp"
#include "../../../Base.hpp"
#include "../../../Assert.hpp"
#include "../Time.hpp"
#include "Errno.hpp"

#include <cstdint>
#include <unistd.h>

#ifdef LUNA_PLATFORM_LINUX
#include <sys/types.h>
#endif

#ifdef LUNA_PLATFORM_MACOS
#include <sys/sysctl.h>
#endif

namespace Luna
{
    namespace Platform
    {
        static pthread_t g_main_thread_handle;

        struct ThreadContext
        {
            Thread* m_owner;
            thread_callback_func_t* m_func = nullptr;
            void* m_params = nullptr;
            c8* m_name_buf = nullptr;

            ~ThreadContext()
            {
                if(m_name_buf)
                {
                    memfree(m_name_buf);
                }
            }
        };

        static void* posix_thread_main(void* cookie)
        {
            ThreadContext* t = (ThreadContext*)cookie;
            if(t->m_name_buf)
            {
#if defined(LUNA_PLATFORM_MACOS) || defined(LUNA_PLATFORM_IOS)
                pthread_setname_np(t->m_name_buf);
#elif defined(LUNA_PLATFORM_LINUX) || defined(LUNA_PLATFORM_ANDROID)
                pthread_setname_np(pthread_self(), t->m_name_buf);
#else
#error "Unrecognized Platform"
#endif
                memfree(t->m_name_buf);
                t->m_name_buf = nullptr;
            }
            t->m_func(t->m_params);
            trigger_signal(t->m_owner->m_finish_signal);
            memdelete(t);
            return 0;
        }

        void thread_init()
        {
            g_main_thread_handle = pthread_self();
        }

        Result new_thread(thread_callback_func_t* callback, void* params, const c8* name, usize stack_size, Thread& out_thread)
        {
            ThreadContext* t = memnew<ThreadContext>();
            t->m_func = callback;
            t->m_params = params;
            t->m_owner = &out_thread;
            if(name)
            {
                t->m_name_buf = (c8*)memalloc(sizeof(c8) * (strlen(name) + 1));
                memcpy(t->m_name_buf, name, sizeof(c8) * (strlen(name) + 1));
            }
            new_signal(true, out_thread.m_finish_signal);
            if (stack_size == 0)
            {
                stack_size = 2_mb;
            }
            pthread_attr_t attr;
            pthread_attr_init(&attr);

            pthread_attr_setstacksize(&attr, stack_size);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
            pthread_attr_getschedpolicy(&attr, &(out_thread.m_sched_policy));
            pthread_attr_getschedparam(&attr, &(out_thread.m_sched_param));

            int r = pthread_create(&(out_thread.m_handle), &attr, &posix_thread_main, t);
            pthread_attr_destroy(&attr);
            if (r != 0)
            {
                memdelete(t);
                delete_signal(out_thread.m_finish_signal);
                return encode_errno(r);
            }
            out_thread.m_valid = true;
            return Result::success;
        }
        Result set_thread_priority(Thread& thread, ThreadPriority priority)
        {
            sched_param param = thread.m_sched_param;
            switch (priority)
            {
            case ThreadPriority::low:
                param.sched_priority = (param.sched_priority + sched_get_priority_min(thread.m_sched_policy)) >> 1;
                break;
            case ThreadPriority::high:
                param.sched_priority = (param.sched_priority + sched_get_priority_max(thread.m_sched_policy)) >> 1;
                break;
            case ThreadPriority::critical:
                param.sched_priority = sched_get_priority_max(thread.m_sched_policy);
                break;
            default: break;
            }
            int r = pthread_setschedparam(thread.m_handle, thread.m_sched_policy, &param);
            if(r != 0)
            {
                return encode_errno(r);
            }
            return Result::success;
        }
        void wait_thread(Thread& thread)
        {
            wait_signal(thread.m_finish_signal);
        }
        bool try_wait_thread(Thread& thread)
        {
            return try_wait_signal(thread.m_finish_signal);
        }
        void detach_thread(Thread& thread)
        {
            pthread_detach(thread.m_handle);
            thread.m_valid = false;
        }
        usize get_current_thread_id()
        {
            static thread_local usize id = 0;
            if (id != 0) return id;
#if defined(LUNA_PLATFORM_MACOS) || defined(LUNA_PLATFORM_IOS)
            static_assert(sizeof(usize) == sizeof(uint64_t), "Only macOS 64-bit is supported.");
            uint64_t tid;
            pthread_threadid_np(0, &tid);
            id = (usize)tid;
#elif defined(LUNA_PLATFORM_LINUX) || defined(LUNA_PLATFORM_ANDROID)
            id = (usize)gettid();
#else
#error "Unrecognized Platform"
#endif
            return id;
        }
        void get_main_thread(Thread& out_thread)
        {
            out_thread.m_handle = g_main_thread_handle;
            int r = pthread_getschedparam(g_main_thread_handle, &out_thread.m_sched_policy, &out_thread.m_sched_param);
            luassert_msg_always(r == 0, "pthread_getschedparam failed");
            out_thread.m_valid = true;
        }
        void sleep(u32 time_milliseconds)
        {
            ::usleep(time_milliseconds * 1000);
        }
        void fast_sleep(u32 time_microseconds)
        {
            u64 t = get_ticks();
            f64 tps = get_ticks_per_second();

            u64 end_time = t + (u64)(tps * time_microseconds / 1000000);
            if (time_microseconds > 4000) //4ms
            {
                ::usleep(time_microseconds);
            }
            t = get_ticks();
            while ((u64)(t) < end_time)
            {
                yield_current_thread();
                yield_current_thread();
                yield_current_thread();
                yield_current_thread();
            }
        }
        void yield_current_thread()
        {
            ::sched_yield();
        }
        Result tls_alloc(void (*destructor)(void*), opaque_t& out_handle)
        {
            pthread_key_t key;
            int r = pthread_key_create(&key, destructor);
            if (r)
            {
                return encode_errno(r);
            }
            out_handle = (opaque_t)(usize)key;
            return Result::success;
        }
        void tls_free(opaque_t handle)
        {
            pthread_key_t key = (pthread_key_t)(usize)handle;
            pthread_key_delete(key);
        }
        void tls_set(opaque_t handle, void* ptr)
        {
            pthread_key_t key = (pthread_key_t)(usize)handle;
            int r = pthread_setspecific(key, ptr);
            if (r)
            {
                lupanic_msg_always("pthread_setspecific failed.");
            }
        }
        void* tls_get(opaque_t handle)
        {
            pthread_key_t key = (pthread_key_t)(usize)handle;
            void* k = pthread_getspecific(key);
            return k;
        }
        u32 get_num_processors()
        {
#ifdef LUNA_PLATFORM_MACOS
            size_t size;
            int name[2];
            size = 4;
            name[0] = CTL_HW;
            name[1] = HW_NCPU;
            int processor_count;
            if (sysctl(name, 2, &processor_count, &size, nullptr, 0) != 0)
            {
                processor_count = 1;
            }
            return (u32)processor_count;
#else
            int processor_count = max<int>(sysconf(_SC_NPROCESSORS_ONLN), 1);
            return (u32)processor_count;
#endif
        }
    }
}