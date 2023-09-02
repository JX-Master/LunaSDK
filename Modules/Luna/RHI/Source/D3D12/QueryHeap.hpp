/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file QueryHeap.hpp
* @author JXMaster
* @date 2023/3/8
*/
#pragma once
#include "D3D12Common.hpp"
#include "Device.hpp"
#include <Luna/Runtime/TSAssert.hpp>
#include "Resource.hpp"

namespace Luna
{
    namespace RHI
    {
        struct QueryHeap : IQueryHeap
        {
            lustruct("RHI::QueryHeap", "{5e5c3f30-a388-4341-8343-1924d18793d3}");
            luiimpl();
            lutsassert_lock();

            Ref<Device> m_device;
            ComPtr<ID3D12QueryHeap> m_heap;
            // The result buffer (readback) used for fetching data from CPU side.
            Ref<BufferResource> m_result_buffer;
            QueryHeapDesc m_desc;

            RV init(const QueryHeapDesc& desc);

            virtual IDevice* get_device() override
            {
                return m_device.as<IDevice>();
            }
            virtual void set_name(const c8* name) override { set_object_name(m_heap.Get(), name); }
            virtual QueryHeapDesc get_desc() override { return m_desc; }
            virtual RV get_timestamp_values(u32 index, u32 count, u64* values) override;
            virtual RV get_occlusion_values(u32 index, u32 count, u64* values) override;
            virtual RV get_pipeline_statistics_values(u32 index, u32 count, PipelineStatistics* values) override;
        };
    }
}