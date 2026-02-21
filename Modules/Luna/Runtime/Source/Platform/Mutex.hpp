/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Mutex.hpp
* @author JXMaster
* @date 2026/2/17
*/
#pragma once
#include "../../Base.hpp"
#if defined(LUNA_PLATFORM_WINDOWS)
#include "../../Platform/Windows/MiniWin.hpp"
#elif defined(LUNA_PLATFORM_POSIX)
#include <pthread.h>
#endif

namespace Luna
{
    namespace Platform
    {
        struct Mutex
        {
#if defined(LUNA_PLATFORM_WINDOWS)
            CRITICAL_SECTION m_sec;
#elif defined(LUNA_PLATFORM_POSIX)
            pthread_mutex_t m_mtx;
#endif
        };

        //! Creates one mutex object.
        void new_mutex(Mutex& out_mutex);

        //! Destroys one mutex object.
        void delete_mutex(Mutex& mutex);

        //! Locks the mutex object.
        void lock_mutex(Mutex& mutex);

        //! Tries to lock the mutex object, returns immediately if the mutex is already locked by another thread.
        bool try_lock_mutex(Mutex& mutex);

        //! Releases the mutex object.
        void unlock_mutex(Mutex& mutex);

        struct MutexGuard
        {
            Mutex& m_handle;

            MutexGuard(Mutex& h) :
                m_handle(h)
            {
                Platform::lock_mutex(m_handle);
            }
            ~MutexGuard()
            {
                Platform::unlock_mutex(m_handle);
            }
        };
    }
}