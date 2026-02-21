/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ReadWriteLock.hpp
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
        struct ReadWriteLock
        {
#if defined(LUNA_PLATFORM_WINDOWS)
            SRWLOCK m_lock;
#elif defined(LUNA_PLATFORM_POSIX)
            pthread_rwlock_t m_lock;
#endif
        };

        //! Initializes read write lock object.
        //! The read write lock object have three modes: unlocked mode, read mode or write mode.
        //! 
        //! On unlocked mode, both read and write request will succeed, changing the lock to read or write mode.
        //! 
        //! On read mode, only read request will succeed. Every read request increases the read count of the lock, and the lock will be changed back to 
        //! unlocked mode when the last read lock is released.
        //! 
        //! On write mode, both read and write request will fail. The read write lock is not recursive, so only one write lock can be acquired.
        //! The lock will be changed back to unlocked mode when the write lock is released.
        void new_read_write_lock(ReadWriteLock& out_lock);

        //! Destroys read write lock object.
        void delete_read_write_lock(ReadWriteLock& lock);

        //! Acquires read lock of the read write lock object.
        //! This call blocks the current thread until the lock is acquired.
        void acquire_read_lock(ReadWriteLock& lock);

        //! Acquires write lock of the read write lock object.
        //! This call blocks the current thread until the lock is acquired.
        void acquire_write_lock(ReadWriteLock& lock);

        //! Tries to acquire read lock of the read write lock object.
        //! This call does not block, it returns `false` if the lock cannot be acquired.
        bool try_acquire_read_lock(ReadWriteLock& lock);

        //! Tries to acquire write lock of the read write lock object.
        //! This call does not block, it returns `false` if the lock cannot be acquired.
        bool try_acquire_write_lock(ReadWriteLock& lock);

        //! Releases the read lock acquired.
        void release_read_lock(ReadWriteLock& lock);

        //! Releases the write lock acquired.
        void release_write_lock(ReadWriteLock& lock);
    }
}