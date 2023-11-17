/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Device.hpp
* @author JXMaster
* @date 2023/7/12
*/
#pragma once
#include "Common.hpp"
#include "../../Device.hpp"
namespace Luna
{
    namespace RHI
    {
        struct CommandQueue
        {
            NSPtr<MTL::CommandQueue> queue;
            CommandQueueDesc desc;

            RV init(MTL::Device* dev, const CommandQueueDesc& desc);
        };
        enum class CounterSamplingSupportFlag : u8
        {
            none = 0x00,
            stage = 0x01,
            draw = 0x02,
            blit = 0x04,
            dispatch = 0x08,
        };
        struct Device : IDevice
        {
            lustruct("RHI::Device", "{89ffffe6-a1d6-413e-bb30-3e0562dacddd}");
            luiimpl();

            NSPtr<MTL::Device> m_device;
            Vector<CommandQueue> m_queues;

            MTL::Timestamp m_start_cpu_time;
            MTL::Timestamp m_start_gpu_time;
            f64 m_timestamp_frequency = 0.0;
            
            CounterSamplingSupportFlag m_counter_sampling_support_flags = CounterSamplingSupportFlag::none;
            bool m_support_metal_3_family;

            RV init();

            MTL::SizeAndAlign get_buffer_size(MemoryType memory_type, const BufferDesc& desc);
            MTL::SizeAndAlign get_texture_size(MemoryType memory_type, const TextureDesc& desc);

            R<NSPtr<MTL::HeapDescriptor>> get_heap_desc(MemoryType memory_type, Span<const BufferDesc> buffers, Span<const TextureDesc> textures);

            virtual DeviceFeatureData check_feature(DeviceFeature feature) override;
            virtual void get_texture_data_placement_info(u32 width, u32 height, u32 depth, Format format,
				u64* size, u64* alignment, u64* row_pitch, u64* slice_pitch) override;
            virtual R<Ref<IBuffer>> new_buffer(MemoryType memory_type, const BufferDesc& desc) override;
			virtual R<Ref<ITexture>> new_texture(MemoryType memory_type, const TextureDesc& desc, const ClearValue* optimized_clear_value) override;
            virtual bool is_resources_aliasing_compatible(MemoryType memory_type, Span<const BufferDesc> buffers, Span<const TextureDesc> textures) override;
			virtual R<Ref<IDeviceMemory>> allocate_memory(MemoryType memory_type, Span<const BufferDesc> buffers, Span<const TextureDesc> textures) override;
			virtual R<Ref<IBuffer>> new_aliasing_buffer(IDeviceMemory* device_memory, const BufferDesc& desc) override;
			virtual R<Ref<ITexture>> new_aliasing_texture(IDeviceMemory* device_memory, const TextureDesc& desc, const ClearValue* optimized_clear_value) override;
            virtual R<Ref<IPipelineLayout>> new_pipeline_layout(const PipelineLayoutDesc& desc) override;
            virtual R<Ref<IPipelineState>> new_graphics_pipeline_state(const GraphicsPipelineStateDesc& desc) override;
            virtual R<Ref<IPipelineState>> new_compute_pipeline_state(const ComputePipelineStateDesc& desc) override;
            virtual R<Ref<IDescriptorSetLayout>> new_descriptor_set_layout(const DescriptorSetLayoutDesc& desc) override;
            virtual R<Ref<IDescriptorSet>> new_descriptor_set(const DescriptorSetDesc& desc) override;
            virtual u32 get_num_command_queues() override;
			virtual CommandQueueDesc get_command_queue_desc(u32 command_queue_index) override;
			virtual R<Ref<ICommandBuffer>> new_command_buffer(u32 command_queue_index) override;
            virtual R<f64> get_command_queue_timestamp_frequency(u32 command_queue_index) override;
            virtual R<Ref<IQueryHeap>> new_query_heap(const QueryHeapDesc& desc) override;
            virtual R<Ref<IFence>> new_fence() override;
            virtual R<Ref<ISwapChain>> new_swap_chain(u32 command_queue_index, Window::IWindow* window, const SwapChainDesc& desc) override;
        };

        extern Ref<IDevice> g_main_device;

        RV init_main_device();
    }
}
