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
#include <Runtime/TSAssert.hpp>
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
            Ref<IResource> m_result_buffer;
            QueryHeapDesc m_desc;

            RV init(const QueryHeapDesc& desc);

            IDevice* get_device()
			{
				return m_device.as<IDevice>();
			}
			void set_name(const Name& name) { set_object_name(m_heap.Get(), name); }
            QueryHeapDesc get_desc() { return m_desc; }
            RV get_timestamp_values(u32 index, u32 count, u64* values);
            RV get_occlusion_values(u32 index, u32 count, u64* values);
            RV get_pipeline_statistics_values(u32 index, u32 count, PipelineStatistics* values);
        };
    }
}