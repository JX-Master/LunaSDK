/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DeviceMemory.hpp
* @author JXMaster
* @date 2023/7/12
*/
#pragma once
#include "Device.hpp"

namespace Luna
{
    namespace RHI
    {
        struct DeviceMemory : IDeviceMemory
        {
            lustruct("RHI::DeviceMemory", "{e12753df-c132-46f8-9e94-b259c1cecb2b}");
            luiimpl();

            Ref<Device> m_device;
            NSPtr<MTL::Heap> m_heap; // This may be `nullptr`, which represents one non-sharable memory.
            MemoryType m_memory_type;
            u64 m_size;

            RV init(MTL::HeapDescriptor* desc);
            ~DeviceMemory();

            virtual IDevice* get_device() override { return m_device; }
            virtual void set_name(const c8* name) override  { set_object_name(m_heap.get(), name); }
            virtual MemoryType get_memory_type() override { return m_memory_type; }
			virtual u64 get_size() override { return m_size; }
        };
    }
}