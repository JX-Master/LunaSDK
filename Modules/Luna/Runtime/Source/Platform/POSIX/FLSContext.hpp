/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file FLSContext.hpp
* @author JXMaster
* @date 2026/2/20
*/
#pragma once
#include "../../../Vector.hpp"
#include "../Memory.hpp"
#include "../../../SpinLock.hpp"

namespace Luna
{
    namespace Platform
    {
        struct FLSContext
        {
            Vector<void*, Platform::Allocator> m_storage;
            SpinLock m_lock;

            void* get(usize index)
            {
                LockGuard guard(m_lock);
                if(index >= m_storage.size()) return nullptr;
                return m_storage[index];
            }

            void* set(usize index, void* ptr)
            {
                LockGuard guard(m_lock);
                if(index >= m_storage.size())
                {
                    m_storage.resize(max<usize>(align_upper(index, 16), 16), nullptr);
                }
                void* ret = m_storage[index];
                m_storage[index] = ptr;
                return ret;
            }
        };
    }
}