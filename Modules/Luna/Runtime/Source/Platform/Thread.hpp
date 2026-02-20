/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Thread.hpp
* @author JXMaster
* @date 2026/2/17
*/
#pragma once
#include "../../Thread.hpp"
#include "Result.hpp"
#if defined(LUNA_PLATFORM_WINDOWS)
#include "../../Platform/Windows/MiniWin.hpp"
#elif defined(LUNA_PLATFORM_POSIX)
#include <pthread.h>
#include "Signal.hpp"
#endif

namespace Luna
{
    namespace Platform
    {
        using thread_callback_func_t = void(void* params);

        struct Thread
        {
#if defined(LUNA_PLATFORM_WINDOWS)
            HANDLE m_handle = NULL;
#elif defined(LUNA_PLATFORM_POSIX)
            pthread_t m_handle;        // Thread handle.
            int m_sched_policy;
            sched_param m_sched_param;
            bool m_valid = false;

            // The following variables are valid only for non-main threads.
            Signal m_finish_signal;
#endif
            bool valid() const
            {
#if defined(LUNA_PLATFORM_WINDOWS)
                return m_handle != NULL;
#elif defined(LUNA_PLATFORM_POSIX)
                return m_valid;
#endif
            }
        };

        //! Create a new system thread and make it run the callback function.
        //! @param[in] callback The callback function to be called by the new thread.
        //! @param[in] params The parameter object passed to callback function as parameter.
        //! @param[in] stack_size The stack size for new thread's call stack.
        //! @param[out] out_thread Returns an interface representing the new created thread, or `nullptr` if failed.
        Result new_thread(thread_callback_func_t* callback, void* params, const c8* name, usize stack_size, Thread& out_thread);

        //! Sets the thread schedule priority.
        //! @param[in] thread The thread handle.
        //! @param[in] priority The priority to set.
        Result set_thread_priority(Thread& thread, ThreadPriority priority);

        //! Waits for the thread to finish.
        void wait_thread(Thread& thread);

        //! Tries to wait for the thread to finish.
        bool try_wait_thread(Thread& thread);

        //! Closes the thread handle. 
        //! This frees all resources attached to `thread`. This must be called after
        //! the thread is finished.
        void detach_thread(Thread& thread);

        //! Gets the thrad ID of the current thread.
        usize get_current_thread_id();

        //! Gets the main thread handle.
        //! @param[out] out_thread Returns the main thread handle.
        void get_main_thread(Thread& out_thread);

        //! Suspends current thread for a specific period of time. The actual suspended time may be longer than required.
        //! @param[in] time_milliseconds The time, in milliseconds, that this thread needs to suspend.
        void sleep(u32 time_milliseconds);

        //! Delay the execution of this thread for a very shout time by yielding this thread several times.
        //! This is more accurate to `sleep` method and will not suspend current thread unless the specified time is larger than several milliseconds.
        //! @param[in] time_microseconds The time, in microseconds, that this thread needs to delay.
        void fast_sleep(u32 time_microseconds);

        //! Yields the remain time slice of the current thread and let OS to schedule other threads.
        //! There is no way to resume a thread from user mode, since threads are scheduled by OS automatically.
        //! 
        //! If multi-thread is not supported on the target platform, this function does nothing and returns immediately
        void yield_current_thread();

        //! Allocates one thread local storage (TLS) slot for every thread running in this process, including the thread that is currently not being 
        //! created yet. After the handle is returned, every thread can set a thread-local value to this slot using this handle.
        //! 
        //! The allocated slot is large enough to store one pointer to the real thread-local data. The pointer is `nullptr` for every thread before it 
        //! is firstly modified by that thread.
        //! @param[out] out_handle The handle to the TLS slot, or error code if failed.
        Result tls_alloc(opaque_t& out_handle);

        //! Free the TLS slot allocated by `tls_alloc`. The handle will be invalid after this call and the pointer stored for every 
        //! thread will be discarded.
        //! 
        //! Make sure to free all resources bound to the specified slot manually before calling this, or they will never be freed.
        //! @param[in] tls The handle returned by `tls_alloc`.
        //! @remark Note that calling `tls_free` will not call the destructor registered for this slot on any thread. After `tls_free` is called, the 
        //! destructor will be cleared and will not be called any more.
        void tls_free(opaque_t handle);

        //! Set the data bound to the current thread's TLS slot specified by `handle`.
        //! @param[in] tls The handle of the slot specified.
        //! @param[in] ptr The pointer value to set to this slot.
        //! @par Valid Usage
        //! * `tls` must specifies one valid slot allocated by @ref tls_alloc.
        void tls_set(opaque_t handle, void* ptr);

        //! Get the value bound to the TLS slot of current thread.
        //! @param[in] tls The handle of the slot to query.
        //! @return The pointer set, or `nullptr` if no pointer is set to this slot.
        void* tls_get(opaque_t handle);

        //! Returns the number of logical processors on the platform.
        u32 get_num_processors();
    }
}