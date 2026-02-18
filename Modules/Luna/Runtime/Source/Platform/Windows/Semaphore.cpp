/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Semaphore.cpp
* @author JXMaster
* @date 2022/3/10
*/
#include "../Semaphore.hpp"
#include "../../../Assert.hpp"

namespace Luna
{
    namespace Platform
    {
        void new_semaphore(i32 initial_count, i32 max_count, Semaphore& out_sema)
        {
            out_sema.m_handle = ::CreateSemaphoreW(NULL, initial_count, max_count, NULL);
            if (!out_sema.m_handle)
            {
                lupanic_msg_always("CreateSemaphoreW failed.");
            }
        }
        void delete_semaphore(Semaphore& sema)
        {
            ::CloseHandle(sema.m_handle);
        }
        void acquire_semaphore(Semaphore& sema)
        {
            if (::WaitForSingleObject(sema.m_handle, INFINITE) != WAIT_OBJECT_0)
            {
                lupanic_always();
            }
        }
        bool try_acquire_semaphore(Semaphore& sema)
        {
            if (::WaitForSingleObject(sema.m_handle, 0) == WAIT_OBJECT_0)
            {
                return true;
            }
            return false;
        }
        void release_semaphore(Semaphore& sema)
        {
            ::ReleaseSemaphore(sema.m_handle, 1, NULL);
        }
    }
}