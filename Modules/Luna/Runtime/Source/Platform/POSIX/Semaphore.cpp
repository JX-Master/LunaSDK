/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Semaphore.cpp
* @author JXMaster
* @date 2026/2/18
*/
#include "../Semaphore.hpp"
#include "../../../Assert.hpp"
#include "../../../Atomic.hpp"

namespace Luna
{
    namespace Platform
    {
        void new_semaphore(i32 initial_count, i32 max_count, Semaphore& out_sema)
        {
            out_sema.m_max_count = max_count;
            out_sema.m_counter = initial_count;
            luassert_msg_always(pthread_mutex_init(&out_sema.m_mutex, NULL) == 0, "pthread_mutex_init failed.");
            luassert_msg_always(pthread_cond_init(&out_sema.m_cond, NULL) == 0, "pthread_cond_init failed.");
        }

        void delete_semaphore(Semaphore& sema)
        {
            pthread_cond_destroy(&sema.m_cond);
            pthread_mutex_destroy(&sema.m_mutex);
        }

        void acquire_semaphore(Semaphore& sema)
        {
            luassert_msg_always(pthread_mutex_lock(&sema.m_mutex) == 0, "pthread_mutex_lock failed.");
            if (sema.m_counter > 0)
            {
                atom_dec_i32(&sema.m_counter);
                luassert_msg_always(pthread_mutex_unlock(&sema.m_mutex) == 0, "pthread_mutex_unlock failed.");
            }
            atom_dec_i32(&sema.m_counter);
            luassert_msg_always(pthread_cond_wait(&sema.m_cond, &sema.m_mutex) == 0, "pthread_cond_wait failed.");
            atom_inc_i32(&sema.m_counter);
            luassert_msg_always(pthread_mutex_unlock(&sema.m_mutex) == 0, "pthread_mutex_unlock failed.");
        }

        bool try_acquire_semaphore(Semaphore& sema)
        {
            luassert_msg_always(pthread_mutex_lock(&sema.m_mutex) == 0, "pthread_mutex_lock failed.");
            if (sema.m_counter > 0)
            {
                atom_dec_i32(&sema.m_counter);
                luassert_msg_always(pthread_mutex_unlock(&sema.m_mutex) == 0, "pthread_mutex_unlock failed.");
                return true;
            }
            luassert_msg_always(pthread_mutex_unlock(&sema.m_mutex) == 0, "pthread_mutex_unlock failed.");
            return false;
        }

        void release_semaphore(Semaphore& sema)
        {
            luassert_msg_always(pthread_mutex_lock(&sema.m_mutex) == 0, "pthread_mutex_lock failed.");
            bool ret = false;
            if (sema.m_counter < sema.m_max_count)
            {
                if (sema.m_counter < 0)
                {
                    luassert_msg_always(pthread_cond_signal(&sema.m_cond) == 0, "pthread_cond_signal failed.");
                }
                else
                {
                    atom_inc_i32(&sema.m_counter);
                }
            }
            luassert_msg_always(pthread_mutex_unlock(&sema.m_mutex) == 0, "pthread_mutex_unlock failed.");
        }
    }
}