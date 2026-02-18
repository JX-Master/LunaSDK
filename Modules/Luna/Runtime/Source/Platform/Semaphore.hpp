/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Semaphore.hpp
* @author JXMaster
* @date 2026/2/17
*/
#pragma once
#include "../../Base.hpp"
#if defined(LUNA_PLATFORM_WINDOWS)
#include "../../Platform/Windows/MiniWin.hpp"
#elif defined(LUNA_PLATFORM_POSIX)

#endif

namespace Luna
{
    namespace Platform
    {
        struct Semaphore
        {
#if defined(LUNA_PLATFORM_WINDOWS)
            HANDLE m_handle;
#elif defined(LUNA_PLATFORM_POSIX)

#endif
        };

        //! Creates one semaphore object.
        void new_semaphore(i32 initial_count, i32 max_count, Semaphore& out_sema);

        //! Destroys one semaphore object.
        void delete_semaphore(Semaphore& sema);

        //! Acquires one semaphore object.
        void acquire_semaphore(Semaphore& sema);

        //! Tries to acquire one semaphore object.
        bool try_acquire_semaphore(Semaphore& sema);

        //! Releases the semaphore acquired.
        void release_semaphore(Semaphore& sema);
    }
}