// Copyright 2018-2022 JXMaster. All rights reserved.
/*
* @file Device.cpp
* @author JXMaster
* @date 2022/10/27
*/
#include "Device.hpp"
#include "VulkanRHI.hpp"
namespace Luna
{
	namespace RHI
	{
		RV Device::init(VkPhysicalDevice physical_device, const Vector<QueueFamily>& queue_families)
		{
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
			VkPhysicalDeviceFeatures device_features{};
			VkDeviceCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			create_info.pQueueCreateInfos = queue_create_infos.data();
			create_info.queueCreateInfoCount = (u32)queue_create_infos.size();
			create_info.pEnabledFeatures = &device_features;
			if (g_enable_validation_layers)
			{
				create_info.enabledLayerCount = g_num_validation_layers;
				create_info.ppEnabledLayerNames = g_validation_layers;
			}
			else
			{
				create_info.enabledLayerCount = 0;
			}
			create_info.enabledExtensionCount = (u32)g_num_device_extensions;
			create_info.ppEnabledExtensionNames = g_device_extensions;
			auto r = encode_vk_result(vkCreateDevice(physical_device, &create_info, nullptr, &m_device));
			if (failed(r))
			{
				return r.errcode();
			}
			// Fetch command queue.
			for (usize i = 0; i < queue_families.size(); ++i)
			{
				CommandQueueHandle handle;
				handle.m_used = false;
				for (usize j = 0; j < queue_families[i].num_queues; ++j)
				{
					vkGetDeviceQueue(m_device, queue_families[i].index, j, &handle.m_queue);
					switch (queue_families[i].type)
					{
					case CommandQueueType::graphic:
						m_graphic_queues.push_back(handle); break;
					case CommandQueueType::compute:
						m_compute_queues.push_back(handle); break;
					case CommandQueueType::copy:
						m_copy_queues.push_back(handle); break;
					default:
						lupanic();
					}
				}
			}
			return ok;
		}
	}
}