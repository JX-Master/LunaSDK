/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Mutex.cpp
* @author JXMaster
* @date 2022/3/2
*/
#include "../Mutex.hpp"

namespace Luna
{
    namespace Platform
    {
        void new_mutex(Mutex& out_mutex)
        {
            InitializeCriticalSection(&out_mutex.m_sec);
        }

        void delete_mutex(Mutex& mutex)
        {
            DeleteCriticalSection(&mutex.m_sec);
        }

        void lock_mutex(Mutex& mutex)
        {
            EnterCriticalSection(&mutex.m_sec);
        }

        bool try_lock_mutex(Mutex& mutex)
        {
            return TryEnterCriticalSection(&mutex.m_sec);
        }

        void unlock_mutex(Mutex& mutex)
        {
            LeaveCriticalSection(&mutex.m_sec);
        }
    }
}