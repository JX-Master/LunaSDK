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
#include "Thread.hpp"

namespace Luna
{
    //! @interface IFiber
    //! Represents a thread executing context that can be switched to 
    //! for a specified thread.
    struct IFiber : virtual Interface
    {
        luiid("{86a46fbf-43fd-40f3-9aa4-3aa10fa7d6f9}");

    };

    //! Creates a new fiber object.
    //! @param[in] stack_size The stack size in bytes of the new fiber object.
    //! @param[in] entry_func The entry function of the fiber.
    //! @param[in] param The parameter that will be passed to `entry_func` when the fiber is 
    //! activated.
    LUNA_RUNTIME_API R<Ref<IFiber>> new_fiber(usize stack_size, void(*entry_func)(void* param), void* param);

    //! Creates a fiber context for current thread.
    //! @return Returns the fiber context created for the current thread.
    //! Calling this function multiple times on the same thread does not create multiple fiber contexts, 
    //! instead the same fiber context will be returned.
    LUNA_RUNTIME_API R<Ref<IFiber>> convert_thread_to_fiber();

    //! Frees the fiber context for the current thread.
    LUNA_RUNTIME_API RV convert_fiber_to_thread();

    //! Sets the context of the current thread to the specified fiber.
    //! @param[in] fiber The fiber to switch to.
    LUNA_RUNTIME_API void switch_to_fiber(IFiber* fiber);

    //! Gets the current fiber context of the current thread.
    //! @return Returns the fiber context of the current thread. Returns `nullptr` if the current thread 
    //! is not converted to a fiber.
    LUNA_RUNTIME_API IFiber* get_current_fiber();
}