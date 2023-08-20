/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file QueryHeap.cpp
* @author JXMaster
* @date 2023/8/1
*/
#include "QueryHeap.hpp"

namespace Luna
{
    namespace RHI
    {
        RV BufferQueryHeap::init(const QueryHeapDesc& desc)
        {
            m_desc = desc;
            m_buffer = box(m_device->m_device->newBuffer(sizeof(u64) * desc.count, encode_resource_options(RHI::MemoryType::readback)));
            if(!m_buffer) return BasicError::bad_platform_call();
        }
        RV BufferQueryHeap::get_occlusion_values(u32 index, u32 count, u64* values)
        {
            const u64* data = (const u64*)m_buffer->contents();
            memcpy(values, data + index, count * sizeof(u64));
        }
        RV CounterSampleQueryHeap::init(const QueryHeapDesc& desc)
        {
            m_desc = desc;
            AutoreleasePool pool;
            NS::Array* counter_sets = m_device->m_device->counterSets();
            MTL::CounterSet* target_counter_set = nullptr;
            MTL::CommonCounterSet target_set;
            switch(desc.type)
            {
                case QueryType::timestamp:
                case QueryType::timestamp_copy_queue:
                    target_set = MTL::CommonCounterSetTimestamp; break;
                case QueryType::pipeline_statistics: target_set = MTL::CommonCounterSetStatistic; break;
                default: break;
            }
            for(u32 i = 0; i < counter_sets->count(); ++i)
            {
                MTL::CounterSet* counter_set = counter_sets->object<MTL::CounterSet>(i);
                if(counter_set->name()->isEqualToString(target_set))
                {
                    target_counter_set = counter_set;
                    break;
                }
            }
            if(!target_counter_set) return BasicError::not_supported();
            
            NSPtr<MTL::CounterSampleBufferDescriptor> d = box(MTL::CounterSampleBufferDescriptor::alloc()->init());
            d->setCounterSet(target_counter_set);
            d->setStorageMode(MTL::StorageModeShared);
            NS::UInteger sample_count = desc.count;
            if(desc.type == QueryType::pipeline_statistics)
            {
                // We need to sample twice for begin_query and end_query.
                sample_count *= 2;
            }
            d->setSampleCount(desc.count);
            NS::Error* err = nullptr;
            m_buffer = box(m_device->m_device->newCounterSampleBuffer(d.get(), &err));
            if(!m_buffer)
            {
                NS::String* err_desc = err->description();
                return set_error(BasicError::bad_platform_call(), "%s", err_desc->cString(NS::UTF8StringEncoding));
            }
            return ok;
        }
        RV CounterSampleQueryHeap::get_timestamp_values(u32 index, u32 count, u64* values)
        {
            if(m_desc.type != QueryType::timestamp && m_desc.type != QueryType::timestamp_copy_queue) return BasicError::not_supported();
            AutoreleasePool pool;
            NS::Data* data = m_buffer->resolveCounterRange(NS::Range::Make(index, count));
            NS::UInteger resolved_samples = data->length() / sizeof(MTL::CounterResultTimestamp);
            if(resolved_samples < count) return BasicError::bad_platform_call();
            MTL::CounterResultTimestamp* src = (MTL::CounterResultTimestamp*)data->mutableBytes();
            for(usize i = 0; i < count; ++i)
            {
                values[i] = src[i].timestamp;
            }
            return ok;
        }
        RV CounterSampleQueryHeap::get_pipeline_statistics_values(u32 index, u32 count, PipelineStatistics* values)
        {
            if(m_desc.type != QueryType::pipeline_statistics) return BasicError::not_supported();
            AutoreleasePool pool;
            usize num_samples_per_count = sizeof(PipelineStatistics) / sizeof(u64);
            NS::Data* data = m_buffer->resolveCounterRange(NS::Range::Make(index * 2, count * 2));
            NS::UInteger resolved_samples = data->length() / sizeof(MTL::CounterResultStatistic);
            if(resolved_samples < count) return BasicError::bad_platform_call();
            MTL::CounterResultStatistic* src = (MTL::CounterResultStatistic*)data->mutableBytes();
            for(usize i = 0; i < count; ++i)
            {
                const MTL::CounterResultStatistic& begin = src[i * 2];
                const MTL::CounterResultStatistic& end = src[i * 2 + 1];
                PipelineStatistics& dest = values[i];
                dest.vs_invocations = end.vertexInvocations - begin.vertexInvocations;
                dest.rasterizer_input_primitives = end.clipperInvocations - begin.clipperInvocations;
                dest.rendered_primitives = end.clipperPrimitivesOut - begin.clipperPrimitivesOut;
                dest.ps_invocations = end.fragmentInvocations - begin.fragmentInvocations;
                dest.cs_invocations = end.computeKernelInvocations - begin.computeKernelInvocations;
            }
            return ok;
        }
    }
}
