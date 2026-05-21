/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ReadWriteLock.hpp
* @author JXMaster
* @date 2022/8/29
*/
#pragma once
#include "../ReadWriteLock.hpp"
#include "Platform/ReadWriteLock.hpp"
#include "ReadWriteLock.generated.hpp"
namespace Luna
{
    struct [[luna::struct("{CF24C77D-6022-4777-9D68-A454DA2E209F}")]] ReadWriteLock : IReadWriteLock
    {
        luiimpl();

        Platform::ReadWriteLock m_lock;

        ReadWriteLock()
        {
            Platform::new_read_write_lock(m_lock);
        }
        ~ReadWriteLock()
        {
            Platform::delete_read_write_lock(m_lock);
        }
        virtual void acquire_read() override
        {
            Platform::acquire_read_lock(m_lock);
        }
        virtual void acquire_write() override
        {
            Platform::acquire_write_lock(m_lock);
        }
        virtual bool try_acquire_read() override
        {
            return Platform::try_acquire_read_lock(m_lock);
        }
        virtual bool try_acquire_write() override
        {
            return Platform::try_acquire_write_lock(m_lock);
        }
        virtual void release_read() override
        {
            Platform::release_read_lock(m_lock);
        }
        virtual void release_write() override
        {
            Platform::release_write_lock(m_lock);
        }
    };
}