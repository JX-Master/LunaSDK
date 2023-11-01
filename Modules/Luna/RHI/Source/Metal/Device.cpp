/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Device.cpp
* @author JXMaster
* @date 2023/7/12
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RHI_API LUNA_EXPORT
#include "Device.hpp"
#include "Adapter.hpp"
#include "DeviceMemory.hpp"
#include "Resource.hpp"
#include "CommandBuffer.hpp"
#include "PipelineLayout.hpp"
#include "PipelineState.hpp"
#include "DescriptorSet.hpp"
#include "QueryHeap.hpp"
#include "Fence.hpp"
#include "SwapChain.hpp"
namespace Luna
{
    namespace RHI
    {
        RV CommandQueue::init(MTL::Device* dev, const CommandQueueDesc& desc)
        {
            this->desc = desc;
            this->queue = box(dev->newCommandQueue());
            if(!this->queue) return BasicError::bad_platform_call();
            return ok;
        }
        RV Device::init()
        {
            lutry
            {
                AutoreleasePool pool;
                m_device->sampleTimestamps(&m_start_cpu_time, &m_start_gpu_time);
                CommandQueueDesc desc;
                desc.type = CommandQueueType::graphics;
                desc.flags = CommandQueueFlag::presenting;
                CommandQueue queue;
                luexp(queue.init(m_device.get(), desc));
                set_object_name(queue.queue.get(), "Render Queue");
                m_queues.push_back(move(queue));
                desc.type = CommandQueueType::compute;
                desc.flags = CommandQueueFlag::none;
                luexp(queue.init(m_device.get(), desc));
                set_object_name(queue.queue.get(), "Compute Queue");
                m_queues.push_back(move(queue));
                luexp(queue.init(m_device.get(), desc));
                set_object_name(queue.queue.get(), "Compute Queue");
                m_queues.push_back(move(queue));
                desc.type = CommandQueueType::copy;
                luexp(queue.init(m_device.get(), desc));
                set_object_name(queue.queue.get(), "Blit Queue");
                m_queues.push_back(move(queue));
                luexp(queue.init(m_device.get(), desc));
                set_object_name(queue.queue.get(), "Blit Queue");
                m_queues.push_back(move(queue));
                if(m_device->supportsCounterSampling(MTL::CounterSamplingPointAtStageBoundary))
                {
                    m_counter_sampling_support_flags |= CounterSamplingSupportFlag::stage;
                }
                if(m_device->supportsCounterSampling(MTL::CounterSamplingPointAtDrawBoundary))
                {
                    m_counter_sampling_support_flags |= CounterSamplingSupportFlag::draw;
                }
                if(m_device->supportsCounterSampling(MTL::CounterSamplingPointAtBlitBoundary))
                {
                    m_counter_sampling_support_flags |= CounterSamplingSupportFlag::blit;
                }
                if(m_device->supportsCounterSampling(MTL::CounterSamplingPointAtDispatchBoundary))
                {
                    m_counter_sampling_support_flags |= CounterSamplingSupportFlag::dispatch;
                }
                m_support_metal_3_family = m_device->supportsFamily(MTL::GPUFamilyMetal3);
            }
            lucatchret;
            return ok;
        }
        MTL::SizeAndAlign Device::get_buffer_size(MemoryType memory_type, const BufferDesc& desc)
        {
            return m_device->heapBufferSizeAndAlign(desc.size, encode_resource_options(memory_type));
        }
        MTL::SizeAndAlign Device::get_texture_size(MemoryType memory_type, const TextureDesc& desc)
        {
            auto tex_desc = encode_texture_desc(memory_type, desc);
            return m_device->heapTextureSizeAndAlign(tex_desc.get());
        }
        R<NSPtr<MTL::HeapDescriptor>> Device::get_heap_desc(MemoryType memory_type, Span<const BufferDesc> buffers, Span<const TextureDesc> textures)
        {
            if(memory_type != MemoryType::local && !textures.empty())
            {
                return set_error(BasicError::not_supported(), "Textures cannot be created in upload or readback heaps.");
            }
            NSPtr<MTL::HeapDescriptor> ret = box(MTL::HeapDescriptor::alloc()->init());
            ret->setType(MTL::HeapTypeAutomatic);
            ret->setStorageMode(encode_storage_mode(memory_type));
            ret->setCpuCacheMode(encode_cpu_cache_mode(memory_type));
            ret->setResourceOptions(encode_resource_options(memory_type));
            usize size = 0;
            for(auto& buffer : buffers)
            {
                auto sz = get_buffer_size(memory_type, buffer);
                size = max<usize>(size, sz.size);
            }
            for(auto& texture : textures)
            {
                auto sz = get_texture_size(memory_type, texture);
                size = max<usize>(size, sz.size);
            }
            ret->setSize(size);
            return ret;
        }
        DeviceFeatureData Device::check_feature(DeviceFeature feature)
        {
            DeviceFeatureData ret;
			switch (feature)
			{
			case DeviceFeature::unbound_descriptor_array:
				ret.unbound_descriptor_array = m_support_metal_3_family;
				break;
			case DeviceFeature::pixel_shader_write:
				ret.pixel_shader_write = true;
				break;
			case DeviceFeature::uniform_buffer_data_alignment:
				ret.uniform_buffer_data_alignment = 0;
				break;
			default: lupanic();
			}
			return ret;
        }
        void Device::get_texture_data_placement_info(u32 width, u32 height, u32 depth, Format format,
				u64* size, u64* alignment, u64* row_pitch, u64* slice_pitch)
        {
            if (alignment) *alignment = 4;
			u64 d_row_pitch = (u64)width * (u64)bits_per_pixel(format) / 8;
			if (row_pitch) *row_pitch = d_row_pitch;
			u64 d_slice_pitch = d_row_pitch * height;
			if (slice_pitch) *slice_pitch = d_slice_pitch;
			u64 d_size = d_slice_pitch * depth;
			if (size) *size = d_size;
        }
        R<Ref<IBuffer>> Device::new_buffer(MemoryType memory_type, const BufferDesc& desc)
        {
            Ref<IBuffer> ret;
            lutry
            {
                Ref<Buffer> buffer = new_object<Buffer>();
                buffer->m_device = this;
                luexp(buffer->init_as_committed(memory_type, desc));
                ret = buffer;
            }
            lucatchret;
            return ret;
        }
		R<Ref<ITexture>> Device::new_texture(MemoryType memory_type, const TextureDesc& desc, const ClearValue* optimized_clear_value)
        {
            Ref<ITexture> ret;
            lutry
            {
                Ref<Texture> texture = new_object<Texture>();
                texture->m_device = this;
                luexp(texture->init_as_committed(memory_type, desc));
                ret = texture;
            }
            lucatchret;
            return ret;
        }
        bool Device::is_resources_aliasing_compatible(MemoryType memory_type, Span<const BufferDesc> buffers, Span<const TextureDesc> textures)
        {
            auto desc = get_heap_desc(memory_type, buffers, textures);
            return succeeded(desc);
        }
		R<Ref<IDeviceMemory>> Device::allocate_memory(MemoryType memory_type, Span<const BufferDesc> buffers, Span<const TextureDesc> textures)
        {
            Ref<IDeviceMemory> ret;
            lutry
            {
                lulet(desc, get_heap_desc(memory_type, buffers, textures));
                Ref<DeviceMemory> memory = new_object<DeviceMemory>();
                memory->m_device = this;
                memory->m_memory_type = memory_type;
                luexp(memory->init(desc.get()));
                ret = memory;
            }
            lucatchret;
            return ret;
        }
        R<Ref<IBuffer>> Device::new_aliasing_buffer(IDeviceMemory* device_memory, const BufferDesc& desc)
        {
            Ref<IBuffer> ret;
            lutry
            {
                Ref<Buffer> buffer = new_object<Buffer>();
                buffer->m_device = this;
                luexp(buffer->init_as_aliasing(device_memory, desc));
                ret = buffer;
            }
            lucatchret;
            return ret;
        }
		R<Ref<ITexture>> Device::new_aliasing_texture(IDeviceMemory* device_memory, const TextureDesc& desc, const ClearValue* optimized_clear_value)
        {
            Ref<ITexture> ret;
            lutry
            {
                Ref<Texture> texture = new_object<Texture>();
                texture->m_device = this;
                luexp(texture->init_as_aliasing(device_memory, desc));
                ret = texture;
            }
            lucatchret;
            return ret;
        }
        R<Ref<IPipelineLayout>> Device::new_pipeline_layout(const PipelineLayoutDesc& desc)
        {
            Ref<IPipelineLayout> ret;
            lutry
            {
                Ref<PipelineLayout> o = new_object<PipelineLayout>();
                o->m_device = this;
                luexp(o->init(desc));
                ret = o;
            }
            lucatchret;
            return ret;
        }
        R<Ref<IPipelineState>> Device::new_graphics_pipeline_state(const GraphicsPipelineStateDesc& desc)
        {
            Ref<IPipelineState> ret;
            lutry
            {
                Ref<RenderPipelineState> o = new_object<RenderPipelineState>();
                o->m_device = this;
                luexp(o->init(desc));
                ret = o;
            }
            lucatchret;
            return ret;
        }
        R<Ref<IPipelineState>> Device::new_compute_pipeline_state(const ComputePipelineStateDesc& desc)
        {
            Ref<IPipelineState> ret;
            lutry
            {
                Ref<ComputePipelineState> o = new_object<ComputePipelineState>();
                o->m_device = this;
                luexp(o->init(desc));
                ret = o;
            }
            lucatchret;
            return ret;
        }
        R<Ref<IDescriptorSetLayout>> Device::new_descriptor_set_layout(const DescriptorSetLayoutDesc& desc)
        {
            Ref<IDescriptorSetLayout> ret;
            lutry
            {
                Ref<DescriptorSetLayout> o = new_object<DescriptorSetLayout>();
                o->m_device = this;
                luexp(o->init(desc));
                ret = o;
            }
            lucatchret;
            return ret;
        }
        R<Ref<IDescriptorSet>> Device::new_descriptor_set(const DescriptorSetDesc& desc)
        {
            Ref<IDescriptorSet> ret;
            lutry
            {
                Ref<DescriptorSet> o = new_object<DescriptorSet>();
                o->m_device = this;
                luexp(o->init(desc));
                ret = o;
            }
            lucatchret;
            return ret;
        }
        u32 Device::get_num_command_queues()
        {
            return (u32)m_queues.size();
        }
		CommandQueueDesc Device::get_command_queue_desc(u32 command_queue_index)
        {
            return m_queues[command_queue_index].desc;
        }
		R<Ref<ICommandBuffer>> Device::new_command_buffer(u32 command_queue_index)
        {
            Ref<ICommandBuffer> ret;
            lutry
            {
                Ref<CommandBuffer> buf = new_object<CommandBuffer>();
                buf->m_device = this;
                luexp(buf->init(command_queue_index));
                ret = buf;
            }
            lucatchret;
            return ret;
        }
        R<f64> Device::get_command_queue_timestamp_frequency(u32 command_queue_index)
        {
            if(m_timestamp_frequency == 0.0)
            {
                MTL::Timestamp cpu_time;
                MTL::Timestamp gpu_time;
                m_device->sampleTimestamps(&cpu_time, &gpu_time);
                u64 cpu_span = cpu_time - m_start_cpu_time;
                u64 gpu_span = gpu_time - m_start_gpu_time;
                f64 ret = 1000000000.0 / (f64)cpu_span * (f64)gpu_span;
                if(cpu_span >= 1000000000.0)
                {
                    m_timestamp_frequency = ret;
                }
                return ret;
            }
            return m_timestamp_frequency;
        }
        R<Ref<IQueryHeap>> Device::new_query_heap(const QueryHeapDesc& desc)
        {
            Ref<IQueryHeap> ret;
            lutry
            {
                if(desc.type == QueryType::occlusion)
                {
                    Ref<BufferQueryHeap> heap = new_object<BufferQueryHeap>();
                    heap->m_device = this;
                    luexp(heap->init(desc));
                    ret = heap;
                }
                else
                {
                    Ref<CounterSampleQueryHeap> heap = new_object<CounterSampleQueryHeap>();
                    heap->m_device = this;
                    luexp(heap->init(desc));
                    ret = heap;
                }
            }
            lucatchret;
            return ret;
        }
        R<Ref<IFence>> Device::new_fence()
        {
            Ref<IFence> ret;
            lutry
            {
                Ref<Fence> fence = new_object<Fence>();
                fence->m_device = this;
                luexp(fence->init());
                ret = fence;
            }
            lucatchret;
            return ret;
        }
        R<Ref<ISwapChain>> Device::new_swap_chain(u32 command_queue_index, Window::IWindow* window, const SwapChainDesc& desc)
        {
            Ref<ISwapChain> ret;
            lutry
            {
                Ref<SwapChain> swap_chain = new_object<SwapChain>();
                swap_chain->m_device = this;
                luexp(swap_chain->init(command_queue_index, window, desc));
                ret = swap_chain;
            }
            lucatchret;
            return ret;
        }
        LUNA_RHI_API R<Ref<IDevice>> new_device(IAdapter* adapter)
        {
            AutoreleasePool pool;
            Adapter* ada = cast_object<Adapter>(adapter->get_object());
            Ref<Device> dev = new_object<Device>();
            dev->m_device = ada->m_device;
            auto r = dev->init();
            if(failed(r)) return r.errcode();
            return Ref<IDevice>(dev);
        }
        Ref<IDevice> g_main_device;
		LUNA_RHI_API IDevice* get_main_device()
		{
			return g_main_device.get();
		}
        RV init_main_device()
        {
            if(!g_main_device)
            {
                Ref<Device> dev = new_object<Device>();
                dev->m_device = box(MTL::CreateSystemDefaultDevice());
                if(!dev->m_device) return BasicError::bad_platform_call();
                auto r = dev->init();
                if(failed(r)) return r;
                g_main_device = dev;
            }
            return ok;
        }
    }
}
