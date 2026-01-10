/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file QueryHeap.mm
* @author JXMaster
* @date 2023/8/1
*/
#include "QueryHeap.h"

namespace Luna
{
    namespace RHI
    {
        RV BufferQueryHeap::init(const QueryHeapDesc& desc)
        {
            m_desc = desc;
            m_buffer = [m_device->m_device newBufferWithLength:sizeof(u64) * desc.count options:encode_resource_options(RHI::MemoryType::readback)];
            if(!m_buffer) return BasicError::bad_platform_call();
            return ok;
        }
        void BufferQueryHeap::set_name(const c8* name)
        {
            @autoreleasepool
            {
                NSString* label = [NSString stringWithUTF8String:name];
                m_buffer.label = label;
            }
        }
        RV BufferQueryHeap::get_occlusion_values(u32 index, u32 count, u64* values)
        {
            const u64* data = (const u64*)[m_buffer contents];
            memcpy(values, data + index, count * sizeof(u64));
            return ok;
        }
        RV CounterSampleQueryHeap::init(const QueryHeapDesc& desc)
        {
            @autoreleasepool
            {
                m_desc = desc;
                NSArray<id<MTLCounterSet>>* counter_sets = [m_device->m_device counterSets];
                id<MTLCounterSet> target_counter_set = nil;
                NSString* target_set = nil;
                switch(desc.type)
                {
                    case QueryType::timestamp:
                    case QueryType::timestamp_copy_queue:
                        target_set = MTLCommonCounterSetTimestamp; break;
                    case QueryType::pipeline_statistics: 
                        target_set = MTLCommonCounterSetStatistic; break;
                    default: break;
                }
                for(u32 i = 0; i < counter_sets.count; ++i)
                {
                    id<MTLCounterSet> counter_set = counter_sets[i];
                    if([counter_set.name isEqualToString:target_set])
                    {
                        target_counter_set = counter_set;
                        break;
                    }
                }
                if(!target_counter_set) return BasicError::not_supported();
                
                MTLCounterSampleBufferDescriptor* d = [[MTLCounterSampleBufferDescriptor alloc]init];
                d.counterSet = target_counter_set;
                d.storageMode = MTLStorageModeShared;
                NSUInteger sample_count = desc.count;
                if(desc.type == QueryType::pipeline_statistics)
                {
                    // We need to sample twice for begin_query and end_query.
                    sample_count *= 2;
                }
                d.sampleCount = desc.count;
                NSError* err = nullptr;
                m_buffer = [m_device->m_device newCounterSampleBufferWithDescriptor:d error:&err];
                if(!m_buffer)
                {
                    NSString* err_desc = [err description];
                    return set_error(BasicError::bad_platform_call(), "%s", [err_desc cStringUsingEncoding:NSUTF8StringEncoding]);
                }
                return ok;
            }
        }
        RV CounterSampleQueryHeap::get_timestamp_values(u32 index, u32 count, u64* values)
        {
            @autoreleasepool
            {
                if(m_desc.type != QueryType::timestamp && m_desc.type != QueryType::timestamp_copy_queue) return BasicError::not_supported();
                NSRange range;
                range.location = index;
                range.length = count;
                NSData* data = [m_buffer resolveCounterRange:range];
                NSUInteger resolved_samples = data.length / sizeof(MTLCounterResultTimestamp);
                if(resolved_samples < count) return BasicError::bad_platform_call();
                const MTLCounterResultTimestamp* src = (const MTLCounterResultTimestamp*)data.bytes;
                for(usize i = 0; i < count; ++i)
                {
                    values[i] = src[i].timestamp;
                }
                return ok;
            }
        }
        RV CounterSampleQueryHeap::get_pipeline_statistics_values(u32 index, u32 count, PipelineStatistics* values)
        {
            @autoreleasepool
            {
                if(m_desc.type != QueryType::pipeline_statistics) return BasicError::not_supported();
                usize num_samples_per_count = sizeof(PipelineStatistics) / sizeof(u64);
                NSRange range;
                range.location = index * 2;
                range.length = count * 2;
                NSData* data = [m_buffer resolveCounterRange:range];
                NSUInteger resolved_samples = data.length / sizeof(MTLCounterResultStatistic);
                if(resolved_samples < count) return BasicError::bad_platform_call();
                const MTLCounterResultStatistic* src = (const MTLCounterResultStatistic*)data.bytes;
                for(usize i = 0; i < count; ++i)
                {
                    const MTLCounterResultStatistic& begin = src[i * 2];
                    const MTLCounterResultStatistic& end = src[i * 2 + 1];
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
}
