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
#include "Fiber.generated.hpp"

namespace Luna
{
    //! @interface IFiber
    //! Represents a thread executing context that can be switched to 
    //! for a specified thread.
    struct [[Luna::interface("{86a46fbf-43fd-40f3-9aa4-3aa10fa7d6f9}")]] IFiber : virtual Interface
    {
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

    //! Allocates fiber local storage.
    //! Fiber local storage is similar to thread local storage, but it will be switched when the fiber is switched.
    //! If the current thread is not converted to fiber, this behaves the same as thread local storage.
    //! @param[in] destructor The optional destructor to use for this slot. If this is not `nullptr`, this destructor will be called 
    //! one fiber is deleted, one thread is returned, or @ref fls_free for that slot is called and the value of this slot for that thread is not `nullptr`.
    //! 
    //! @return Returns the handle to the allocated slot.
    LUNA_RUNTIME_API R<opaque_t> fls_alloc(void(*destructor)(void* ptr));

    //! Frees fiber local storage.
    LUNA_RUNTIME_API void fls_free(opaque_t handle);

    //! Set the data bound to the current thread's FLS slot specified by `handle`.
    //! @param[in] handle The handle of the slot specified. The handle must be allocated first by @ref fls_alloc.
    //! @param[in] ptr The pointer value to set to this slot.
    LUNA_RUNTIME_API void fls_set(opaque_t handle, void* ptr);

    //! Get the value bound to the FLS slot of current thread.
    //! @param[in] handle The handle of the slot to query.
    //! @return Returns the pointer set, or `nullptr` if no pointer is set to this slot.
    LUNA_RUNTIME_API void* fls_get(opaque_t handle);
}
