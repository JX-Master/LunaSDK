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
                lulet(mapped, m_result_buffer->map(index * sizeof(u64), (index + count) * sizeof(u64)));
                memcpy(values, (u64*)mapped + index, count * sizeof(u64));
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
                lulet(mapped, m_result_buffer->map(index * sizeof(u64), (index + count) * sizeof(u64)));
                memcpy(values, (u64*)mapped + index, count * sizeof(u64));
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
                lulet(mapped, m_result_buffer->map(index * sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS),
                    (index + count) * sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS)));
                for (usize i = 0; i < count; ++i)
                {
                    values[i].input_vertices = ((D3D12_QUERY_DATA_PIPELINE_STATISTICS*)mapped)[index + i].IAVertices;
                    values[i].input_primitives = ((D3D12_QUERY_DATA_PIPELINE_STATISTICS*)mapped)[index + i].IAPrimitives;
                    values[i].vs_invocations = ((D3D12_QUERY_DATA_PIPELINE_STATISTICS*)mapped)[index + i].VSInvocations;
                    values[i].rasterizer_input_primitives = ((D3D12_QUERY_DATA_PIPELINE_STATISTICS*)mapped)[index + i].CInvocations;
                    values[i].rendered_primitives = ((D3D12_QUERY_DATA_PIPELINE_STATISTICS*)mapped)[index + i].CPrimitives;
                    values[i].ps_invocations = ((D3D12_QUERY_DATA_PIPELINE_STATISTICS*)mapped)[index + i].PSInvocations;
                    values[i].cs_invocations = ((D3D12_QUERY_DATA_PIPELINE_STATISTICS*)mapped)[index + i].CSInvocations;
                }
                m_result_buffer->unmap(0, 0);
            }
            lucatchret;
            return ok;
        }
    }
}