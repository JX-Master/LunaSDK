/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Signal.cpp
* @author JXMaster
* @date 2026/2/18
*/
#include "../Signal.hpp"
#include "../../../Assert.hpp"
#include <sys/time.h>

namespace Luna
{
    namespace Platform
    {
        void new_signal(bool manual_reset, Signal& out_signal)
        {
            out_signal.m_signaled = false;
            luassert_msg_always(pthread_mutex_init(&out_signal.m_mutex, NULL) == 0, "pthread_mutex_init failed.");
            luassert_msg_always(pthread_cond_init(&out_signal.m_cond, NULL) == 0, "pthread_cond_init failed.");
            out_signal.m_manual_reset = manual_reset;
        }
        void delete_signal(Signal& sig)
        {
            pthread_cond_destroy(&sig.m_cond);
            pthread_mutex_destroy(&sig.m_mutex);
        }
        void wait_signal(Signal& sig)
        {
            luassert_msg_always(pthread_mutex_lock(&sig.m_mutex) == 0, "pthread_mutex_lock failed.");
            while (!sig.m_signaled)
            {
                luassert_msg_always(pthread_cond_wait(&sig.m_cond, &sig.m_mutex) == 0, "pthread_cond_wait failed.");
            }
            if (!sig.m_manual_reset)
            {
                sig.m_signaled = false;
            }
            luassert_msg_always(pthread_mutex_unlock(&sig.m_mutex) == 0, "pthread_mutex_unlock failed.");
        }
        bool try_wait_signal(Signal& sig)
        {
            int rc = 0;
            struct timespec abstime;
            struct timeval tv;
            gettimeofday(&tv, NULL);
            abstime.tv_sec = tv.tv_sec;
            abstime.tv_nsec = tv.tv_usec * 1000;
            if (abstime.tv_nsec >= 1000000000)
            {
                abstime.tv_nsec -= 1000000000;
                abstime.tv_sec++;
            }
            luassert_msg_always(pthread_mutex_lock(&sig.m_mutex) == 0, "pthread_mutex_lock failed.");
            while (!sig.m_signaled)
            {
                rc = pthread_cond_timedwait(&sig.m_cond, &sig.m_mutex, &abstime);
                if (rc != 0)
                {
                    break;
                }
            }
            if (rc == 0 && !sig.m_manual_reset)
            {
                sig.m_signaled = false;
            }
            luassert_msg_always(pthread_mutex_unlock(&sig.m_mutex) == 0, "pthread_mutex_unlock failed.");
            return rc == 0 ? true : false;
        }
        void trigger_signal(Signal& sig)
        {
            luassert_msg_always(pthread_mutex_lock(&sig.m_mutex) == 0, "pthread_mutex_lock failed.");
            sig.m_signaled = true;
            if (sig.m_manual_reset)
            {
                luassert_msg_always(pthread_cond_broadcast(&sig.m_cond) == 0, "pthread_cond_broadcast failed.");
            }
            else
            {
                luassert_msg_always(pthread_cond_signal(&sig.m_cond) == 0, "pthread_cond_signal failed.");
            }
            luassert_msg_always(pthread_mutex_unlock(&sig.m_mutex) == 0, "pthread_mutex_unlock failed.");
        }
        void reset_signal(Signal& sig)
        {
            luassert_msg_always(pthread_mutex_lock(&sig.m_mutex) == 0, "pthread_mutex_lock failed.");
            sig.m_signaled = false;
            luassert_msg_always(pthread_mutex_unlock(&sig.m_mutex) == 0, "pthread_mutex_unlock failed.");
        }
    }
}
