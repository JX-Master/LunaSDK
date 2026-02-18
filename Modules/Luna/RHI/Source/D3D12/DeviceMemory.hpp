/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file DeviceMemory.hpp
* @author JXMaster
* @date 2023/5/16
*/
#pragma once
#include "D3D12Common.hpp"
#include "Device.hpp"

namespace Luna
{
    namespace RHI
    {
        struct DeviceMemory : IDeviceMemory
        {
            lustruct("RHI::DeviceMemory", "{070A7A5C-8C56-4F93-B13A-8E34BCFDAD67}");
            luiimpl();

            Ref<Device> m_device;
            ComPtr<D3D12MA::Allocation> m_allocation;
            MemoryType m_memory_type;

            RV init(MemoryType memory_type, const D3D12MA::ALLOCATION_DESC& allocation_desc, const D3D12_RESOURCE_ALLOCATION_INFO& allocation_info);
            ~DeviceMemory();
            virtual IDevice* get_device() override { return m_device; }
            virtual void set_name(const c8* name) override
            {
                auto buf = utf8_to_utf16_arr(name);
                m_allocation->SetName((wchar_t*)buf.data());
            }
            virtual MemoryType get_memory_type() override
            {
                return m_memory_type;
            }
            virtual u64 get_size() override
            {
                return m_allocation->GetSize();
            }
        };
    }
}