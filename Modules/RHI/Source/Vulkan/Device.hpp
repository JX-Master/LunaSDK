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
#include "VulkanRHI.hpp"
#include <Runtime/Mutex.hpp>
#include "Adapter.hpp"
#include "RenderPassPool.hpp"
namespace Luna
{
	namespace RHI
	{
		struct QueueInfo
		{
			VkQueue queue;
			CommandQueueDesc desc;
		};

		struct Device : IDevice
		{
			lustruct("RHI::Device", "{9C0F7754-FA08-4FF3-BF66-B23125FA19F9}");
			luiimpl();

			VkDevice m_device = VK_NULL_HANDLE;
			VkPhysicalDevice m_physical_device;
			// All created queues.
			Vector<QueueInfo> m_queues;
			// The queue family index for all created queues. We only use 1 queue family here.
			u32 m_queue_family_index;
			Vector<bool> m_queue_allocated;
			Ref<IMutex> m_mtx;

			VolkDeviceTable m_funcs;

			// Features.
			//VkPhysicalDeviceMemoryProperties m_memory_properties;
			VkPhysicalDeviceProperties m_physical_device_properties;

			// Descriptor Pools.
			VkDescriptorPool m_desc_pool = VK_NULL_HANDLE;

			// Vulkan memory allocator.
			VmaAllocator m_allocator = VK_NULL_HANDLE;

			// Render pass pools.
			RenderPassPool m_render_pass_pool;

			RV init(VkPhysicalDevice physical_device, u32 queue_family_index, u32 num_queues);
			RV init_descriptor_pools();
			RV init_vma_allocator();
			~Device();

			RV get_memory_requirements(Span<const BufferDesc> buffers, Span<const TextureDesc> textures, VkMemoryRequirements& memory_requirements);
			R<VkBuffer> create_vk_buffer(const BufferDesc& desc);
			R<VkImage> create_vk_image(const TextureDesc& desc);

			virtual bool check_device_feature(DeviceFeature feature) override;
			virtual usize get_constant_buffer_data_alignment() override
			{
				return 0; // Vulkan does not have constant buffer data alignment requirement.
			}
			virtual void get_texture_data_placement_info(u32 width, u32 height, u32 depth, Format format,
				u64* size, u64* alignment, u64* row_pitch, u64* slice_pitch) override;
			virtual R<Ref<IBuffer>> new_buffer(const BufferDesc& desc) override;
			virtual R<Ref<ITexture>> new_texture(const TextureDesc& desc, const ClearValue* optimized_clear_value) override;
			virtual bool is_resources_aliasing_compatible(Span<const BufferDesc> buffers, Span<const TextureDesc> textures) override;
			virtual R<Ref<IBuffer>> new_aliasing_buffer(IResource* existing_resource, const BufferDesc& desc) override;
			virtual R<Ref<ITexture>> new_aliasing_texture(IResource* existing_resource, const TextureDesc& desc, const ClearValue* optimized_clear_value) override;
			virtual RV new_aliasing_resources(
				Span<const BufferDesc> buffers,
				Span<const TextureDesc> textures,
				Span<const ClearValue*> optimized_clear_values,
				Span<Ref<IBuffer>> out_buffers,
				Span<Ref<ITexture>> out_textures) override;
			virtual R<Ref<IShaderInputLayout>> new_shader_input_layout(const ShaderInputLayoutDesc& desc) override;
			virtual R<Ref<IPipelineState>> new_graphics_pipeline_state(const GraphicsPipelineStateDesc& desc) override;
			virtual R<Ref<IPipelineState>> new_compute_pipeline_state(const ComputePipelineStateDesc& desc) override;
			virtual R<Ref<IDescriptorSetLayout>> new_descriptor_set_layout(const DescriptorSetLayoutDesc& desc) override;
			virtual R<Ref<IDescriptorSet>> new_descriptor_set(DescriptorSetDesc& desc) override;
			virtual R<Ref<ICommandQueue>> new_command_queue(const CommandQueueDesc& desc) override;
			virtual R<Ref<IRenderTargetView>> new_render_target_view(ITexture* resource, const RenderTargetViewDesc* desc) override;
			virtual R<Ref<IDepthStencilView>> new_depth_stencil_view(ITexture* resource, const DepthStencilViewDesc* desc) override;
			virtual R<Ref<IResolveTargetView>> new_resolve_target_view(ITexture* resource, const ResolveTargetViewDesc* desc) override;
			virtual R<Ref<IQueryHeap>> new_query_heap(const QueryHeapDesc& desc) override;
			virtual R<Ref<IFence>> new_fence() override;
			virtual RV copy_resource(Span<const ResourceCopyDesc> copies) override;
		};

		extern Ref<IDevice> g_main_device;
	}
}