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
#include "DescriptorSetLayout.hpp"
#include "CommandQueue.hpp"
#include "Resource.hpp"
namespace Luna
{
	namespace RHI
	{
		RV Device::init(VkPhysicalDevice physical_device, const Vector<QueueFamily>& queue_families)
		{
			Vector<const c8*> enabled_extensions;
			// This is required.
			enabled_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

			m_mtx = new_mutex();
			m_physical_device = physical_device;
			// Create queues for every valid queue family.
			Vector<VkDeviceQueueCreateInfo> queue_create_infos(queue_families.size());
			for (usize i = 0; i < queue_create_infos.size(); ++i)
			{
				auto& dest = queue_create_infos[i];
				auto& src = queue_families[i];
				dest.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				dest.pNext = nullptr;
				dest.queueFamilyIndex = src.index;
				dest.queueCount = src.num_queues;
				f32* priorities = (f32*)alloca(sizeof(f32) * src.num_queues);
				for (usize i = 0; i < src.num_queues; ++i) priorities[i] = 1.0f;
				dest.pQueuePriorities = priorities;
				dest.flags = 0;
			}
			// Check device features.
			vkGetPhysicalDeviceMemoryProperties(physical_device, &m_memory_properties);
			VkPhysicalDeviceFeatures2 device_features{};
			device_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_features{};
			dynamic_rendering_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
			VkPhysicalDeviceDescriptorIndexingFeaturesEXT descriptor_indexing_features{};
			descriptor_indexing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
			VkStructureHeader* last_feature = (VkStructureHeader*)&device_features;
			// Check "VK_KHR_dynamic_rendering", proposed to Vulkan 1.3.
			if (g_vk_version >= VULKAN_1_3)
			{
				last_feature->pNext = &dynamic_rendering_features;
				last_feature = (VkStructureHeader*)&dynamic_rendering_features;
			}
			// Check VK_EXT_descriptor_indexing, proposed to Vulkan 1.2 and is mandatory in Vulkan 1.3
			if (g_vk_version >= VULKAN_1_2)
			{
				last_feature->pNext = &descriptor_indexing_features;
				last_feature = (VkStructureHeader*)&descriptor_indexing_features;
			}
			vkGetPhysicalDeviceFeatures2(physical_device, &device_features);
			m_dynamic_rendering_supported = (dynamic_rendering_features.dynamicRendering != VK_FALSE);
			m_descriptor_binding_variable_descriptor_count_supported = (descriptor_indexing_features.runtimeDescriptorArray &&
				descriptor_indexing_features.descriptorBindingVariableDescriptorCount &&
				descriptor_indexing_features.shaderSampledImageArrayNonUniformIndexing);
			device_features.pNext = nullptr;
			dynamic_rendering_features.pNext = nullptr;
			descriptor_indexing_features.pNext = nullptr;
			
			// Create device.
			VkDeviceCreateInfo create_info{};
			VkStructureHeader* last = (VkStructureHeader*)&create_info;
			create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			create_info.pQueueCreateInfos = queue_create_infos.data();
			create_info.queueCreateInfoCount = (u32)queue_create_infos.size();
			create_info.pEnabledFeatures = &device_features.features;	// Enable all supported features.
			create_info.enabledLayerCount = g_enabled_layers.size();	// Enable all layers specified when creating VkInstance.
			create_info.ppEnabledLayerNames = g_enabled_layers.data();
			if (m_dynamic_rendering_supported)
			{
				enabled_extensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
				last->pNext = &dynamic_rendering_features;
				last = (VkStructureHeader*)&dynamic_rendering_features;
			}
			if (m_descriptor_binding_variable_descriptor_count_supported)
			{
				enabled_extensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
				last->pNext = &descriptor_indexing_features;
				last = (VkStructureHeader*)&descriptor_indexing_features;
			}
			create_info.enabledExtensionCount = (u32)enabled_extensions.size();
			create_info.ppEnabledExtensionNames = enabled_extensions.data();
			auto r = encode_vk_result(vkCreateDevice(physical_device, &create_info, nullptr, &m_device));
			if (failed(r))
			{
				return r.errcode();
			}
			// Fetch command queue.
			for (usize i = 0; i < queue_families.size(); ++i)
			{
				VkQueue queue;
				for (usize j = 0; j < queue_families[i].num_queues; ++j)
				{
					vkGetDeviceQueue(m_device, queue_families[i].index, j, &queue);
					QueueInfo info;
					info.queue = queue;
					info.desc = queue_families[i].desc;
					info.queue_family_index = queue_families[i].index;
					m_queues.push_back(info);
					m_queue_allocated.push_back(false);
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
			return encode_vk_result(vkCreateDescriptorPool(m_device, &create_info, nullptr, &m_desc_pool));
		}
		RV Device::init_vma_allocator()
		{
			VmaAllocatorCreateInfo allocator_create_info = {};
			allocator_create_info.vulkanApiVersion = g_vk_version;
			allocator_create_info.physicalDevice = m_physical_device;
			allocator_create_info.device = m_device;
			allocator_create_info.instance = g_vk_instance;
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
				vkDestroyDescriptorPool(m_device, m_desc_pool, nullptr);
				m_desc_pool = VK_NULL_HANDLE;
			}
			if (m_device != VK_NULL_HANDLE)
			{
				vkDestroyDevice(m_device, nullptr);
				m_device = nullptr;
			}
		}
		RV Device::get_memory_requirements(Span<const ResourceDesc> descs, VkMemoryRequirements& memory_requirements)
		{
			lutry
			{
				memory_requirements.size = 0;
				memory_requirements.alignment = 0;
				memory_requirements.memoryTypeBits = U32_MAX;
				for (auto& desc : descs)
				{
					VkMemoryRequirements desc_memory_requirements;
					auto validated_desc = validate_resource_desc(desc);
					if (g_vk_version >= VULKAN_1_3)
					{
						VkMemoryRequirements2 memory_requirements_2{};
						memory_requirements_2.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
						if (validated_desc.type == ResourceType::buffer)
						{
							VkBufferCreateInfo create_info;
							encode_buffer_create_info(create_info, validated_desc);
							VkDeviceBufferMemoryRequirements buffer_memory_requirements{};
							buffer_memory_requirements.sType = VK_STRUCTURE_TYPE_DEVICE_BUFFER_MEMORY_REQUIREMENTS;
							buffer_memory_requirements.pCreateInfo = &create_info;
							vkGetDeviceBufferMemoryRequirements(m_device, &buffer_memory_requirements, &memory_requirements_2);
						}
						else
						{
							VkImageCreateInfo create_info;
							encode_image_create_info(create_info, validated_desc);
							VkDeviceImageMemoryRequirements image_memory_requirements{};
							image_memory_requirements.sType = VK_STRUCTURE_TYPE_DEVICE_IMAGE_MEMORY_REQUIREMENTS;
							image_memory_requirements.pCreateInfo = &create_info;
							vkGetDeviceImageMemoryRequirements(m_device, &image_memory_requirements, &memory_requirements_2);
						}
						desc_memory_requirements = memory_requirements_2.memoryRequirements;
					}
					else
					{
						if (validated_desc.type == ResourceType::buffer)
						{
							VkBuffer buffer;
							luexp(create_vk_buffer(m_device, validated_desc, &buffer));
							vkGetBufferMemoryRequirements(m_device, buffer, &desc_memory_requirements);
							vkDestroyBuffer(m_device, buffer, nullptr);
						}
						else
						{
							VkImage image;
							luexp(create_vk_image(m_device, validated_desc, &image));
							vkGetImageMemoryRequirements(m_device, image, &desc_memory_requirements);
							vkDestroyImage(m_device, image, nullptr);
						}
					}
					memory_requirements.size = max(memory_requirements.size, desc_memory_requirements.size);
					memory_requirements.alignment = max(memory_requirements.alignment, desc_memory_requirements.alignment);
					memory_requirements.memoryTypeBits &= desc_memory_requirements.memoryTypeBits;
				}
				memory_requirements.size = align_upper(memory_requirements.size, memory_requirements.alignment);
			}
			lucatchret;
			return ok;
		}
		bool Device::check_device_feature(DeviceFeature feature)
		{
			switch (feature)
			{
			case DeviceFeature::unbound_descriptor_array:
				return m_descriptor_binding_variable_descriptor_count_supported;
			}
			return false;
		}
		R<Ref<IResource>> Device::new_resource(const ResourceDesc& desc, const ClearValue* optimized_clear_value)
		{
			MutexGuard guard(m_mtx);
			Ref<IResource> ret;
			lutry
			{
				if (desc.type == ResourceType::buffer)
				{
					auto res = new_object<BufferResource>();
					res->m_device = this;
					luexp(res->init_as_committed(desc));
					ret = res;
				}
				else
				{
					auto res = new_object<ImageResource>();
					res->m_device = this;
					luexp(res->init_as_committed(desc));
					ret = res;
				}
			}
			lucatchret;
			return ret;
		}
		bool Device::is_resources_aliasing_compatible(Span<const ResourceDesc> descs)
		{
			MutexGuard guard(m_mtx);
			if (descs.size() <= 1) return true;
			ResourceHeapType heap_type = descs[0].heap_type;
			for (auto& desc : descs)
			{
				if (desc.heap_type != heap_type) return false;
			}
			VkMemoryRequirements memory_requirements{};
			auto r = get_memory_requirements(descs, memory_requirements);
			if (failed(r)) return false;
			if (!memory_requirements.memoryTypeBits) return false;
			return true;
		}
		R<Ref<IResource>> Device::new_aliasing_resource(IResource* existing_resource, const ResourceDesc& desc, const ClearValue* optimized_clear_value)
		{
			MutexGuard guard(m_mtx);
			Ref<IResource> ret;
			lutry
			{
				auto exist_desc = existing_resource->get_desc();
				if (exist_desc.heap_type != desc.heap_type) return BasicError::not_supported();
				DeviceMemory* memory = nullptr;
				if (exist_desc.type == ResourceType::buffer)
				{
					BufferResource* buffer = (BufferResource*)existing_resource->get_object();
					memory = buffer->m_memory;
				}
				else
				{
					ImageResource* image = (ImageResource*)existing_resource->get_object();
					memory = image->m_memory;
				}
				if (desc.type == ResourceType::buffer)
				{
					auto res = new_object<BufferResource>();
					res->m_device = this;
					luexp(res->init_as_aliasing(desc, memory));
					ret = res;
				}
				else
				{
					auto res = new_object<ImageResource>();
					res->m_device = this;
					luexp(res->init_as_aliasing(desc, memory));
					ret = res;
				}
			}
			lucatchret;
			return ret;
		}
		RV Device::new_aliasing_resources(Span<const ResourceDesc> descs, Span<const ClearValue*> optimized_clear_values, Span<Ref<IResource>> out_resources)
		{
			MutexGuard guard(m_mtx);
			lutry
			{
				if (out_resources.size() < descs.size()) return BasicError::insufficient_user_buffer();
				if (descs.size() < 1) return BasicError::bad_arguments();
				VkMemoryRequirements memory_requirements;
				ResourceHeapType heap_type = descs[0].heap_type;
				for (usize i = 0; i < descs.size(); ++i)
				{
					if (descs[i].heap_type != heap_type) return BasicError::not_supported();
				}
				memory_requirements.size = 0;
				memory_requirements.alignment = 0;
				memory_requirements.memoryTypeBits = U32_MAX;
				for (usize i = 0; i < descs.size(); ++i)
				{
					VkMemoryRequirements desc_memory_requirements;
					if (descs[i].type == ResourceType::buffer)
					{
						auto buffer = new_object<BufferResource>();
						buffer->m_device = this;
						buffer->m_desc = validate_resource_desc(descs[i]);
						luexp(create_vk_buffer(m_device, buffer->m_desc, &buffer->m_buffer));
						vkGetBufferMemoryRequirements(m_device, buffer->m_buffer, &desc_memory_requirements);
						out_resources[i] = buffer;
					}
					else
					{
						auto image = new_object<ImageResource>();
						image->m_device = this;
						image->m_desc = validate_resource_desc(descs[i]);
						luexp(create_vk_image(m_device, image->m_desc, &image->m_image));
						vkGetImageMemoryRequirements(m_device, image->m_image, &desc_memory_requirements);
						out_resources[i] = image;
					}
					memory_requirements.size = max(memory_requirements.size, desc_memory_requirements.size);
					memory_requirements.alignment = max(memory_requirements.alignment, desc_memory_requirements.alignment);
					memory_requirements.memoryTypeBits &= desc_memory_requirements.memoryTypeBits;
				}
				memory_requirements.size = align_upper(memory_requirements.size, memory_requirements.alignment);
				if (!memory_requirements.memoryTypeBits)
				{
					for (usize i = 0; i < descs.size(); ++i)
					{
						out_resources[i].reset();
					}
					return BasicError::not_supported();
				}
				VmaAllocationCreateInfo allocation{};
				encode_allocation_info(allocation, heap_type);
				lulet(memory, allocate_device_memory(this, memory_requirements, allocation));
				for (usize i = 0; i < descs.size(); ++i)
				{
					if (descs[i].type == ResourceType::buffer)
					{
						BufferResource* buffer = (BufferResource*)(out_resources[i]->get_object());
						buffer->m_memory = memory;
						luexp(buffer->post_init());
					}
					else
					{
						ImageResource* image = (ImageResource*)(out_resources[i]->get_object());
						image->m_memory = memory;
						luexp(image->post_init());
					}
				}
			}
			lucatchret;
			return ok;
		}
		R<Ref<IDescriptorSetLayout>> Device::new_descriptor_set_layout(const DescriptorSetLayoutDesc& desc)
		{
			MutexGuard guard(m_mtx);
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
		R<Ref<ICommandQueue>> Device::new_command_queue(const CommandQueueDesc& desc)
		{
			MutexGuard guard(m_mtx);
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