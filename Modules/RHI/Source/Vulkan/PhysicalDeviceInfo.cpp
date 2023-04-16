// Copyright 2018-2022 JXMaster. All rights reserved.
/*
* @file PhysicalDeviceInfo.cpp
* @author JXMaster
* @date 2023/4/16
*/
#include "PhysicalDeviceInfo.hpp"
#include "Instance.hpp"

namespace Luna
{
	namespace RHI
	{
		Vector<VkPhysicalDevice> g_vk_physical_devices;
		u32 g_vk_main_physical_device_index;

		RV init_physical_devices()
		{
			lutry
			{
				u32 device_count = 0;
				vkEnumeratePhysicalDevices(g_vk_instance, &device_count, nullptr);
				if (device_count == 0)
				{
					return set_error(BasicError::not_supported(), "Failed to find GPUs with Vulkan support!");
				}
				g_vk_physical_devices.resize(device_count);
				vkEnumeratePhysicalDevices(g_vk_instance, &device_count, g_vk_physical_devices.data());
			}
			lucatchret;
			return ok;
		}
		void destroy_physical_devices()
		{
			g_vk_physical_devices.clear();
			g_vk_physical_devices.shrink_to_fit();
		}
	}
}