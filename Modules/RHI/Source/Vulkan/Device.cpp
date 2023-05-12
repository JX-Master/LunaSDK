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
#include "ResourceStateTrackingSystem.hpp"
#include "SwapChain.hpp"
namespace Luna
{
	namespace RHI
	{
		RV QueueTransferTracker::init(u32 queue_family_index)
		{
			lutry
			{
				VkCommandPoolCreateInfo pool_info{};
				pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				pool_info.flags = 0;
				pool_info.queueFamilyIndex = queue_family_index;
				luexp(encode_vk_result(m_funcs->vkCreateCommandPool(m_device, &pool_info, nullptr, &m_command_pool)));
				VkCommandBufferAllocateInfo alloc_info{};
				alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				alloc_info.commandPool = m_command_pool;
				alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				alloc_info.commandBufferCount = 1;
				luexp(encode_vk_result(m_funcs->vkAllocateCommandBuffers(m_device, &alloc_info, &m_command_buffer)));
				VkSemaphoreCreateInfo semaphore_info{};
				semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
				luexp(encode_vk_result(m_funcs->vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_semaphore)));
			}
			lucatchret;
			return ok;
		}
		R<VkSemaphore> QueueTransferTracker::submit_barrier(VkQueue queue, IMutex* queue_mtx, Span<const VkBufferMemoryBarrier> buffer_barriers, Span<const VkImageMemoryBarrier> texture_barriers)
		{
			lutry
			{
				luexp(encode_vk_result(m_funcs->vkResetCommandPool(m_device, m_command_pool, 0)));
				VkCommandBufferBeginInfo begin_info{};
				begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
				begin_info.pInheritanceInfo = nullptr;
				luexp(encode_vk_result(m_funcs->vkBeginCommandBuffer(m_command_buffer, &begin_info)));
				m_funcs->vkCmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, 0, nullptr, 
					(u32)buffer_barriers.size(), buffer_barriers.data(), (u32)texture_barriers.size(), texture_barriers.data());
				luexp(encode_vk_result(m_funcs->vkEndCommandBuffer(m_command_buffer)));
				VkSubmitInfo submit{};
				submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submit.waitSemaphoreCount = 0;
				submit.pWaitSemaphores = nullptr;
				submit.pWaitDstStageMask = nullptr;
				submit.signalSemaphoreCount = 1;
				submit.pSignalSemaphores = &m_semaphore;
				MutexGuard guard(queue_mtx);
				luexp(encode_vk_result(m_funcs->vkQueueSubmit(queue, 1, &submit, VK_NULL_HANDLE)));
			}
			lucatchret;
			return m_semaphore;
		}
		QueueTransferTracker::~QueueTransferTracker()
		{
			if (m_command_buffer != VK_NULL_HANDLE)
			{
				m_funcs->vkFreeCommandBuffers(m_device, m_command_pool, 1, &m_command_buffer);
				m_command_buffer = VK_NULL_HANDLE;
			}
			if (m_command_pool != VK_NULL_HANDLE)
			{
				m_funcs->vkDestroyCommandPool(m_device, m_command_pool, nullptr);
				m_command_pool = VK_NULL_HANDLE;
			}
			if (m_semaphore != VK_NULL_HANDLE)
			{
				m_funcs->vkDestroySemaphore(m_device, m_semaphore, nullptr);
				m_semaphore = VK_NULL_HANDLE;
			}
		}
		ResourceCopyContext::~ResourceCopyContext()
		{
			if (m_command_buffer != VK_NULL_HANDLE)
			{
				m_funcs->vkFreeCommandBuffers(m_device, m_command_pool, 1, &m_command_buffer);
				m_command_buffer = VK_NULL_HANDLE;
			}
			if (m_command_pool != VK_NULL_HANDLE)
			{
				m_funcs->vkDestroyCommandPool(m_device, m_command_pool, nullptr);
				m_command_pool = VK_NULL_HANDLE;
			}
			if (m_fence != VK_NULL_HANDLE)
			{
				m_funcs->vkDestroyFence(m_device, m_fence, nullptr);
				m_fence = VK_NULL_HANDLE;
			}
		}
		RV ResourceCopyContext::init(const QueuePool& queue_pool)
		{
			lutry
			{
				VkCommandPoolCreateInfo pool_info{};
				pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				pool_info.flags = 0;
				pool_info.queueFamilyIndex = queue_pool.queue_family_index;
				luexp(encode_vk_result(m_funcs->vkCreateCommandPool(m_device, &pool_info, nullptr, &m_command_pool)));
				VkCommandBufferAllocateInfo alloc_info{};
				alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				alloc_info.commandPool = m_command_pool;
				alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				alloc_info.commandBufferCount = 1;
				luexp(encode_vk_result(m_funcs->vkAllocateCommandBuffers(m_device, &alloc_info, &m_command_buffer)));
				VkFenceCreateInfo fence_create_info{};
				fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				luexp(encode_vk_result(m_funcs->vkCreateFence(m_device, &fence_create_info, nullptr, &m_fence)));
			}
			lucatchret;
			return ok;
		}
		R<QueueTransferTracker*> ResourceCopyContext::get_transfer_tracker(u32 queue_family_index)
		{
			QueueTransferTracker* ret;
			lutry
			{
				auto iter = m_transfer_trackers.find(queue_family_index);
				if (iter == m_transfer_trackers.end())
				{
					UniquePtr<QueueTransferTracker> tracker(memnew<QueueTransferTracker>());
					tracker->m_device = m_device;
					tracker->m_funcs = m_funcs;
					luexp(tracker->init(queue_family_index));
					iter = m_transfer_trackers.insert(make_pair(queue_family_index, move(tracker))).first;
				}
				ret = iter->second.get();
			}
			lucatchret;
			return ret;
		}
		RV Device::init(VkPhysicalDevice physical_device, const Vector<QueueFamily>& queue_families)
		{
			Vector<const c8*> enabled_extensions;
			// This is required.
			enabled_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

			m_queue_pool_mtx = new_mutex();
			m_desc_pool_mtx = new_mutex();
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
				pool.internal_queue_mtx = new_mutex();
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
					BufferResource* buffer = cast_object<BufferResource>(existing_resource->get_object());
					if (buffer)
					{
						heap_type = buffer->m_desc.heap_type;
						memory = buffer->m_memory;
					}
					else
					{
						ImageResource* image = cast_object<ImageResource>(existing_resource->get_object());
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
					BufferResource* buffer = cast_object<BufferResource>(existing_resource->get_object());
					if (buffer)
					{
						heap_type = buffer->m_desc.heap_type;
						memory = buffer->m_memory;
					}
					else
					{
						ImageResource* image = cast_object<ImageResource>(existing_resource->get_object());
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
		R<Ref<ISwapChain>> Device::new_swap_chain(ICommandQueue* queue, Window::IWindow* window, const SwapChainDesc& desc)
		{
			Ref<ISwapChain> ret;
			lutry
			{
				auto swap_chain = new_object<SwapChain>();
				swap_chain->m_device = this;
				CommandQueue* command_queue = cast_object<CommandQueue>(queue->get_object());
				luexp(swap_chain->init(command_queue, window, desc));
				ret = swap_chain;
			}
			lucatchret;
			return ret;
		}
		struct CopyBufferPlacementInfo
		{
			u64 offset;
			u64 row_pitch;
			u64 depth_pitch;
			Format pixel_format;
		};
		RV Device::copy_resource(Span<const ResourceCopyDesc> copies)
		{
			lutry
			{
				// Acquire copy queue.
				QueuePool* copy_queue = nullptr;
				for (auto& queue : m_queue_pools)
				{
					if (queue.desc.type == CommandQueueType::copy)
					{
						copy_queue = &queue;
						break;
					}
				}
				if (!copy_queue)
				{
					for (auto& queue : m_queue_pools)
					{
						if (queue.desc.type == CommandQueueType::graphics)
						{
							copy_queue = &queue;
							break;
						}
					}
				}
				luassert(copy_queue);

				ResourceStateTrackingSystem tracking_system;
				
				// Allocate one upload and one readback heap.
				u64 upload_buffer_size = 0;
				u64 readback_buffer_size = 0;
				Vector<CopyBufferPlacementInfo> placements;
				placements.reserve(copies.size());
				for (auto& i : copies)
				{
					if (i.op == ResourceCopyOp::read_buffer)
					{
						u64 offset = readback_buffer_size;
						placements.push_back({ offset, 0, 0, Format::unknown });
						readback_buffer_size += i.read_buffer.size;
						tracking_system.pack_buffer({ i.read_buffer.src, BufferStateFlag::automatic, BufferStateFlag::copy_source, ResourceBarrierFlag::none });
					}
					else if (i.op == ResourceCopyOp::write_buffer)
					{
						u64 offset = upload_buffer_size;
						placements.push_back({ offset, 0, 0, Format::unknown });
						upload_buffer_size += i.write_buffer.size;
						tracking_system.pack_buffer({i.write_buffer.dst, BufferStateFlag::automatic, BufferStateFlag::copy_dest, ResourceBarrierFlag::none});
					}
					else if (i.op == ResourceCopyOp::read_texture)
					{
						u64 size, alignment, row_pitch, depth_pitch;
						auto desc = i.read_texture.src->get_desc();
						get_texture_data_placement_info(i.read_texture.read_box.width, i.read_texture.read_box.height, i.read_texture.read_box.depth,
							desc.pixel_format, &size, &alignment, &row_pitch, &depth_pitch);
						u64 offset = align_upper(readback_buffer_size, alignment);
						placements.push_back({ offset, row_pitch, depth_pitch, desc.pixel_format });
						readback_buffer_size = offset + size;
						tracking_system.pack_image({ i.read_texture.src, i.read_texture.src_subresource, TextureStateFlag::automatic, TextureStateFlag::copy_source, ResourceBarrierFlag::none });
					}
					else if (i.op == ResourceCopyOp::write_texture)
					{
						u64 size, alignment, row_pitch, depth_pitch;
						auto desc = i.write_texture.dst->get_desc();
						get_texture_data_placement_info(i.write_texture.write_box.width, i.write_texture.write_box.height, i.write_texture.write_box.depth,
							desc.pixel_format, &size, &alignment, &row_pitch, &depth_pitch);
						u64 offset = align_upper(upload_buffer_size, alignment);
						placements.push_back({ offset, row_pitch, depth_pitch, desc.pixel_format });
						upload_buffer_size = offset + size;
						tracking_system.pack_image({ i.write_texture.dst, i.write_texture.dst_subresource, TextureStateFlag::automatic, TextureStateFlag::copy_dest, ResourceBarrierFlag::none });
					}
				}
				// Create staging buffer.
				Ref<IBuffer> upload_buffer;
				Ref<IBuffer> readback_buffer;
				void* upload_data = nullptr;
				void* readback_data = nullptr;
				if (upload_buffer_size)
				{
					luset(upload_buffer, new_buffer(BufferDesc(ResourceHeapType::upload, upload_buffer_size, BufferUsageFlag::copy_source)));
					luset(upload_data, upload_buffer->map(0, 0));
					// Fill upload data.
					for (usize i = 0; i < copies.size(); ++i)
					{
						auto& copy = copies[i];
						if (copy.op == ResourceCopyOp::write_buffer)
						{
							memcpy((byte_t*)upload_data + (usize)placements[i].offset, copy.write_buffer.src, copy.write_buffer.size);
						}
						else if (copy.op == ResourceCopyOp::write_texture)
						{
							usize copy_size_per_row = bits_per_pixel(placements[i].pixel_format) * copy.write_texture.write_box.width / 8;
							memcpy_bitmap3d((byte_t*)upload_data + (usize)placements[i].offset, copy.write_texture.src,
								copy_size_per_row, copy.write_texture.write_box.height, copy.write_texture.write_box.depth,
								(usize)placements[i].row_pitch, copy.write_texture.src_row_pitch, (usize)placements[i].depth_pitch, copy.write_texture.src_depth_pitch);
						}
					}
					upload_buffer->unmap(0, USIZE_MAX);
					tracking_system.pack_buffer({ upload_buffer, BufferStateFlag::automatic, BufferStateFlag::copy_source, ResourceBarrierFlag::none });
				}
				if (readback_buffer_size)
				{
					luset(readback_buffer, new_buffer(BufferDesc(ResourceHeapType::readback, readback_buffer_size, BufferUsageFlag::copy_dest)));
					tracking_system.pack_buffer({ readback_buffer, BufferStateFlag::automatic, BufferStateFlag::copy_dest, ResourceBarrierFlag::none });
				}
				
				// Use GPU to copy data.
				ResourceCopyContext context;
				m_copy_contexts_lock.lock();
				if (m_copy_contexts.empty())
				{
					m_copy_contexts_lock.unlock();
					context.m_device = m_device;
					context.m_funcs = &m_funcs;
					luexp(context.init(*copy_queue));
				}
				else
				{
					context = move(m_copy_contexts.back());
					m_copy_contexts.pop_back();
					m_copy_contexts_lock.unlock();
				}
				auto buffer_barriers = tracking_system.m_buffer_barriers;
				auto image_barriers = tracking_system.m_image_barriers;
				auto src_stages = tracking_system.m_src_stage_flags;
				auto dst_stages = tracking_system.m_dest_stage_flags;
				tracking_system.resolve();
				buffer_barriers.insert(buffer_barriers.end(), tracking_system.m_buffer_barriers.begin(), tracking_system.m_buffer_barriers.end());
				image_barriers.insert(image_barriers.end(), tracking_system.m_image_barriers.begin(), tracking_system.m_image_barriers.end());
				src_stages |= tracking_system.m_src_stage_flags;
				dst_stages |= tracking_system.m_dest_stage_flags;
				VkCommandBufferBeginInfo begin_info{};
				begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
				begin_info.pInheritanceInfo = nullptr;
				luexp(encode_vk_result(m_funcs.vkBeginCommandBuffer(context.m_command_buffer, &begin_info)));
				m_funcs.vkCmdPipelineBarrier(context.m_command_buffer, src_stages, dst_stages, 0, 0, nullptr,
					(u32)buffer_barriers.size(), buffer_barriers.data(), (u32)image_barriers.size(), image_barriers.data());
				// Queue ownership transfer.
				Vector<VkSemaphore> wait_semaphores;
				Vector<VkPipelineStageFlags> wait_stages;
				for (auto& transfer_barriers : tracking_system.m_queue_transfer_barriers)
				{
					lulet(transfer_tracker, context.get_transfer_tracker(transfer_barriers.first));
					VkQueue queue = VK_NULL_HANDLE;
					IMutex* queue_mtx = nullptr;
					for (auto& q : m_queue_pools)
					{
						if (q.queue_family_index == transfer_barriers.first)
						{
							queue = q.internal_queue;
							queue_mtx = q.internal_queue_mtx;
							break;
						}
					}
					luassert(queue != VK_NULL_HANDLE);
					lulet(sema, transfer_tracker->submit_barrier(queue, queue_mtx,
						{ transfer_barriers.second.buffer_barriers.data(), transfer_barriers.second.buffer_barriers.size() },
						{ transfer_barriers.second.image_barriers.data(), transfer_barriers.second.image_barriers.size() }
					));
					wait_semaphores.push_back(sema);
					wait_stages.push_back(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
				}
				
				for (usize i = 0; i < copies.size(); ++i)
				{
					auto& copy = copies[i];
					if (copy.op == ResourceCopyOp::read_buffer)
					{
						BufferResource* src = cast_object<BufferResource>(copy.read_buffer.src->get_object());
						BufferResource* dst = cast_object<BufferResource>(readback_buffer->get_object());
						VkBufferCopy buffer_copy;
						buffer_copy.srcOffset = copy.read_buffer.src_offset;
						buffer_copy.dstOffset = placements[i].offset;
						buffer_copy.size = copy.read_buffer.size;
						m_funcs.vkCmdCopyBuffer(context.m_command_buffer, src->m_buffer, dst->m_buffer, 1, &buffer_copy);
					}
					else if (copy.op == ResourceCopyOp::write_buffer)
					{
						BufferResource* src = cast_object<BufferResource>(upload_buffer->get_object());
						BufferResource* dst = cast_object<BufferResource>(copy.write_buffer.dst->get_object());
						VkBufferCopy buffer_copy;
						buffer_copy.srcOffset = placements[i].offset;
						buffer_copy.dstOffset = copy.write_buffer.dst_offset;
						buffer_copy.size = copy.write_buffer.size;
						m_funcs.vkCmdCopyBuffer(context.m_command_buffer, src->m_buffer, dst->m_buffer, 1, &buffer_copy);
					}
					else if (copy.op == ResourceCopyOp::read_texture)
					{
						ImageResource* src = cast_object<ImageResource>(copy.read_texture.src->get_object());
						BufferResource* dst = cast_object<BufferResource>(readback_buffer->get_object());
						VkBufferImageCopy image_copy{};
						image_copy.imageSubresource.aspectMask = is_depth_stencil_format(placements[i].pixel_format) ?
							VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT :
							VK_IMAGE_ASPECT_COLOR_BIT;
						image_copy.imageSubresource.baseArrayLayer = copy.read_texture.src_subresource.array_slice;
						image_copy.imageSubresource.layerCount = 1;
						image_copy.imageSubresource.mipLevel = copy.read_texture.src_subresource.mip_slice;
						image_copy.imageOffset.x = copy.read_texture.read_box.offset_x;
						image_copy.imageOffset.y = copy.read_texture.read_box.offset_y;
						image_copy.imageOffset.z = copy.read_texture.read_box.offset_z;
						image_copy.imageExtent.width = copy.read_texture.read_box.width;
						image_copy.imageExtent.height = copy.read_texture.read_box.height;
						image_copy.imageExtent.depth = copy.read_texture.read_box.depth;
						auto& placement = placements[i];
						image_copy.bufferOffset = placement.offset;
						image_copy.bufferRowLength = placement.row_pitch * 8 / bits_per_pixel(placement.pixel_format);
						image_copy.bufferImageHeight = placement.depth_pitch * 8 / bits_per_pixel(placement.pixel_format);
						m_funcs.vkCmdCopyImageToBuffer(context.m_command_buffer, src->m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
							dst->m_buffer, 1, &image_copy);
					}
					else if (copy.op == ResourceCopyOp::write_texture)
					{
						BufferResource* src = cast_object<BufferResource>(upload_buffer->get_object());
						ImageResource* dst = cast_object<ImageResource>(copy.write_texture.dst->get_object());
						VkBufferImageCopy image_copy{};
						auto& placement = placements[i];
						image_copy.bufferOffset = placement.offset;
						image_copy.bufferRowLength = placement.row_pitch * 8 / bits_per_pixel(placement.pixel_format);
						image_copy.bufferImageHeight = placement.depth_pitch * 8 / bits_per_pixel(placement.pixel_format);
						image_copy.imageSubresource.aspectMask = is_depth_stencil_format(placement.pixel_format) ?
							VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT :
							VK_IMAGE_ASPECT_COLOR_BIT;
						image_copy.imageSubresource.baseArrayLayer = copy.write_texture.dst_subresource.array_slice;
						image_copy.imageSubresource.layerCount = 1;
						image_copy.imageSubresource.mipLevel = copy.write_texture.dst_subresource.mip_slice;
						image_copy.imageOffset.x = copy.write_texture.write_box.offset_x;
						image_copy.imageOffset.y = copy.write_texture.write_box.offset_y;
						image_copy.imageOffset.z = copy.write_texture.write_box.offset_z;
						image_copy.imageExtent.width = copy.write_texture.write_box.width;
						image_copy.imageExtent.height = copy.write_texture.write_box.height;
						image_copy.imageExtent.depth = copy.write_texture.write_box.depth;
						m_funcs.vkCmdCopyBufferToImage(context.m_command_buffer, src->m_buffer, dst->m_image,
							VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &image_copy);
					}
				}
				tracking_system.generate_finish_barriers();
				buffer_barriers = tracking_system.m_buffer_barriers;
				image_barriers = tracking_system.m_image_barriers;
				src_stages = tracking_system.m_src_stage_flags;
				dst_stages = tracking_system.m_dest_stage_flags;
				m_funcs.vkCmdPipelineBarrier(context.m_command_buffer, src_stages, dst_stages, 0, 0, nullptr,
					(u32)buffer_barriers.size(), buffer_barriers.data(), (u32)image_barriers.size(), image_barriers.data());
				tracking_system.apply();
				luexp(encode_vk_result(m_funcs.vkEndCommandBuffer(context.m_command_buffer)));
				VkSubmitInfo submit{};
				submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submit.waitSemaphoreCount = (u32)wait_semaphores.size();
				submit.pWaitSemaphores = wait_semaphores.data();
				submit.pWaitDstStageMask = wait_stages.data();
				submit.signalSemaphoreCount = 0;
				submit.pSignalSemaphores = nullptr;
				MutexGuard guard(copy_queue->internal_queue_mtx);
				luexp(encode_vk_result(m_funcs.vkQueueSubmit(copy_queue->internal_queue, 1, &submit, context.m_fence)));
				luexp(encode_vk_result(m_funcs.vkWaitForFences(m_device, 1, &context.m_fence, VK_TRUE, U64_MAX)));
				luexp(encode_vk_result(m_funcs.vkResetFences(m_device, 1, &context.m_fence)));
				luexp(encode_vk_result(m_funcs.vkResetCommandPool(m_device, context.m_command_pool, 0)));
				// Give back context.
				m_copy_contexts_lock.lock();
				m_copy_contexts.push_back(move(context));
				m_copy_contexts_lock.unlock();
				// Read data for read calls.
				if (readback_buffer)
				{
					luset(readback_data, readback_buffer->map(0, USIZE_MAX));
					for (usize i = 0; i < copies.size(); ++i)
					{
						auto& copy = copies[i];
						if (copy.op == ResourceCopyOp::read_buffer)
						{
							memcpy(copy.read_buffer.dst, (byte_t*)readback_data + (usize)placements[i].offset, copy.read_buffer.size);
						}
						else if (copy.op == ResourceCopyOp::read_texture)
						{
							usize copy_size_per_row = bits_per_pixel(placements[i].pixel_format) * copy.read_texture.read_box.width / 8;
							memcpy_bitmap3d(copy.read_texture.dst, (byte_t*)readback_data + (usize)placements[i].offset,
								copy_size_per_row, copy.read_texture.read_box.height, copy.read_texture.read_box.depth,
								copy.read_texture.dst_row_pitch, (usize)placements[i].row_pitch, copy.read_texture.dst_depth_pitch, (usize)placements[i].depth_pitch);
						}
					}
					readback_buffer->unmap(0, 0);
				}
			}
			lucatchret;
			return ok;
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
		LUNA_RHI_API APIType get_current_platform_api_type()
		{
			return APIType::vulkan;
		}
	}
}