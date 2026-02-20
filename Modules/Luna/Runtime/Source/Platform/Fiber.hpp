/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Fiber.hpp
* @author JXMaster
* @date 2026/2/17
*/
#pragma once
#include "../../Thread.hpp"
#include "Result.hpp"

#ifdef LUNA_PLATFORM_WINDOWS
#include "../../Platform/Windows/MiniWin.hpp"
#elif defined(LUNA_PLATFORM_POSIX)
#include "POSIX/FiberContext.hpp"
#endif

namespace Luna
{
    namespace Platform
    {
        struct Fiber
        {
#if defined(LUNA_PLATFORM_WINDOWS)
            LPVOID fiber = NULL;
#elif defined(LUNA_PLATFORM_POSIX)
            FiberContext context;
            void* stack = nullptr;
            usize stack_size = 0;
            void(*entry_func)(void*) = nullptr;
            void* param = nullptr;
#else
#error "Unsupported Platform"
#endif
        };

        //! Creates a new fiber.
        //! @param[in] stack_size The stack size in bytes of the new fiber object.
        //! @param[in] entry_func The entry function of the fiber.
        //! @param[in] param The parameter that will be passed to `entry_func` when the fiber is 
        //! activated.
        //! @param[out] out_fiber Returns the created fiber.
        Result new_fiber(usize stack_size, void(*entry_func)(void* param), void* param, Fiber& out_fiber);

        //! Deletes a fiber. This call frees all resources attached to the specifiec fiber.
        //! Only fibers created by @ref new_fiber should be deleted. Fibers created by @ref convert_thread_to_fiber
        //! should not be deleted.
        void delete_fiber(Fiber& fiber);

        //! Creates a fiber context for current thread.
        //! @return Returns the fiber context created for the current thread.
        //! Calling this function multiple times on the same thread does not create multiple fiber contexts, 
        //! instead the same fiber context will be returned.
        Result convert_thread_to_fiber(Fiber& out_fiber);

        //! Frees the fiber context for the current thread.
        Result convert_fiber_to_thread();

        //! Sets the context of the current thread to the specified fiber.
        //! @param[in] fiber The fiber to switch to.
        void switch_to_fiber(Fiber& fiber);

        //! Allocates fiber local storage.
        //! Fiber local storage is similar to thread local storage, but it will be switched when the fiber is switched.
        //! If the current thread is not converted to fiber, this behaves the same as thread local storage.
        //! @param[in] destructor The optional destructor to use for this slot. If this is not `nullptr`, this destructor will be called 
        //! one fiber is deleted, one thread is returned, or @ref fls_free for that slot is called and the value of this slot for that thread is not `nullptr`.
        //! 
        //! @param[out] out_handle Returns the handle to the allocated slot.
        Result fls_alloc(void(*destructor)(void* ptr), opaque_t& out_handle);

        //! Frees fiber local storage.
        void fls_free(opaque_t handle);

        //! Set the data bound to the current thread's FLS slot specified by `handle`.
        //! @param[in] handle The handle of the slot specified. The handle must be allocated first by @ref fls_alloc.
        //! @param[in] ptr The pointer value to set to this slot.
        void fls_set(opaque_t handle, void* ptr);

        //! Get the value bound to the FLS slot of current thread.
        //! @param[in] handle The handle of the slot to query.
        //! @return Returns the pointer set, or `nullptr` if no pointer is set to this slot.
        void* fls_get(opaque_t handle);
    }
}