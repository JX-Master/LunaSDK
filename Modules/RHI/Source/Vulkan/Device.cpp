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
		Device::~Device()
		{
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
		bool Device::check_device_feature(DeviceFeature feature)
		{
			switch (feature)
			{
			case DeviceFeature::unbound_descriptor_array:
				return m_descriptor_binding_variable_descriptor_count_supported;
			}
			return false;
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