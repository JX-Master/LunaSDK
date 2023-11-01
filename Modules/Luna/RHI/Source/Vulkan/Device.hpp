/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Device.hpp
* @author JXMaster
* @date 2022/10/27
*/
#pragma once
#include "Common.hpp"
#include <Luna/Runtime/Mutex.hpp>
#include "Adapter.hpp"
#include "RenderPassPool.hpp"
#include <Luna/Runtime/SpinLock.hpp>
#include <Luna/Runtime/UniquePtr.hpp>
namespace Luna
{
	namespace RHI
	{
		struct CommandQueue
		{
			VkQueue queue;
			CommandQueueDesc desc;
			u32 queue_family_index;
			u32 queue_index_in_family;
			Ref<IMutex> queue_mtx;
		};

		struct Device : IDevice
		{
			lustruct("RHI::Device", "{9C0F7754-FA08-4FF3-BF66-B23125FA19F9}");
			luiimpl();

			VkDevice m_device = VK_NULL_HANDLE;
			VkPhysicalDevice m_physical_device;
			// All created command queues.
			Vector<CommandQueue> m_queues;

			VolkDeviceTable m_funcs;

			// Features.
			//VkPhysicalDeviceMemoryProperties m_memory_properties;
			VkPhysicalDeviceFeatures m_physical_device_features;
			VkPhysicalDeviceProperties m_physical_device_properties;

			// Descriptor Pools.
			VkDescriptorPool m_desc_pool = VK_NULL_HANDLE;
			Ref<IMutex> m_desc_pool_mtx;

			// Vulkan memory allocator.
			VmaAllocator m_allocator = VK_NULL_HANDLE;

			// Render pass pools.
			RenderPassPool m_render_pass_pool;
			SpinLock m_render_pass_pool_lock;

			RV init(VkPhysicalDevice physical_device, const Vector<QueueFamily>& queue_families);
			RV init_descriptor_pools();
			RV init_vma_allocator();
			~Device();

			RV get_memory_requirements(Span<const BufferDesc> buffers, Span<const TextureDesc> textures, VkMemoryRequirements& memory_requirements);
			R<VkBuffer> create_vk_buffer(const BufferDesc& desc);
			R<VkImage> create_vk_image(const TextureDesc& desc);

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
	}
}