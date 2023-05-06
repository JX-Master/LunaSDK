/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Device.cpp
* @author JXMaster
* @date 2022/10/27
*/
#include <Runtime/PlatformDefines.hpp>
#define LUNA_RHI_API LUNA_EXPORT
#include "Device.hpp"
#include "../RHI.hpp"
#include "VulkanRHI.hpp"
#include "Instance.hpp"
#include "ShaderInputLayout.hpp"
#include "DescriptorSetLayout.hpp"
#include "DescriptorSet.hpp"
#include "CommandQueue.hpp"
#include "Resource.hpp"
#include "RenderTargetView.hpp"
#include "DepthStencilView.hpp"
#include "ResolveTargetView.hpp"
#include "Fence.hpp"
#include "PipelineState.hpp"
#include "QueryHeap.hpp"
namespace Luna
{
	namespace RHI
	{
		RV Device::init(VkPhysicalDevice physical_device, const Vector<QueueFamily>& queue_families)
		{
			Vector<const c8*> enabled_extensions;
			// This is required.
			enabled_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

			m_queue_pool_mtx = new_mutex();
			m_render_pass_pool_mtx = new_mutex();
			m_physical_device = physical_device;
			vkGetPhysicalDeviceProperties(physical_device, &m_physical_device_properties);
			// Create queues for every valid queue family.
			Vector<VkDeviceQueueCreateInfo> queue_create_infos;
			for (usize i = 0; i < queue_create_infos.size(); ++i)
			{
				VkDeviceQueueCreateInfo create_info{};
				auto& src = queue_families[i];
				if (src.num_queues < 2) continue;
				create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				create_info.pNext = nullptr;
				create_info.queueFamilyIndex = src.index;
				create_info.queueCount = src.num_queues;
				f32* priorities = (f32*)alloca(sizeof(f32) * src.num_queues);
				for (usize i = 0; i < src.num_queues; ++i) priorities[i] = 1.0f;
				create_info.pQueuePriorities = priorities;
				create_info.flags = 0;
				queue_create_infos.push_back(create_info);
			}
			// Check device features.
			//vkGetPhysicalDeviceMemoryProperties(physical_device, &m_memory_properties);
			VkPhysicalDeviceFeatures device_features{};
			vkGetPhysicalDeviceFeatures(physical_device, &device_features);
			
			// Create device.
			VkDeviceCreateInfo create_info{};
			VkStructureHeader* last = (VkStructureHeader*)&create_info;
			create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			create_info.pQueueCreateInfos = queue_create_infos.data();
			create_info.queueCreateInfoCount = (u32)queue_create_infos.size();
			create_info.pEnabledFeatures = &device_features;	// Enable all supported features.
			create_info.enabledLayerCount = g_enabled_layers.size();	// Enable all layers specified when creating VkInstance.
			create_info.ppEnabledLayerNames = g_enabled_layers.data();
			create_info.enabledExtensionCount = (u32)enabled_extensions.size();
			create_info.ppEnabledExtensionNames = enabled_extensions.data();
			auto r = encode_vk_result(vkCreateDevice(physical_device, &create_info, nullptr, &m_device));
			volkLoadDeviceTable(&m_funcs, m_device);
			if (failed(r))
			{
				return r.errcode();
			}
			// Fetch command queue.
			for (usize i = 0; i < queue_families.size(); ++i)
			{
				if (queue_families[i].num_queues < 2) continue;
				QueuePool pool;
				pool.desc = queue_families[i].desc;
				pool.queue_family_index = queue_families[i].index;
				pool.internal_queue = VK_NULL_HANDLE;
				m_funcs.vkGetDeviceQueue(m_device, queue_families[i].index, 0, &pool.internal_queue);
				for (usize j = 1; j < queue_families[i].num_queues; ++j)
				{
					VkQueue queue = VK_NULL_HANDLE;
					m_funcs.vkGetDeviceQueue(m_device, queue_families[i].index, j, &queue);
					pool.free_queues.push_back(queue);
				}
				m_queue_pools.push_back(move(pool));
			}
			r = init_descriptor_pools();
			if (failed(r))
			{
				return r.errcode();
			}
			r = init_vma_allocator();
			if (failed(r))
			{
				return r.errcode();
			}
			m_render_pass_pool.m_device = m_device;
			m_render_pass_pool.m_vkCreateRenderPass = m_funcs.vkCreateRenderPass;
			m_render_pass_pool.m_vkDestroyRenderPass = m_funcs.vkDestroyRenderPass;
			return ok;
		}
		RV Device::init_descriptor_pools()
		{
			VkDescriptorPoolSize pool_sizes[5] = {
				{VK_DESCRIPTOR_TYPE_SAMPLER, 1024},
				{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 8192},
				{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1024},
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 8192},
				{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1024}
			};
			VkDescriptorPoolCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			create_info.poolSizeCount = 5;
			create_info.pPoolSizes = pool_sizes;
			create_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			create_info.maxSets = 8192;
			return encode_vk_result(m_funcs.vkCreateDescriptorPool(m_device, &create_info, nullptr, &m_desc_pool));
		}
		RV Device::init_vma_allocator()
		{
			VmaAllocatorCreateInfo allocator_create_info = {};
			allocator_create_info.vulkanApiVersion = g_vk_version;
			allocator_create_info.physicalDevice = m_physical_device;
			allocator_create_info.device = m_device;
			allocator_create_info.instance = g_vk_instance;
			VmaVulkanFunctions funcs{};
			funcs.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
			funcs.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
			funcs.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
			funcs.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
			funcs.vkAllocateMemory = m_funcs.vkAllocateMemory;
			funcs.vkFreeMemory = m_funcs.vkFreeMemory;
			funcs.vkMapMemory = m_funcs.vkMapMemory;
			funcs.vkUnmapMemory = m_funcs.vkUnmapMemory;
			funcs.vkFlushMappedMemoryRanges = m_funcs.vkFlushMappedMemoryRanges;
			funcs.vkInvalidateMappedMemoryRanges = m_funcs.vkInvalidateMappedMemoryRanges;
			funcs.vkBindBufferMemory = m_funcs.vkBindBufferMemory;
			funcs.vkBindImageMemory = m_funcs.vkBindImageMemory;
			funcs.vkGetBufferMemoryRequirements = m_funcs.vkGetBufferMemoryRequirements;
			funcs.vkGetImageMemoryRequirements = m_funcs.vkGetImageMemoryRequirements;
			funcs.vkCreateBuffer = m_funcs.vkCreateBuffer;
			funcs.vkDestroyBuffer = m_funcs.vkDestroyBuffer;
			funcs.vkCreateImage = m_funcs.vkCreateImage;
			funcs.vkDestroyImage = m_funcs.vkDestroyImage;
			funcs.vkCmdCopyBuffer = m_funcs.vkCmdCopyBuffer;
			allocator_create_info.pVulkanFunctions = &funcs;
			return encode_vk_result(vmaCreateAllocator(&allocator_create_info, &m_allocator));
		}
		Device::~Device()
		{
			if (m_allocator != VK_NULL_HANDLE)
			{
				vmaDestroyAllocator(m_allocator);
				m_allocator = VK_NULL_HANDLE;
			}
			if (m_desc_pool != VK_NULL_HANDLE)
			{
				m_funcs.vkDestroyDescriptorPool(m_device, m_desc_pool, nullptr);
				m_desc_pool = VK_NULL_HANDLE;
			}
			if (m_device != VK_NULL_HANDLE)
			{
				m_funcs.vkDestroyDevice(m_device, nullptr);
				m_device = nullptr;
			}
		}
		RV Device::get_memory_requirements(Span<const BufferDesc> buffers, Span<const TextureDesc> textures, VkMemoryRequirements& memory_requirements)
		{
			lutry
			{
				memory_requirements.size = 0;
				memory_requirements.alignment = 0;
				memory_requirements.memoryTypeBits = U32_MAX;
				for (auto& desc : buffers)
				{
					VkMemoryRequirements desc_memory_requirements;
					lulet(buffer, create_vk_buffer(desc));
					m_funcs.vkGetBufferMemoryRequirements(m_device, buffer, &desc_memory_requirements);
					m_funcs.vkDestroyBuffer(m_device, buffer, nullptr);
					memory_requirements.size = max(memory_requirements.size, desc_memory_requirements.size);
					memory_requirements.alignment = max(memory_requirements.alignment, desc_memory_requirements.alignment);
					memory_requirements.memoryTypeBits &= desc_memory_requirements.memoryTypeBits;
				}
				for (auto& desc : textures)
				{
					VkMemoryRequirements desc_memory_requirements;
					lulet(image, create_vk_image(desc));
					m_funcs.vkGetImageMemoryRequirements(m_device, image, &desc_memory_requirements);
					m_funcs.vkDestroyImage(m_device, image, nullptr);
					memory_requirements.size = max(memory_requirements.size, desc_memory_requirements.size);
					memory_requirements.alignment = max(memory_requirements.alignment, desc_memory_requirements.alignment);
					memory_requirements.memoryTypeBits &= desc_memory_requirements.memoryTypeBits;
				}
				memory_requirements.size = align_upper(memory_requirements.size, memory_requirements.alignment);
			}
			lucatchret;
			return ok;
		}
		R<VkBuffer> Device::create_vk_buffer(const BufferDesc& desc)
		{
			VkBuffer ret = VK_NULL_HANDLE;
			lutry
			{
				VkBufferCreateInfo create_info{};
				encode_buffer_create_info(create_info, desc);
				luexp(encode_vk_result(m_funcs.vkCreateBuffer(m_device, &create_info, nullptr, &ret)));
			}
			lucatchret;
			return ret;
		}
		R<VkImage> Device::create_vk_image(const TextureDesc& desc)
		{
			VkImage ret = VK_NULL_HANDLE;
			lutry
			{
				VkImageCreateInfo create_info{};
				encode_image_create_info(create_info, desc);
				luexp(encode_vk_result(m_funcs.vkCreateImage(m_device, &create_info, nullptr, &ret)));
			}
			lucatchret;
			return ret;
		}
		bool Device::check_device_feature(DeviceFeature feature)
		{
			switch (feature)
			{
			case DeviceFeature::unbound_descriptor_array:
				return false;
			}
			return false;
		}
		void Device::get_texture_data_placement_info(u32 width, u32 height, u32 depth, Format format,
			u64* size, u64* alignment, u64* row_pitch, u64* slice_pitch)
		{
			// Vulkan does not have image data alignment requirement.
			if (alignment) *alignment = 0;
			u64 d_row_pitch = (u64)width * (u64)bits_per_pixel(format) / 8;
			if (row_pitch) *row_pitch = d_row_pitch;
			u64 d_slice_pitch = d_row_pitch * height;
			if (slice_pitch) *slice_pitch = d_slice_pitch;
			u64 d_size = d_slice_pitch * depth;
			if (size) *size = d_size;
		}
		R<Ref<IBuffer>> Device::new_buffer(const BufferDesc& desc)
		{
			Ref<IResource> ret;
			lutry
			{
				auto res = new_object<BufferResource>();
				res->m_device = this;
				luexp(res->init_as_committed(desc));
				ret = res;
			}
			lucatchret;
			return ret;
		}
		R<Ref<ITexture>> Device::new_texture(const TextureDesc& desc, const ClearValue* optimized_clear_value)
		{
			Ref<IResource> ret;
			lutry
			{
				auto res = new_object<ImageResource>();
				res->m_device = this;
				luexp(res->init_as_committed(desc));
				ret = res;
			}
			lucatchret;
			return ret;
		}
		bool Device::is_resources_aliasing_compatible(Span<const BufferDesc> buffers, Span<const TextureDesc> textures)
		{
			usize num_descs = buffers.size() + textures.size();
			if (num_descs <= 1) return true;
			ResourceHeapType heap_type = buffers[0].heap_type;
			for (auto& desc : buffers)
			{
				if (desc.heap_type != heap_type) return false;
			}
			for (auto& desc : textures)
			{
				if (desc.heap_type != heap_type) return false;
			}
			VkMemoryRequirements memory_requirements{};
			auto r = get_memory_requirements(buffers, textures, memory_requirements);
			if (failed(r)) return false;
			if (!memory_requirements.memoryTypeBits) return false;
			return true;
		}
		R<Ref<IBuffer>> Device::new_aliasing_buffer(IResource* existing_resource, const BufferDesc& desc)
		{
			Ref<IResource> ret;
			lutry
			{
				DeviceMemory* memory = nullptr;
				ResourceHeapType heap_type;
				{
					BufferResource* buffer = cast_objct<BufferResource>(existing_resource->get_object());
					if (buffer)
					{
						heap_type = buffer->m_desc.heap_type;
						memory = buffer->m_memory;
					}
					else
					{
						ImageResource* image = cast_objct<ImageResource>(existing_resource->get_object());
						heap_type = image->m_desc.heap_type;
						memory = image->m_memory;
					}
				}
				if (heap_type != desc.heap_type) return BasicError::not_supported();
				auto res = new_object<BufferResource>();
				res->m_device = this;
				luexp(res->init_as_aliasing(desc, memory));
				ret = res;
			}
			lucatchret;
			return ret;
		}
		R<Ref<ITexture>> Device::new_aliasing_texture(IResource* existing_resource, const TextureDesc& desc, const ClearValue* optimized_clear_value)
		{
			Ref<IResource> ret;
			lutry
			{
				DeviceMemory* memory = nullptr;
				ResourceHeapType heap_type;
				{
					BufferResource* buffer = cast_objct<BufferResource>(existing_resource->get_object());
					if (buffer)
					{
						heap_type = buffer->m_desc.heap_type;
						memory = buffer->m_memory;
					}
					else
					{
						ImageResource* image = cast_objct<ImageResource>(existing_resource->get_object());
						heap_type = image->m_desc.heap_type;
						memory = image->m_memory;
					}
				}
				// This may happen if the resource memory is externally managed.
				if (!memory) return BasicError::not_supported();
				if (heap_type != desc.heap_type) return BasicError::not_supported();
				auto res = new_object<ImageResource>();
				res->m_device = this;
				luexp(res->init_as_aliasing(desc, memory));
				ret = res;
			}
			lucatchret;
			return ret;
		}
		RV Device::new_aliasing_resources(
			Span<const BufferDesc> buffers,
			Span<const TextureDesc> textures,
			Span<const ClearValue*> optimized_clear_values,
			Span<Ref<IBuffer>> out_buffers,
			Span<Ref<ITexture>> out_textures)
		{
			lutry
			{
				if (out_buffers.size() < buffers.size()) return BasicError::insufficient_user_buffer();
				if (out_textures.size() < textures.size()) return BasicError::insufficient_user_buffer();
				if (buffers.size() + textures.size() < 1) return BasicError::bad_arguments();
				VkMemoryRequirements memory_requirements;
				ResourceHeapType heap_type = buffers[0].heap_type;
				for (usize i = 0; i < buffers.size(); ++i)
				{
					if (buffers[i].heap_type != heap_type) return BasicError::not_supported();
				}
				for (usize i = 0; i < textures.size(); ++i)
				{
					if (textures[i].heap_type != heap_type) return BasicError::not_supported();
				}
				memory_requirements.size = 0;
				memory_requirements.alignment = 0;
				memory_requirements.memoryTypeBits = U32_MAX;
				for (usize i = 0; i < buffers.size(); ++i)
				{
					VkMemoryRequirements desc_memory_requirements;
					auto buffer = new_object<BufferResource>();
					buffer->m_device = this;
					buffer->m_desc = buffers[i];
					luset(buffer->m_buffer, create_vk_buffer(buffer->m_desc));
					m_funcs.vkGetBufferMemoryRequirements(m_device, buffer->m_buffer, &desc_memory_requirements);
					out_buffers[i] = buffer;
					memory_requirements.size = max(memory_requirements.size, desc_memory_requirements.size);
					memory_requirements.alignment = max(memory_requirements.alignment, desc_memory_requirements.alignment);
					memory_requirements.memoryTypeBits &= desc_memory_requirements.memoryTypeBits;
				}
				for (usize i = 0; i < textures.size(); ++i)
				{
					VkMemoryRequirements desc_memory_requirements;
					auto image = new_object<ImageResource>();
					image->m_device = this;
					image->m_desc = textures[i];
					luset(image->m_image, create_vk_image(image->m_desc));
					m_funcs.vkGetImageMemoryRequirements(m_device, image->m_image, &desc_memory_requirements);
					out_textures[i] = image;
					memory_requirements.size = max(memory_requirements.size, desc_memory_requirements.size);
					memory_requirements.alignment = max(memory_requirements.alignment, desc_memory_requirements.alignment);
					memory_requirements.memoryTypeBits &= desc_memory_requirements.memoryTypeBits;
				}
				memory_requirements.size = align_upper(memory_requirements.size, memory_requirements.alignment);
				if (!memory_requirements.memoryTypeBits)
				{
					for (usize i = 0; i < buffers.size(); ++i)
					{
						out_buffers[i].reset();
					}
					for (usize i = 0; i < textures.size(); ++i)
					{
						out_textures[i].reset();
					}
					return BasicError::not_supported();
				}
				VmaAllocationCreateInfo allocation{};
				encode_allocation_info(allocation, heap_type);
				lulet(memory, allocate_device_memory(this, memory_requirements, allocation));
				for (usize i = 0; i < buffers.size(); ++i)
				{
					BufferResource* buffer = (BufferResource*)(out_buffers[i]->get_object());
					buffer->m_memory = memory;
					luexp(buffer->post_init());
				}
				for (usize i = 0; i < textures.size(); ++i)
				{
					ImageResource* image = (ImageResource*)(out_textures[i]->get_object());
					image->m_memory = memory;
					luexp(image->post_init());
				}
			}
			lucatchret;
			return ok;
		}
		R<Ref<IShaderInputLayout>> Device::new_shader_input_layout(const ShaderInputLayoutDesc& desc)
		{
			Ref<IShaderInputLayout> ret;
			lutry
			{
				auto layout = new_object<ShaderInputLayout>();
				layout->m_device = this;
				luexp(layout->init(desc));
				ret = layout;
			}
			lucatchret;
			return ret;
		}
		R<Ref<IPipelineState>> Device::new_graphics_pipeline_state(const GraphicsPipelineStateDesc& desc)
		{
			Ref<IPipelineState> ret;
			lutry
			{
				auto layout = new_object<PipelineState>();
				layout->m_device = this;
				luexp(layout->init_as_graphics(desc));
				ret = layout;
			}
			lucatchret;
			return ret;
		}
		R<Ref<IPipelineState>> Device::new_compute_pipeline_state(const ComputePipelineStateDesc& desc)
		{
			Ref<IPipelineState> ret;
			lutry
			{
				auto layout = new_object<PipelineState>();
				layout->m_device = this;
				luexp(layout->init_as_compute(desc));
				ret = layout;
			}
			lucatchret;
			return ret;
		}
		R<Ref<IDescriptorSetLayout>> Device::new_descriptor_set_layout(const DescriptorSetLayoutDesc& desc)
		{
			Ref<IDescriptorSetLayout> ret;
			lutry
			{
				auto layout = new_object<DescriptorSetLayout>();
				layout->m_device = this;
				luexp(layout->init(desc));
				ret = layout;
			}
			lucatchret;
			return ret;
		}
		R<Ref<IDescriptorSet>> Device::new_descriptor_set(DescriptorSetDesc& desc)
		{
			Ref<IDescriptorSet> ret;
			lutry
			{
				auto set = new_object<DescriptorSet>();
				set->m_device = this;
				luexp(set->init(desc));
				ret = set;
			}
			lucatchret;
			return ret;
		}
		R<Ref<ICommandQueue>> Device::new_command_queue(const CommandQueueDesc& desc)
		{
			Ref<ICommandQueue> ret;
			lutry
			{
				auto queue = new_object<CommandQueue>();
				queue->m_device = this;
				luexp(queue->init(desc));
				ret = queue;
			}
			lucatchret;
			return ret;
		}
		R<Ref<IRenderTargetView>> Device::new_render_target_view(ITexture* resource, const RenderTargetViewDesc* desc)
		{
			Ref<IRenderTargetView> ret;
			lutry
			{
				auto view = new_object<RenderTargetView>();
				view->m_device = this;
				luexp(view->init(resource, desc));
				ret = view;
			}
			lucatchret;
			return ret;
		}
		R<Ref<IDepthStencilView>> Device::new_depth_stencil_view(ITexture* resource, const DepthStencilViewDesc* desc)
		{
			Ref<IDepthStencilView> ret;
			lutry
			{
				auto view = new_object<DepthStencilView>();
				view->m_device = this;
				luexp(view->init(resource, desc));
				ret = view;
			}
			lucatchret;
			return ret;
		}
		R<Ref<IResolveTargetView>> Device::new_resolve_target_view(ITexture* resource, const ResolveTargetViewDesc* desc)
		{
			Ref<IResolveTargetView> ret;
			lutry
			{
				auto view = new_object<ResolveTargetView>();
				view->m_device = this;
				luexp(view->init(resource, desc));
				ret = view;
			}
			lucatchret;
			return ret;
		}
		R<Ref<IQueryHeap>> Device::new_query_heap(const QueryHeapDesc& desc)
		{
			Ref<IQueryHeap> ret;
			lutry
			{
				auto view = new_object<QueryHeap>();
				view->m_device = this;
				luexp(view->init(desc));
				ret = view;
			}
			lucatchret;
			return ret;
		}
		R<Ref<IFence>> Device::new_fence()
		{
			Ref<IFence> ret;
			lutry
			{
				auto fence = new_object<Fence>();
				fence->m_device = this;
				luexp(fence->init());
				ret = fence;
			}
			lucatchret;
			return ret;
		}
		LUNA_RHI_API R<Ref<IDevice>> new_device(u32 adapter_index)
		{
			Ref<IDevice> ret;
			lutry
			{
				if (adapter_index >= g_physical_devices.size()) return set_error(BasicError::not_found(), "The specified adapter is not found.");
				Ref<Device> dev = new_object<Device>();
				luexp(dev->init(g_physical_devices[adapter_index], g_physical_device_queue_families[adapter_index]));
				ret = dev;
			}
			lucatchret;
			return ret;
		}
		Ref<IDevice> g_main_device;
		LUNA_RHI_API IDevice* get_main_device()
		{
			return g_main_device.get();
		}
	}
}