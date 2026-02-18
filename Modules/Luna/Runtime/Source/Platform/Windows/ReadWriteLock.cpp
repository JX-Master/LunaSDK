/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ReadWriteLock.cpp
* @author JXMaster
* @date 2022/8/29
*/
#include "../ReadWriteLock.hpp"

namespace Luna
{
    namespace Platform
    {
        void new_read_write_lock(ReadWriteLock& out_lock)
        {
            InitializeSRWLock(&out_lock.m_lock);
        }
        void delete_read_write_lock(ReadWriteLock& lock)
        {
            
        }
        void acquire_read_lock(ReadWriteLock& lock)
        {
            AcquireSRWLockShared(&lock.m_lock);
        }
        void acquire_write_lock(ReadWriteLock& lock)
        {
            AcquireSRWLockExclusive(&lock.m_lock);
        }
        bool try_acquire_read_lock(ReadWriteLock& lock)
        {
            return TryAcquireSRWLockShared(&lock.m_lock);
        }
        bool try_acquire_write_lock(ReadWriteLock& lock)
        {
            return TryAcquireSRWLockExclusive(&lock.m_lock);
        }
        void release_read_lock(ReadWriteLock& lock)
        {
            ReleaseSRWLockShared(&lock.m_lock);
        }
        void release_write_lock(ReadWriteLock& lock)
        {
            ReleaseSRWLockExclusive(&lock.m_lock);
        }
    }
}