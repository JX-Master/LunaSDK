/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Device.hpp
* @author JXMaster
* @date 2022/7/12
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
        struct Device : IDevice
        {
            lustruct("RHI::Device", "{89ffffe6-a1d6-413e-bb30-3e0562dacddd}");
            luiimpl();

            NSPtr<MTL::Device> m_device;
            Vector<CommandQueue> m_queues;

            RV init();

            MTL::SizeAndAlign get_buffer_size(MemoryType memory_type, const BufferDesc& desc);
            MTL::SizeAndAlign get_texture_size(MemoryType memory_type, const TextureDesc& desc);

            R<NSPtr<MTL::HeapDescriptor>> get_heap_desc(MemoryType memory_type, Span<const BufferDesc> buffers, Span<const TextureDesc> textures);

            virtual bool check_device_feature(DeviceFeature feature) override;
			virtual usize get_uniform_buffer_data_alignment() override;
            virtual void get_texture_data_placement_info(u32 width, u32 height, u32 depth, Format format,
				u64* size, u64* alignment, u64* row_pitch, u64* slice_pitch) override;
            virtual R<Ref<IBuffer>> new_buffer(MemoryType memory_type, const BufferDesc& desc) override;
			virtual R<Ref<ITexture>> new_texture(MemoryType memory_type, const TextureDesc& desc, const ClearValue* optimized_clear_value) override;
            virtual bool is_resources_aliasing_compatible(MemoryType memory_type, Span<const BufferDesc> buffers, Span<const TextureDesc> textures) override;
			virtual R<Ref<IDeviceMemory>> allocate_memory(MemoryType memory_type, Span<const BufferDesc> buffers, Span<const TextureDesc> textures) override;
			virtual R<Ref<IBuffer>> new_aliasing_buffer(IDeviceMemory* device_memory, const BufferDesc& desc) override;
			virtual R<Ref<ITexture>> new_aliasing_texture(IDeviceMemory* device_memory, const TextureDesc& desc, const ClearValue* optimized_clear_value) override;
            virtual u32 get_num_command_queues() override;
			virtual CommandQueueDesc get_command_queue_desc(u32 command_queue_index) override;
			virtual R<Ref<ICommandBuffer>> new_command_buffer(u32 command_queue_index) override;
        };

        extern Ref<IDevice> g_main_device;

        RV init_main_device();
    }
}