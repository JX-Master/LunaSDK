/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DeviceMemory.mm
* @author JXMaster
* @date 2023/7/12
*/
#include "DeviceMemory.h"
#include <Luna/Runtime/Profiler.hpp>

namespace Luna
{
    namespace RHI
    {
        RV DeviceMemory::init(MTLHeapDescriptor* desc)
        {
            lutry
            {
                m_heap = [m_device->m_device newHeapWithDescriptor:desc];
                if(!m_heap) return BasicError::bad_platform_call();
                m_size = m_heap.size;
#ifdef LUNA_MEMORY_PROFILER_ENABLED
                memory_profiler_allocate((__bridge void*)m_heap, m_size);
                memory_profiler_set_memory_domain((__bridge void*)m_heap, "GPU", 3);
                memory_profiler_set_memory_type((__bridge void*)m_heap, "Aliasing Memory", 15);
#endif
            }
            lucatchret;
            return ok;
        }
        DeviceMemory::~DeviceMemory()
        {
#ifdef LUNA_MEMORY_PROFILER_ENABLED
            if(m_heap) memory_profiler_deallocate((__bridge void*)m_heap);
#endif
            m_heap = nil;
        }
        void DeviceMemory::set_name(const c8* name)
        {
            @autoreleasepool
            {
                NSString* label = [NSString stringWithUTF8String:name];
                m_heap.label = label;
            }
        }
    }
}