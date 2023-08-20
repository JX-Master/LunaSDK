/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file QueryHeap.hpp
* @author JXMaster
* @date 2023/8/1
*/
#pragma once
#include "Device.hpp"
namespace Luna
{
    namespace RHI
    {
        struct BufferQueryHeap : IQueryHeap
        {
            lustruct("RHI::BufferQueryHeap", "{5e568a4a-b522-441b-b421-efe46777d725}");
            luiimpl();

            Ref<Device> m_device;
            NSPtr<MTL::Buffer> m_buffer;
            QueryHeapDesc m_desc;

            RV init(const QueryHeapDesc& desc);

            virtual IDevice* get_device() override { return m_device; }
            virtual void set_name(const c8* name) override  { set_object_name(m_buffer.get(), name); }
            virtual QueryHeapDesc get_desc() override { return m_desc; }
            virtual RV get_timestamp_values(u32 index, u32 count, u64* values) override { return BasicError::not_supported(); }
            virtual RV get_occlusion_values(u32 index, u32 count, u64* values) override;
            virtual RV get_pipeline_statistics_values(u32 index, u32 count, PipelineStatistics* values) override { return BasicError::not_supported(); }
        };

        struct CounterSampleQueryHeap : IQueryHeap
        {
            lustruct("RHI::CounterSampleQueryHeap", "{2660efc3-0198-45e6-b2c0-1f6539f82ccd}");
            luiimpl();

            Ref<Device> m_device;
            NSPtr<MTL::CounterSampleBuffer> m_buffer;
            QueryHeapDesc m_desc;

            RV init(const QueryHeapDesc& desc);

            virtual IDevice* get_device() override { return m_device; }
            virtual void set_name(const c8* name) override  { }
            virtual QueryHeapDesc get_desc() override { return m_desc; }
            virtual RV get_timestamp_values(u32 index, u32 count, u64* values) override;
            virtual RV get_occlusion_values(u32 index, u32 count, u64* values) override { return BasicError::not_supported(); }
            virtual RV get_pipeline_statistics_values(u32 index, u32 count, PipelineStatistics* values) override;
        };
    }
}