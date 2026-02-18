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
#define _XOPEN_SOURCE
#include <ucontext.h>
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
            ucontext_t context;
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
    }
}