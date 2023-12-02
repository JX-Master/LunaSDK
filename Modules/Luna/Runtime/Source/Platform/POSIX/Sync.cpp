/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Sync.cpp
* @author JXMaster
* @date 2020/9/22
 */
#include "../../OS.hpp"
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include "../../../Assert.hpp"
#include "../../../Atomic.hpp"

namespace Luna
{
    namespace OS
    {
        struct Signal
        {
            pthread_cond_t m_cond;
            pthread_mutex_t m_mutex;
            volatile bool m_signaled;
            volatile bool m_manual_reset;

            Signal()
            {
                m_signaled = false;
                luassert_msg_always(pthread_mutex_init(&m_mutex, NULL) == 0, "pthread_mutex_init failed.");
                luassert_msg_always(pthread_cond_init(&m_cond, NULL) == 0, "pthread_cond_init failed.");
            }
            ~Signal()
            {
                pthread_cond_destroy(&m_cond);
                pthread_mutex_destroy(&m_mutex);
            }
        };
        opaque_t new_signal(bool manual_reset)
        {
            Signal* sig = memnew<Signal>();
            sig->m_manual_reset = manual_reset;
            return sig;
        }
        void delete_signal(opaque_t sig)
        {
            Signal* o = (Signal*)sig;
            memdelete(o);
        }
        void wait_signal(opaque_t sig)
        {
            Signal* o = (Signal*)sig;
            luassert_msg_always(pthread_mutex_lock(&o->m_mutex) == 0, "pthread_mutex_lock failed.");
            while (!o->m_signaled)
            {
                luassert_msg_always(pthread_cond_wait(&o->m_cond, &o->m_mutex) == 0, "pthread_cond_wait failed.");
            }
            if (!o->m_manual_reset)
            {
                o->m_signaled = false;
            }
            luassert_msg_always(pthread_mutex_unlock(&o->m_mutex) == 0, "pthread_mutex_unlock failed.");
        }
        bool try_wait_signal(opaque_t sig)
        {
            Signal* o = (Signal*)sig;
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
            luassert_msg_always(pthread_mutex_lock(&o->m_mutex) == 0, "pthread_mutex_lock failed.");
            while (!o->m_signaled)
            {
                rc = pthread_cond_timedwait(&o->m_cond, &o->m_mutex, &abstime);
                if (rc != 0)
                {
                    break;
                }
            }
            if (rc == 0 && !o->m_manual_reset)
            {
                o->m_signaled = false;
            }
            luassert_msg_always(pthread_mutex_unlock(&o->m_mutex) == 0, "pthread_mutex_unlock failed.");
            return rc == 0 ? true : false;
        }
        void trigger_signal(opaque_t sig)
        {
            Signal* o = (Signal*)sig;
            luassert_msg_always(pthread_mutex_lock(&o->m_mutex) == 0, "pthread_mutex_lock failed.");
            o->m_signaled = true;
            if (o->m_manual_reset)
            {
                luassert_msg_always(pthread_cond_broadcast(&o->m_cond) == 0, "pthread_cond_broadcast failed.");
            }
            else
            {
                luassert_msg_always(pthread_cond_signal(&o->m_cond) == 0, "pthread_cond_signal failed.");
            }
            luassert_msg_always(pthread_mutex_unlock(&o->m_mutex) == 0, "pthread_mutex_unlock failed.");
        }
        void reset_signal(opaque_t sig)
        {
            Signal* o = (Signal*)sig;
            luassert_msg_always(pthread_mutex_lock(&o->m_mutex) == 0, "pthread_mutex_lock failed.");
            o->m_signaled = false;
            luassert_msg_always(pthread_mutex_unlock(&o->m_mutex) == 0, "pthread_mutex_unlock failed.");
        }
        opaque_t new_mutex()
        {
            pthread_mutex_t* mtx = memnew<pthread_mutex_t>();
            pthread_mutexattr_t attr;
            luassert_msg_always(pthread_mutexattr_init(&attr) == 0, "pthread_mutexattr_init failed.");
            luassert_msg_always(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) == 0, "pthread_mutexattr_settype failed.");
            luassert_msg_always(pthread_mutex_init(mtx, &attr) == 0, "pthread_mutex_init failed.");
            pthread_mutexattr_destroy(&attr);
            return mtx;
        }
        void delete_mutex(opaque_t mtx)
        {
            pthread_mutex_t* o = (pthread_mutex_t*)mtx;
            pthread_mutex_destroy(o);
            memdelete(o);
        }
        void lock_mutex(opaque_t mtx)
        {
            pthread_mutex_t* o = (pthread_mutex_t*)mtx;
            luassert_msg_always(pthread_mutex_lock(o) == 0, "pthread_mutex_lock failed.");
        }
        bool try_lock_mutex(opaque_t mtx)
        {
            pthread_mutex_t* o = (pthread_mutex_t*)mtx;
            int rv = pthread_mutex_trylock(o);
            return (rv == 0) ? true : false;
        }
        void unlock_mutex(opaque_t mtx)
        {
            pthread_mutex_t* o = (pthread_mutex_t*)mtx;
            luassert_msg_always(pthread_mutex_unlock(o) == 0, "pthread_mutex_unlock failed.");
        }
        struct Semaphore
        {
            pthread_mutex_t m_mutex;
            pthread_cond_t m_cond;
            i32 m_counter;
            i32 m_max_count;

            Semaphore(i32 initial_count, i32 max_count)
            {
                m_max_count = max_count;
                m_counter = initial_count;
                luassert_msg_always(pthread_mutex_init(&m_mutex, NULL) == 0, "pthread_mutex_init failed.");
                luassert_msg_always(pthread_cond_init(&m_cond, NULL) == 0, "pthread_cond_init failed.");
            }
            ~Semaphore()
            {
                pthread_cond_destroy(&m_cond);
                pthread_mutex_destroy(&m_mutex);
            }
        };
        opaque_t new_semaphore(i32 initial_count, i32 max_count)
        {
            Semaphore* sema = memnew<Semaphore>(initial_count, max_count);
            return sema;
        }
        void delete_semaphore(opaque_t sema)
        {
            Semaphore* o = (Semaphore*)sema;
            memdelete(o);
        }
        void acquire_semaphore(opaque_t sema)
        {
            Semaphore* o = (Semaphore*)sema;
            luassert_msg_always(pthread_mutex_lock(&o->m_mutex) == 0, "pthread_mutex_lock failed.");
            if (o->m_counter > 0)
            {
                atom_dec_i32(&o->m_counter);
                luassert_msg_always(pthread_mutex_unlock(&o->m_mutex) == 0, "pthread_mutex_unlock failed.");
            }
            atom_dec_i32(&o->m_counter);
            luassert_msg_always(pthread_cond_wait(&o->m_cond, &o->m_mutex) == 0, "pthread_cond_wait failed.");
            atom_inc_i32(&o->m_counter);
            luassert_msg_always(pthread_mutex_unlock(&o->m_mutex) == 0, "pthread_mutex_unlock failed.");
        }
        bool try_acquire_semaphore(opaque_t sema)
        {
            Semaphore* o = (Semaphore*)sema;
            luassert_msg_always(pthread_mutex_lock(&o->m_mutex) == 0, "pthread_mutex_lock failed.");
            if (o->m_counter > 0)
            {
                atom_dec_i32(&o->m_counter);
                luassert_msg_always(pthread_mutex_unlock(&o->m_mutex) == 0, "pthread_mutex_unlock failed.");
                return true;
            }
            luassert_msg_always(pthread_mutex_unlock(&o->m_mutex) == 0, "pthread_mutex_unlock failed.");
            return false;
        }
        void release_semaphore(opaque_t sema)
        {
            Semaphore* o = (Semaphore*)sema;
            luassert_msg_always(pthread_mutex_lock(&o->m_mutex) == 0, "pthread_mutex_lock failed.");
            bool ret = false;
            if (o->m_counter < o->m_max_count)
            {
                if (o->m_counter < 0)
                {
                    luassert_msg_always(pthread_cond_signal(&o->m_cond) == 0, "pthread_cond_signal failed.");
                }
                else
                {
                    atom_inc_i32(&o->m_counter);
                }
            }
            luassert_msg_always(pthread_mutex_unlock(&o->m_mutex) == 0, "pthread_mutex_unlock failed.");
        }
        opaque_t new_read_write_lock()
        {
            pthread_rwlock_t* o = memnew<pthread_rwlock_t>();
            auto ret = pthread_rwlock_init(o, nullptr);
            luassert_msg_always(ret == 0, "pthread_rwlock_init failed.");
            return o;
        }
		void delete_read_write_lock(opaque_t lock)
        {
            pthread_rwlock_t* o = (pthread_rwlock_t*)lock;
            auto ret = pthread_rwlock_destroy(o);
            luassert_msg_always(ret == 0, "pthread_rwlock_destroy failed.");
            memdelete(o);
        }
		void acquire_read_lock(opaque_t lock)
        {
            pthread_rwlock_t* o = (pthread_rwlock_t*)lock;
            auto ret = pthread_rwlock_rdlock(o);
            luassert_msg_always(ret == 0, "pthread_rwlock_rdlock failed.");
        }
		void acquire_write_lock(opaque_t lock)
        {
            pthread_rwlock_t* o = (pthread_rwlock_t*)lock;
            auto ret = pthread_rwlock_wrlock(o);
            luassert_msg_always(ret == 0, "pthread_rwlock_wrlock failed.");
        }
		bool try_acquire_read_lock(opaque_t lock)
        {
            pthread_rwlock_t* o = (pthread_rwlock_t*)lock;
            auto ret = pthread_rwlock_tryrdlock(o);
            return ret == 0 ? true : false;
        }
		bool try_acquire_write_lock(opaque_t lock)
        {
            pthread_rwlock_t* o = (pthread_rwlock_t*)lock;
            auto ret = pthread_rwlock_trywrlock(o);
            return ret == 0 ? true : false;
        }
		void release_read_lock(opaque_t lock)
        {
            pthread_rwlock_t* o = (pthread_rwlock_t*)lock;
            pthread_rwlock_unlock(o);
        }
		void release_write_lock(opaque_t lock)
        {
            pthread_rwlock_t* o = (pthread_rwlock_t*)lock;
            pthread_rwlock_unlock(o);
        }
    }
}