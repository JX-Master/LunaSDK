/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Thread.cpp
* @author JXMaster
* @date 2020/9/28
*/
#include "../../OS.hpp"
#include "../../../Base.hpp"
#include "../../../Assert.hpp"

#include <cstdint>
#include <unistd.h>
#include <pthread.h>

#ifdef LUNA_PLATFORM_LINUX
#include <sys/types.h>
#endif

namespace Luna
{
    namespace OS
    {
        struct Thread
        {
            pthread_t m_handle;        // Thread handle.
            int m_sched_policy;
            sched_param m_sched_param;

            // The following params are used only for non-main thread.

            thread_callback_func_t* m_func = nullptr;
            void* m_params = nullptr;
            opaque_t m_finish_signal = nullptr;

            bool m_detached = false;
            
            c8* m_name_buf = nullptr;
            
            ~Thread()
            {
                if(m_name_buf)
                {
                    memfree(m_name_buf);
                }
            }
        };

        static thread_local Thread* tls_current_thread;
        static Thread main_thread_handle;

        static void* posix_thread_main(void* cookie)
        {
            Thread* t = (Thread*)cookie;
            tls_current_thread = t;
            if(t->m_name_buf)
            {
#ifdef LUNA_PLATFORM_MACOS
                pthread_setname_np(t->m_name_buf);
#elif LUNA_PLATFORM_LINUX
                pthread_setname_np(pthread_self(), t->m_name_buf);
#else
#error "Unrecognized Platform"
#endif
            }
            t->m_func(t->m_params);
            trigger_signal(t->m_finish_signal);
            while (!t->m_detached)
            {
                yield_current_thread();
            }
            memdelete(t);
            return 0;
        }

        void thread_init()
        {
            main_thread_handle.m_handle = pthread_self();
            int r = pthread_getschedparam(main_thread_handle.m_handle, &main_thread_handle.m_sched_policy, &main_thread_handle.m_sched_param);
            luassert_msg_always(r == 0, "pthread_getschedparam failed");
            tls_current_thread = &main_thread_handle;
        }

        opaque_t new_thread(thread_callback_func_t* callback, void* params, const c8* name, usize stack_size)
        {
            Thread* t = memnew<Thread>();
            t->m_func = callback;
            t->m_params = params;
            t->m_finish_signal = new_signal(true);
            if (stack_size == 0)
            {
                stack_size = 2_mb;
            }
            if(name)
            {
                t->m_name_buf = (c8*)memalloc(sizeof(c8) * (strlen(name) + 1));
                memcpy(t->m_name_buf, name, sizeof(c8) * (strlen(name) + 1));
            }
            pthread_attr_t attr;
            pthread_attr_init(&attr);

            pthread_attr_setstacksize(&attr, stack_size);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
            pthread_attr_getschedpolicy(&attr, &(t->m_sched_policy));
            pthread_attr_getschedparam(&attr, &(t->m_sched_param));

            int r = pthread_create(&(t->m_handle), &attr, &posix_thread_main, t);
            pthread_attr_destroy(&attr);
            if (r != 0)
            {
                memdelete(t);
                lupanic_msg_always("pthread_create failed.");
            }
            return t;
        }
        void set_thread_priority(opaque_t thread, ThreadPriority priority)
        {
            Thread* t = (Thread*)thread;
            sched_param param = t->m_sched_param;
            switch (priority)
            {
            case ThreadPriority::low:
                param.sched_priority = (param.sched_priority + sched_get_priority_min(t->m_sched_policy)) >> 1;
                break;
            case ThreadPriority::high:
                param.sched_priority = (param.sched_priority + sched_get_priority_max(t->m_sched_policy)) >> 1;
                break;
            case ThreadPriority::critical:
                param.sched_priority = sched_get_priority_max(t->m_sched_policy);
                break;
            default: break;
            }
            int r = pthread_setschedparam(t->m_handle, t->m_sched_policy, &param);
            luassert_msg_always(r == 0, "pthread_setschedparam failed");
        }
        void wait_thread(opaque_t thread)
        {
            Thread* t = (Thread*)thread;
            wait_signal(t->m_finish_signal);
        }
        bool try_wait_thread(opaque_t thread)
        {
            Thread* t = (Thread*)thread;
            return try_wait_signal(t->m_finish_signal);
        }
        void detach_thread(opaque_t thread)
        {
            Thread* t = (Thread*)thread;
            pthread_detach(t->m_handle);
            t->m_detached = true;
        }
        usize get_current_thread_id()
        {
            static thread_local usize id = 0;
            if (id != 0) return id;
#ifdef LUNA_PLATFORM_MACOS
            static_assert(sizeof(usize) == sizeof(uint64_t), "Only macOS 64-bit is supported.");
            uint64_t tid;
            pthread_threadid_np(0, &tid);
            id = (usize)tid;
#elif LUNA_PLATFORM_LINUX
            id = (usize)gettid();
#else
#error "Unrecognized Platform"
#endif
            return id;
        }
        opaque_t get_current_thread_handle()
        {
            return tls_current_thread;
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
        opaque_t tls_alloc(void (*destructor)(void*))
        {
            pthread_key_t key;
            int r = pthread_key_create(&key, destructor);
            if (r)
            {
                lupanic_msg_always("pthread_key_create failed.");
            }
            return (opaque_t)(usize)key;
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
    }
}