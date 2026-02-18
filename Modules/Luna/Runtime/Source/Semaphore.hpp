/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Semaphore.hpp
* @author JXMaster
* @date 2018/12/22
* @brief Windows implementation of Sync System.
*/
#pragma once
#include "../Semaphore.hpp"
#include "Platform/Semaphore.hpp"

namespace Luna
{
    struct Semaphore : ISemaphore
    {
        lustruct("Semaphore", "{4d155da3-acdb-4ac6-aecb-70e43a5faedf}");
        luiimpl();

        Platform::Semaphore m_data;

        Semaphore(i32 initial_count, i32 max_count)
        {
            Platform::new_semaphore(initial_count, max_count, m_data);
        }
        ~Semaphore()
        {
            Platform::delete_semaphore(m_data);
        }
        virtual void wait() override
        {
            Platform::acquire_semaphore(m_data);
        }
        virtual bool try_wait() override
        {
            return Platform::try_acquire_semaphore(m_data);
        }
        virtual void release() override
        {
            Platform::release_semaphore(m_data);
        }
    };
}
