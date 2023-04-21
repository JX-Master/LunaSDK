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
namespace Luna
{
	namespace RHI
	{
		struct QueueInfo
		{
			VkQueue queue;
			CommandQueueDesc desc;
			u32 queue_family_index;
		};

		struct Device : IDevice
		{
			lustruct("RHI::Device", "{9C0F7754-FA08-4FF3-BF66-B23125FA19F9}");
			luiimpl();

			VkDevice m_device = VK_NULL_HANDLE;
			VkPhysicalDevice m_physical_device;
			// All created queues.
			Vector<QueueInfo> m_queues;
			Vector<bool> m_queue_allocated;
			Ref<IMutex> m_mtx;

			VolkDeviceTable m_funcs;

			// Features.
			//VkPhysicalDeviceMemoryProperties m_memory_properties;

			// Descriptor Pools.
			VkDescriptorPool m_desc_pool = VK_NULL_HANDLE;

			// Vulkan memory allocator.
			VmaAllocator m_allocator = VK_NULL_HANDLE;

			RV init(VkPhysicalDevice physical_device, const Vector<QueueFamily>& queue_families);
			RV init_descriptor_pools();
			RV init_vma_allocator();
			~Device();

			RV get_memory_requirements(Span<const ResourceDesc> descs, VkMemoryRequirements& memory_requirements);
			R<VkBuffer> create_vk_buffer(const ResourceDesc& validated_desc);
			R<VkImage> create_vk_image(const ResourceDesc& validated_desc);

			virtual bool check_device_feature(DeviceFeature feature) override;
			virtual usize get_constant_buffer_data_alignment() override;
			virtual void get_texture_data_placement_info(u32 width, u32 height, u32 depth, Format format,
				u64* size, u64* alignment, u64* row_pitch, u64* slice_pitch) override;
			virtual R<Ref<IResource>> new_resource(const ResourceDesc& desc, const ClearValue* optimized_clear_value) override;
			virtual bool is_resources_aliasing_compatible(Span<const ResourceDesc> descs) override;
			virtual R<Ref<IResource>> new_aliasing_resource(IResource* existing_resource, const ResourceDesc& desc, const ClearValue* optimized_clear_value) override;
			virtual RV new_aliasing_resources(Span<const ResourceDesc> descs, Span<const ClearValue*> optimized_clear_values, Span<Ref<IResource>> out_resources) override;
			virtual R<Ref<IShaderInputLayout>> new_shader_input_layout(const ShaderInputLayoutDesc& desc) override;
			virtual R<Ref<IPipelineState>> new_graphics_pipeline_state(const GraphicsPipelineStateDesc& desc) override;
			virtual R<Ref<IPipelineState>> new_compute_pipeline_state(const ComputePipelineStateDesc& desc) override;
			virtual R<Ref<IDescriptorSetLayout>> new_descriptor_set_layout(const DescriptorSetLayoutDesc& desc) override;
			virtual R<Ref<IDescriptorSet>> new_descriptor_set(DescriptorSetDesc& desc) override;
			virtual R<Ref<ICommandQueue>> new_command_queue(const CommandQueueDesc& desc) override;
			virtual R<Ref<IRenderTargetView>> new_render_target_view(IResource* resource, const RenderTargetViewDesc* desc) override;
			virtual R<Ref<IDepthStencilView>> new_depth_stencil_view(IResource* resource, const DepthStencilViewDesc* desc) override;
			virtual R<Ref<IQueryHeap>> new_query_heap(const QueryHeapDesc& desc) override;
			virtual R<Ref<IDeviceFence>> new_device_fence() override;
			virtual R<Ref<IHostFence>> new_host_fence() override;
			virtual RV copy_resource(Span<const ResourceCopyDesc> copies) override;
		};

		extern Ref<IDevice> g_main_device;
	}
}