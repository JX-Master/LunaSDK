// Copyright 2018-2022 JXMaster. All rights reserved.
/*
* @file PhysicalDeviceInfo.hpp
* @author JXMaster
* @date 2023/4/16
*/
#pragma once
#include "Common.hpp"

namespace Luna
{
	namespace RHI
	{
		extern Vector<VkPhysicalDevice> g_vk_physical_devices;
		extern u32 g_vk_main_physical_device_index;

		RV init_physical_devices();
		void destroy_physical_devices();
	}
}