/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file QueryHeap.cpp
* @author JXMaster
* @date 2023/3/8
*/
#include "QueryHeap.hpp"
namespace Luna
{
    namespace RHI
    {
        RV QueryHeap::init(const QueryHeapDesc& desc)
        {
            m_desc = desc;
            D3D12_QUERY_HEAP_DESC d;
            switch(desc.type)
            {
            case QueryHeapType::occlusion:
                d.Type = D3D12_QUERY_HEAP_TYPE_OCCLUSION; break;
            case QueryHeapType::timestamp:
                d.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP; break;
            case QueryHeapType::pipeline_statistics:
                d.Type = D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS; break;
            default: lupanic(); break;
            }
            d.Count = desc.count;
            d.NodeMask = 0;
            HRESULT hr = m_device->m_device->CreateQueryHeap(&d, 
                IID_PPV_ARGS(&m_heap));
            if(FAILED(hr))
            {
                return encode_d3d12_error(hr);
            }
            // Create resource buffer.
            usize query_size = 0;
            switch (desc.type)
            {
            case QueryHeapType::occlusion: query_size = 8; break;
            case QueryHeapType::timestamp: query_size = 8; break;
            case QueryHeapType::pipeline_statistics: query_size = sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS); break;
            default: lupanic(); break;
            }
            auto result_buffer = m_device->new_resource(ResourceDesc::buffer(ResourceHeapType::readback, ResourceUsageFlag::none,
                query_size * desc.count), nullptr);
            if (failed(result_buffer)) return result_buffer.errcode();
            m_result_buffer = result_buffer.get();
            return ok;
        }
        RV QueryHeap::get_timestamp_values(u32 index, u32 count, u64* values)
        {
            lutsassert();
            if (m_desc.type != QueryHeapType::timestamp) return BasicError::not_supported();
            lutry
            {
                u64* mapped = nullptr;
                luexp(m_result_buffer->map_subresource(0, index * sizeof(u64), (index + count) * sizeof(u64), (void**)&mapped));
                memcpy(values, mapped + index, count * sizeof(u64));
                m_result_buffer->unmap_subresource(0, 0, 0);
            }
            lucatchret;
            return ok;
        }
        RV QueryHeap::get_occlusion_values(u32 index, u32 count, u64* values)
        {
            lutsassert();
            if (m_desc.type != QueryHeapType::occlusion) return BasicError::not_supported();
            lutry
            {
                u64* mapped = nullptr;
                luexp(m_result_buffer->map_subresource(0, index * sizeof(u64), (index + count) * sizeof(u64), (void**)&mapped));
                memcpy(values, mapped + index, count * sizeof(u64));
                m_result_buffer->unmap_subresource(0, 0, 0);
            }
            lucatchret;
            return ok;
        }
        RV QueryHeap::get_pipeline_statistics_values(u32 index, u32 count, PipelineStatistics* values)
        {
            lutsassert();
            if (m_desc.type != QueryHeapType::occlusion) return BasicError::not_supported();
            lutry
            {
                D3D12_QUERY_DATA_PIPELINE_STATISTICS* mapped = nullptr;
                luexp(m_result_buffer->map_subresource(0, index * sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS), 
                    (index + count) * sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS), (void**)&mapped));
                for (usize i = 0; i < count; ++i)
                {
                    values[i].input_vertices = mapped[index + i].IAVertices;
                    values[i].input_primitives = mapped[index + i].IAPrimitives;
                    values[i].vs_invocations = mapped[index + i].VSInvocations;
                    values[i].gs_invocations = mapped[index + i].GSInvocations;
                    values[i].gs_output_primitives = mapped[index + i].GSPrimitives;
                    values[i].rasterizer_input_primitives = mapped[index + i].CInvocations;
                    values[i].rendered_primitives = mapped[index + i].CPrimitives;
                    values[i].ps_invocations = mapped[index + i].PSInvocations;
                    values[i].hs_invocations = mapped[index + i].HSInvocations;
                    values[i].ds_invocations = mapped[index + i].DSInvocations;
                    values[i].cs_invocations = mapped[index + i].CSInvocations;
                }
                m_result_buffer->unmap_subresource(0, 0, 0);
            }
            lucatchret;
            return ok;
        }
    }
}