/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DeviceMemory.cpp
* @author JXMaster
* @date 2022/7/12
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
            }
            lucatchret;
            return ok;
        }
    }
}