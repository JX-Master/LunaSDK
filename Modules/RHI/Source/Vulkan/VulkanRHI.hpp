// Copyright 2018-2022 JXMaster. All rights reserved.
/*
* @file VulkanRHI.hpp
* @author JXMaster
* @date 2022/10/27
*/
#pragma once
#include "Common.hpp"
namespace Luna
{
	namespace RHI
	{
		struct QueueFamily
		{
			u32 index;
			CommandQueueType type;
			u32 num_queues;
			bool present_support;
		};

		struct PhysicalDeviceSurfaceInfo
		{
			VkSurfaceCapabilitiesKHR capabilities;
			Vector<VkSurfaceFormatKHR> formats;
			Vector<VkPresentModeKHR> present_modes;
		};

		PhysicalDeviceSurfaceInfo get_physical_device_surface_info(VkPhysicalDevice device, VkSurfaceKHR surface);

		extern Vector<VkPhysicalDevice> g_physical_devices;
		//! The physical device selected as the main device.
		extern u32 g_main_physical_device_index;
		//! The device queue families for the physical device.
		extern Vector<QueueFamily> g_device_queue_families;
		
		extern bool g_enable_validation_layers;
		extern const c8* g_validation_layers[];
		extern usize g_num_validation_layers;

		extern const c8* g_device_extensions[];
		extern usize g_num_device_extensions;


	}
}