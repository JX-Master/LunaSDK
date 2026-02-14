/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Coroutine.hpp
* @author JXMaster
* @date 2026/2/14
*/
#pragma once
#include "Thread.hpp"

namespace Luna
{
    //! @addtogroup Runtime
    //! @{
    //! @defgroup RuntimeCoroutine Stackful coroutine library.
    //! @}

    //! @addtogroup RuntimeCoroutine
    //! @{

    //! @interface ICoroutine
    //! Represents one coroutine object that can be switch to.
    struct ICoroutine : virtual Interface
    {
        luiid("{1b962e21-009e-401f-aa45-5434101250ce}");

        //! Gets the underlying fiber context for this coroutine.
        virtual IFiber* get_fiber() = 0;
    };

    //! Gets the current executing coroutine.
    //! @return Returns the current executing coroutine. Returns `nullptr` if called in a non-coroutine context.
    LUNA_RUNTIME_API ICoroutine* get_current_coroutine();

    //! Creates a new coroutine context.
    //! @param[in] stack_size The stack size in bytes of the new underlying fiber object of the coroutine.
    //! @param[in] entry_func The entry function of the coroutine.
    //! @param[in] param The parameter that will be passed to `entry_func` when the coroutine is 
    //! activated.
    //! @remarks Generally the user should use coroutines instead of fibers, since coroutine provides a more 
    //! well-formed calling structure between parent and child procedures, preventing protential bugs caused by
    //! C++ local variables not destructed correctly.
    LUNA_RUNTIME_API R<Ref<ICoroutine>> new_coroutine(usize stack_size, void(*entry_func)(void* param), void* param);

    //! Resumes to the target coroutine.
    //! @param[in] coroutine The coroutine to resume.
    //! @par Valid Usage
    //! The current thread must be converted to fiber first by calling @ref convert_thread_to_fiber in order
    //! to run coroutine.
    LUNA_RUNTIME_API void resume_coroutine(ICoroutine* coroutine);

    //! Yields the current coroutine to its parent coroutine.
    LUNA_RUNTIME_API void yield_coroutine();

    //! @}
}