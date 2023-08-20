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
            switch (desc.type)
            {
            case QueryType::occlusion:
                d.Type = D3D12_QUERY_HEAP_TYPE_OCCLUSION; break;
            case QueryType::timestamp:
                d.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP; break;
            case QueryType::timestamp_copy_queue:
                d.Type = D3D12_QUERY_HEAP_TYPE_COPY_QUEUE_TIMESTAMP; break;
            case QueryType::pipeline_statistics:
                d.Type = D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS; break;
            default: lupanic(); break;
            }
            d.Count = desc.count;
            d.NodeMask = 0;
            HRESULT hr = m_device->m_device->CreateQueryHeap(&d,
                IID_PPV_ARGS(&m_heap));
            if (FAILED(hr))
            {
                return encode_hresult(hr);
            }
            // Create resource buffer.
            usize query_size = 0;
            switch (desc.type)
            {
            case QueryType::occlusion: query_size = 8; break;
            case QueryType::timestamp: query_size = 8; break;
            case QueryType::pipeline_statistics: query_size = sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS); break;
            default: lupanic(); break;
            }
            auto result_buffer = m_device->new_buffer(MemoryType::readback, BufferDesc(BufferUsageFlag::none, query_size * desc.count));
            if (failed(result_buffer)) return result_buffer.errcode();
            m_result_buffer = result_buffer.get();
            return ok;
        }
        RV QueryHeap::get_timestamp_values(u32 index, u32 count, u64* values)
        {
            lutsassert();
            if (m_desc.type != QueryType::timestamp) return BasicError::not_supported();
            lutry
            {
                u64* mapped = nullptr;
                luexp(m_result_buffer->map(index * sizeof(u64), (index + count) * sizeof(u64), (void**)&mapped));
                memcpy(values, mapped + index, count * sizeof(u64));
                m_result_buffer->unmap(0, 0);
            }
            lucatchret;
            return ok;
        }
        RV QueryHeap::get_occlusion_values(u32 index, u32 count, u64* values)
        {
            lutsassert();
            if (m_desc.type != QueryType::occlusion) return BasicError::not_supported();
            lutry
            {
                u64* mapped = nullptr;
                luexp(m_result_buffer->map(index * sizeof(u64), (index + count) * sizeof(u64), (void**)&mapped));
                memcpy(values, mapped + index, count * sizeof(u64));
                m_result_buffer->unmap(0, 0);
            }
            lucatchret;
            return ok;
        }
        RV QueryHeap::get_pipeline_statistics_values(u32 index, u32 count, PipelineStatistics* values)
        {
            lutsassert();
            if (m_desc.type != QueryType::pipeline_statistics) return BasicError::not_supported();
            lutry
            {
                D3D12_QUERY_DATA_PIPELINE_STATISTICS* mapped = nullptr;
                luexp(m_result_buffer->map(index * sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS),
                    (index + count) * sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS), (void**)&mapped));
                for (usize i = 0; i < count; ++i)
                {
                    auto& dst = values[i];
                    auto& src = mapped[index + i];
                    dst.vs_invocations = src.VSInvocations;
                    dst.rasterizer_input_primitives = src.CInvocations;
                    dst.rendered_primitives = src.CPrimitives;
                    dst.ps_invocations = src.PSInvocations;
                    dst.cs_invocations = src.CSInvocations;
                }
                m_result_buffer->unmap(0, 0);
            }
            lucatchret;
            return ok;
        }
    }
}