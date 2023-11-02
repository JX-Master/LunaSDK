/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Device.cpp
* @author JXMaster
* @date 2022/10/27
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RHI_API LUNA_EXPORT
#include "Device.hpp"
#include "../RHI.hpp"
#include "Instance.hpp"
#include "PipelineLayout.hpp"
#include "DescriptorSetLayout.hpp"
#include "DescriptorSet.hpp"
#include "CommandBuffer.hpp"
#include "Resource.hpp"
#include "Fence.hpp"
#include "PipelineState.hpp"
#include "QueryHeap.hpp"
#include "ResourceStateTrackingSystem.hpp"
#include "SwapChain.hpp"
namespace Luna
{
	namespace RHI
	{
		RV Device::init(VkPhysicalDevice physical_device, const Vector<QueueFamily>& queue_families)
		{
			Vector<const c8*> enabled_extensions;
			// This is required.
			enabled_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
			if (g_vk_version < VK_API_VERSION_1_1)
			{
				enabled_extensions.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
			}

			m_desc_pool_mtx = new_mutex();
			m_physical_device = physical_device;
			vkGetPhysicalDeviceProperties(physical_device, &m_physical_device_properties);
			// Check device features.
			vkGetPhysicalDeviceFeatures(physical_device, &m_physical_device_features);
			// Create queues for every valid queue family.
			Vector<VkDeviceQueueCreateInfo> queue_create_infos;
			for (usize i = 0; i < queue_families.size(); ++i)
			{
				VkDeviceQueueCreateInfo create_info{};
				auto& src = queue_families[i];
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
			
			// Create device.
			VkDeviceCreateInfo create_info{};
			VkStructureHeader* last = (VkStructureHeader*)&create_info;
			create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			create_info.pQueueCreateInfos = queue_create_infos.data();
			create_info.queueCreateInfoCount = (u32)queue_create_infos.size();
			create_info.pEnabledFeatures = &m_physical_device_features;	// Enable all supported features.
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
				CommandQueue queue;
				queue.desc = queue_families[i].desc;
				queue.queue_family_index = queue_families[i].index;
				for (usize j = 0; j < queue_families[i].num_queues; ++j)
				{
					queue.queue_index_in_family = j;
					queue.queue_mtx = new_mutex();
					queue.queue = VK_NULL_HANDLE;
					m_funcs.vkGetDeviceQueue(m_device, queue_families[i].index, j, &queue.queue);
					m_queues.push_back(queue);
				}
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
			if (m_funcs.vkGetBufferMemoryRequirements2)
			{
				funcs.vkGetBufferMemoryRequirements2KHR = m_funcs.vkGetBufferMemoryRequirements2;
				funcs.vkGetImageMemoryRequirements2KHR = m_funcs.vkGetImageMemoryRequirements2;
			}
			else
			{
				funcs.vkGetBufferMemoryRequirements2KHR = m_funcs.vkGetBufferMemoryRequirements2KHR;
				funcs.vkGetImageMemoryRequirements2KHR = m_funcs.vkGetImageMemoryRequirements2KHR;
			}
			if (m_funcs.vkBindBufferMemory2)
			{
				funcs.vkBindBufferMemory2KHR = m_funcs.vkBindBufferMemory2;
				funcs.vkBindImageMemory2KHR = m_funcs.vkBindImageMemory2;
			}
			else
			{
				funcs.vkBindBufferMemory2KHR = m_funcs.vkBindBufferMemory2KHR;
				funcs.vkBindImageMemory2KHR = m_funcs.vkBindImageMemory2KHR;
			}
			if (vkGetPhysicalDeviceMemoryProperties2)
			{
				funcs.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2;
			}
			else
			{
				funcs.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2KHR;
			}
			funcs.vkGetDeviceBufferMemoryRequirements = m_funcs.vkGetDeviceBufferMemoryRequirements;
			funcs.vkGetDeviceImageMemoryRequirements = m_funcs.vkGetDeviceImageMemoryRequirements;
			allocator_create_info.pVulkanFunctions = &funcs;
			return encode_vk_result(vmaCreateAllocator(&allocator_create_info, &m_allocator));
		}
		Device::~Device()
		{
			m_render_pass_pool.clean_up();
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
		DeviceFeatureData Device::check_feature(DeviceFeature feature)
		{
			DeviceFeatureData ret;
			switch (feature)
			{
			case DeviceFeature::unbound_descriptor_array:
				ret.unbound_descriptor_array = false;
				break;
			case DeviceFeature::pixel_shader_write: 
				ret.pixel_shader_write = m_physical_device_features.fragmentStoresAndAtomics == VK_TRUE;
				break;
			case DeviceFeature::uniform_buffer_data_alignment:
				ret.uniform_buffer_data_alignment = (u32)m_physical_device_properties.limits.minUniformBufferOffsetAlignment;
				break;
			default: lupanic();
			}
			return ret;
		}
		void Device::get_texture_data_placement_info(u32 width, u32 height, u32 depth, Format format,
			u64* size, u64* alignment, u64* row_pitch, u64* slice_pitch)
		{
			if (alignment) *alignment = (u64)get_texel_block_size(format);
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
				auto res = new_object<BufferResource>();
				res->m_device = this;
				luexp(res->init_as_committed(memory_type, desc));
				ret = res;
			}
			lucatchret;
			return ret;
		}
		R<Ref<ITexture>> Device::new_texture(MemoryType memory_type, const TextureDesc& desc, const ClearValue* optimized_clear_value)
		{
			Ref<ITexture> ret;
			lutry
			{
				auto res = new_object<ImageResource>();
				res->m_device = this;
				luexp(res->init_as_committed(memory_type, desc));
				ret = res;
			}
			lucatchret;
			return ret;
		}
		bool Device::is_resources_aliasing_compatible(MemoryType memory_type, Span<const BufferDesc> buffers, Span<const TextureDesc> textures)
		{
			usize num_descs = buffers.size() + textures.size();
			if (num_descs <= 1) return true;
			VkMemoryRequirements memory_requirements{};
			auto r = get_memory_requirements(buffers, textures, memory_requirements);
			if (failed(r)) return false;
			if (!memory_requirements.memoryTypeBits) return false;
			return true;
		}
		R<Ref<IDeviceMemory>> Device::allocate_memory(MemoryType memory_type, Span<const BufferDesc> buffers, Span<const TextureDesc> textures)
		{
			Ref<IDeviceMemory> ret;
			lutry
			{
				if (buffers.size() + textures.size() < 1) return BasicError::bad_arguments();
				VkMemoryRequirements memory_requirements;
				memory_requirements.size = 0;
				memory_requirements.alignment = 0;
				memory_requirements.memoryTypeBits = U32_MAX;
				for (usize i = 0; i < buffers.size(); ++i)
				{
					VkMemoryRequirements desc_memory_requirements;
					lulet(buffer, create_vk_buffer(buffers[i]));
					m_funcs.vkGetBufferMemoryRequirements(m_device, buffer, &desc_memory_requirements);
					memory_requirements.size = max(memory_requirements.size, desc_memory_requirements.size);
					memory_requirements.alignment = max(memory_requirements.alignment, desc_memory_requirements.alignment);
					memory_requirements.memoryTypeBits &= desc_memory_requirements.memoryTypeBits;
					m_funcs.vkDestroyBuffer(m_device, buffer, nullptr);
				}
				for (usize i = 0; i < textures.size(); ++i)
				{
					VkMemoryRequirements desc_memory_requirements;
					lulet(image, create_vk_image(textures[i]));
					m_funcs.vkGetImageMemoryRequirements(m_device, image, &desc_memory_requirements);
					memory_requirements.size = max(memory_requirements.size, desc_memory_requirements.size);
					memory_requirements.alignment = max(memory_requirements.alignment, desc_memory_requirements.alignment);
					memory_requirements.memoryTypeBits &= desc_memory_requirements.memoryTypeBits;
					m_funcs.vkDestroyImage(m_device, image, nullptr);
				}
				memory_requirements.size = align_upper(memory_requirements.size, memory_requirements.alignment);
				if (!memory_requirements.memoryTypeBits)
				{
					return BasicError::not_supported();
				}
				auto memory = new_object<DeviceMemory>();
				memory->m_device = this;
				luexp(memory->init(memory_type, true, memory_requirements));
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
				DeviceMemory* memory = cast_object<DeviceMemory>(device_memory->get_object());
				auto res = new_object<BufferResource>();
				res->m_device = this;
				luexp(res->init_as_aliasing(desc, memory));
				ret = res;
			}
			lucatchret;
			return ret;
		}
		R<Ref<ITexture>> Device::new_aliasing_texture(IDeviceMemory* device_memory, const TextureDesc& desc, const ClearValue* optimized_clear_value)
		{
			Ref<ITexture> ret;
			lutry
			{
				DeviceMemory* memory = cast_object<DeviceMemory>(device_memory->get_object());
				auto res = new_object<ImageResource>();
				res->m_device = this;
				luexp(res->init_as_aliasing(desc, memory));
				ret = res;
			}
			lucatchret;
			return ret;
		}
		R<Ref<IPipelineLayout>> Device::new_pipeline_layout(const PipelineLayoutDesc& desc)
		{
			Ref<IPipelineLayout> ret;
			lutry
			{
				auto layout = new_object<PipelineLayout>();
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
		R<Ref<IDescriptorSet>> Device::new_descriptor_set(const DescriptorSetDesc& desc)
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
		u32 Device::get_num_command_queues()
		{
			return (u32)m_queues.size();
		}
		CommandQueueDesc Device::get_command_queue_desc(u32 command_queue_index)
		{
			lucheck(command_queue_index < m_queues.size());
			return m_queues[command_queue_index].desc;
		}
		R<Ref<ICommandBuffer>> Device::new_command_buffer(u32 command_queue_index)
		{
			Ref<ICommandBuffer> ret;
			lutry
			{
				auto buf = new_object<CommandBuffer>();
				buf->m_device = this;
				luexp(buf->init(command_queue_index));
				ret = buf;
			}
			lucatchret;
			return ret;
		}
		R<f64> Device::get_command_queue_timestamp_frequency(u32 command_queue_index)
		{
			f64 period = m_physical_device_properties.limits.timestampPeriod;
			return 1000000000.0 / period;
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
		R<Ref<ISwapChain>> Device::new_swap_chain(u32 command_queue_index, Window::IWindow* window, const SwapChainDesc& desc)
		{
			Ref<ISwapChain> ret;
			lutry
			{
				auto swap_chain = new_object<SwapChain>();
				swap_chain->m_device = this;
				luexp(swap_chain->init(m_queues[command_queue_index], window, desc));
				ret = swap_chain;
			}
			lucatchret;
			return ret;
		}
		LUNA_RHI_API R<Ref<IDevice>> new_device(IAdapter* adapter)
		{
			Ref<IDevice> ret;
			lutry
			{
				Ref<Device> dev = new_object<Device>();
				Adapter* ada = cast_object<Adapter>(adapter->get_object());
				auto& queue_families = ada->m_queue_families;
				for (auto& queue_family : queue_families)
				{
					if (queue_family.desc.type == CommandQueueType::graphics)
					{
						queue_family.num_queues = min<u32>(queue_family.num_queues, 1);
					}
					else
					{
						queue_family.num_queues = min<u32>(queue_family.num_queues, 2);
					}
				}
				luexp(dev->init(ada->m_physical_device, queue_families));
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
		LUNA_RHI_API BackendType get_backend_type()
		{
			return BackendType::vulkan;
		}
	}
}