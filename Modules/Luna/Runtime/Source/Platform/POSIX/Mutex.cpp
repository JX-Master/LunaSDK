/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Mutex.cpp
* @author JXMaster
* @date 2026/2/18
*/
#include "../Mutex.hpp"
#include "../../../Assert.hpp"

namespace Luna
{
    namespace Platform
    {
        void new_mutex(Mutex& out_mutex)
        {
            pthread_mutexattr_t attr;
            luassert_msg_always(pthread_mutexattr_init(&attr) == 0, "pthread_mutexattr_init failed.");
            luassert_msg_always(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) == 0, "pthread_mutexattr_settype failed.");
            luassert_msg_always(pthread_mutex_init(&out_mutex.m_mtx, &attr) == 0, "pthread_mutex_init failed.");
            pthread_mutexattr_destroy(&attr);
        }

        void delete_mutex(Mutex& mutex)
        {
            pthread_mutex_destroy(&mutex.m_mtx);
        }

        void lock_mutex(Mutex& mutex)
        {
            luassert_msg_always(pthread_mutex_lock(&mutex.m_mtx) == 0, "pthread_mutex_lock failed.");
        }

        bool try_lock_mutex(Mutex& mutex)
        {
            int rv = pthread_mutex_trylock(&mutex.m_mtx);
            return (rv == 0) ? true : false;
        }

        void unlock_mutex(Mutex& mutex)
        {
            luassert_msg_always(pthread_mutex_unlock(&mutex.m_mtx) == 0, "pthread_mutex_unlock failed.");
        }
    }
}