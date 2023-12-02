/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MemoryProfiler.hpp
* @author JXMaster
* @date 2023/11/3
*/
#pragma once
#include "StudioHeader.hpp"
#include <Luna/Runtime/HashSet.hpp>
#include <Luna/Runtime/HashMap.hpp>
#include <Luna/Runtime/Profiler.hpp>
#include <Luna/Runtime/SpinLock.hpp>

namespace Luna
{
    struct MemoryProfiler
    {
        struct MemoryBlockInfo
        {
            usize size;
            Name name;
            Name type;
            Name domain;
        };

        HashMap<usize, MemoryBlockInfo> m_memory_blocks;
        bool m_snapshoting = false; // Stop collecting memory info for the current thread when snapshoting.
        RecursiveSpinLock m_lock;

        void on_allocate(void* ptr, usize size);
        void on_reallocate(void* ptr, void* new_ptr, usize new_size);
        void on_deallocate(void* ptr);
        void on_set_memory_name(void* ptr, const c8* name);
        void on_set_memory_type(void* ptr, const c8* type);
        void on_set_memory_domain(void* ptr, const c8* domain);
        void render();
    };
    struct MemoryProfilerCallback
    {
        MemoryProfiler* m_profiler;

        void operator()(const ProfilerEvent& event);
    };
}