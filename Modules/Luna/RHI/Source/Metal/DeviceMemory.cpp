/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DeviceMemory.cpp
* @author JXMaster
* @date 2023/7/12
*/
#include "DeviceMemory.hpp"

namespace Luna
{
    namespace RHI
    {
        RV DeviceMemory::init(MTL::HeapDescriptor* desc)
        {
            lutry
            {
                m_heap = box(m_device->m_device->newHeap(desc));
                if(!m_heap) return BasicError::bad_platform_call();
                m_size = m_heap->size();
#ifdef LUNA_MEMORY_PROFILER_ENABLED
                memory_profiler_allocate(m_heap.get(), m_size);
                memory_profiler_set_memory_domain(m_heap.get(), "GPU", 3);
                memory_profiler_set_memory_type(m_heap.get(), "Aliasing Memory", 15);
#endif
            }
            lucatchret;
            return ok;
        }
        DeviceMemory::~DeviceMemory()
        {
#ifdef LUNA_MEMORY_PROFILER_ENABLED
            if(m_heap) memory_profiler_deallocate(m_heap.get());
#endif
        }
    }
}