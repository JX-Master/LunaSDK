/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Mutex.hpp
* @author JXMaster
* @date 2018/12/22
* @brief Windows implementation of Sync System.
*/
#pragma once
#include "../Mutex.hpp"
#include "Platform/Mutex.hpp"
#include "MutexImpl.generated.hpp"
namespace Luna
{
    struct [[luna::struct("{0df3d468-0d98-4aee-b11d-905ad291def2}")]] Mutex : IMutex
    {
        luiimpl();

        Platform::Mutex m_data;

        Mutex()
        {
            Platform::new_mutex(m_data);
        }
        ~Mutex()
        {
            Platform::delete_mutex(m_data);
        }
        virtual void wait() override
        {
            Platform::lock_mutex(m_data);
        }
        virtual bool try_wait() override
        {
            return Platform::try_lock_mutex(m_data);
        }
        virtual void unlock() override
        {
            Platform::unlock_mutex(m_data);
        }
    };
}
