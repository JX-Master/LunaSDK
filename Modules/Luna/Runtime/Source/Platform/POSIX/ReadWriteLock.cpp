/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ReadWriteLock.cpp
* @author JXMaster
* @date 2026/2/18
*/
#include "../ReadWriteLock.hpp"
#include "../../../Assert.hpp"

namespace Luna
{
    namespace Platform
    {
        void new_read_write_lock(ReadWriteLock& out_lock)
        {
            auto ret = pthread_rwlock_init(&out_lock.m_lock, nullptr);
            luassert_msg_always(ret == 0, "pthread_rwlock_init failed.");
        }

        void delete_read_write_lock(ReadWriteLock& lock)
        {
            auto ret = pthread_rwlock_destroy(&lock.m_lock);
            luassert_msg_always(ret == 0, "pthread_rwlock_destroy failed.");
        }

        void acquire_read_lock(ReadWriteLock& lock)
        {
            auto ret = pthread_rwlock_rdlock(&lock.m_lock);
            luassert_msg_always(ret == 0, "pthread_rwlock_rdlock failed.");
        }

        void acquire_write_lock(ReadWriteLock& lock)
        {
            auto ret = pthread_rwlock_wrlock(&lock.m_lock);
            luassert_msg_always(ret == 0, "pthread_rwlock_wrlock failed.");
        }

        bool try_acquire_read_lock(ReadWriteLock& lock)
        {
            auto ret = pthread_rwlock_tryrdlock(&lock.m_lock);
            return ret == 0 ? true : false;
        }

        bool try_acquire_write_lock(ReadWriteLock& lock)
        {
            auto ret = pthread_rwlock_trywrlock(&lock.m_lock);
            return ret == 0 ? true : false;
        }

        void release_read_lock(ReadWriteLock& lock)
        {
            auto ret = pthread_rwlock_unlock(&lock.m_lock);
            luassert_msg_always(ret == 0, "pthread_rwlock_unlock failed.");
        }

        void release_write_lock(ReadWriteLock& lock)
        {
            auto ret = pthread_rwlock_unlock(&lock.m_lock);
            luassert_msg_always(ret == 0, "pthread_rwlock_unlock failed.");
        }
    }
}