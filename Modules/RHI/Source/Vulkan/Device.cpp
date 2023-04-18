// Copyright 2018-2022 JXMaster. All rights reserved.
/*
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
#include "CommandQueue.hpp"
namespace Luna
{
	namespace RHI
	{
		constexpr const c8* VK_DEVICE_EXTENSIONS[] = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
		constexpr usize NUM_VK_DEVICE_EXTENSIONS = 1;
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
				create_info.enabledLayerCount = NUM_VK_ENABLED_LAYERS;
				create_info.ppEnabledLayerNames = VK_ENABLED_LAYERS;
			}
			else
			{
				create_info.enabledLayerCount = 0;
			}
			create_info.enabledExtensionCount = (u32)NUM_VK_DEVICE_EXTENSIONS;
			create_info.ppEnabledExtensionNames = VK_DEVICE_EXTENSIONS;
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
					m_queues.push_back(info);
					m_queue_allocated.push_back(false);
				}
			}
			return ok;
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