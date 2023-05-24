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
            if(FAILED(hr))
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
            auto result_buffer = m_device->new_buffer(MemoryType::readback, BufferDesc(BufferUsageFlag::copy_dest, query_size * desc.count));
            if (failed(result_buffer)) return result_buffer.errcode();
            m_result_buffer = result_buffer.get();
            return ok;
        }
        RV QueryHeap::get_timestamp_values(u32 index, u32 count, void* buffer, usize stride)
        {
            lutry
            {
                if (stride < sizeof(u64)) return BasicError::bad_arguments();
                usize dst_addr = (usize)buffer;
                lulet(mapped, m_result_buffer->map(index * sizeof(u64), (index + count) * sizeof(u64)));
                for (u32 i = 0; i < count; ++i)
                {
                    u64* src = ((u64*)mapped) + index + i;
                    u64* dst = (u64*)dst_addr;
                    *dst = *src;
                    dst_addr += stride;
                }
                m_result_buffer->unmap(0, 0);
            }
            lucatchret;
            return ok;
        }
        RV QueryHeap::get_occlusion_values(u32 index, u32 count, void* buffer, usize stride)
        {
            lutry
            {
                if (stride < sizeof(u64)) return BasicError::bad_arguments();
                usize dst_addr = (usize)buffer;
                lulet(mapped, m_result_buffer->map(index * sizeof(u64), (index + count) * sizeof(u64)));
                for (u32 i = 0; i < count; ++i)
                {
                    u64* src = ((u64*)mapped) + index + i;
                    u64* dst = (u64*)dst_addr;
                    *dst = *src;
                    dst_addr += stride;
                }
                m_result_buffer->unmap(0, 0);
            }
            lucatchret;
            return ok;
        }
        RV QueryHeap::get_pipeline_statistics_values(u32 index, u32 count, void* buffer, usize stride)
        {
            lutry
            {
                // Count statistics items.
                u32 num_items = 0;
                {
                    u32 flags = static_cast<u32>(m_desc.pipeline_statistics);
                    while (flags)
                    {
                        if (flags & 1) ++num_items;
                        flags >>= 1;
                    }
                }
                if (stride < sizeof(u64) * num_items) return BasicError::bad_arguments();
                usize dst_addr = (usize)buffer;
                lulet(mapped, m_result_buffer->map(index * sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS),
                    (index + count) * sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS)));
                auto flags = m_desc.pipeline_statistics;
                for (u32 i = 0; i < count; ++i)
                {
                    D3D12_QUERY_DATA_PIPELINE_STATISTICS* src = ((D3D12_QUERY_DATA_PIPELINE_STATISTICS*)mapped) + index + i;
                    u64* dst = (u64*)dst_addr;
                    if (test_flags(flags, QueryPipelineStatisticFlag::input_vertices)) { *dst = src->IAVertices; ++dst; }
                    if (test_flags(flags, QueryPipelineStatisticFlag::input_primitives)) { *dst = src->IAPrimitives; ++dst; }
                    if (test_flags(flags, QueryPipelineStatisticFlag::vs_invocations)) { *dst = src->VSInvocations; ++dst; }
                    if (test_flags(flags, QueryPipelineStatisticFlag::rasterizer_input_primitives)) { *dst = src->CInvocations; ++dst; }
                    if (test_flags(flags, QueryPipelineStatisticFlag::rendered_primitives)) { *dst = src->CPrimitives; ++dst; }
                    if (test_flags(flags, QueryPipelineStatisticFlag::ps_invocations)) { *dst = src->PSInvocations; ++dst; }
                    if (test_flags(flags, QueryPipelineStatisticFlag::cs_invocations)) { *dst = src->CSInvocations; ++dst; }
                    dst_addr += stride;
                }
                m_result_buffer->unmap(0, 0);
            }
            lucatchret;
            return ok;
        }
        RV QueryHeap::get_query_results(u32 start_index, u32 count, void* buffer, usize buffer_size, usize stride)
        {
            lutsassert();
            lutry
            {
                if (buffer_size < stride * count) return BasicError::insufficient_user_buffer();
                switch (m_desc.type)
                {
                case QueryType::occlusion:
                    luexp(get_occlusion_values(start_index, count, buffer, stride));
                    break;
                case QueryType::timestamp:
                    luexp(get_timestamp_values(start_index, count, buffer, stride));
                    break;
                case QueryType::pipeline_statistics:
                    luexp(get_pipeline_statistics_values(start_index, count, buffer, stride));
                    break;
                }
            }
            lucatchret;
            return ok;
        }
    }
}