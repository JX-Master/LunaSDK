// Copyright 2018-2022 JXMaster. All rights reserved.
/*
* @file Instance.hpp
* @author JXMaster
* @date 2023/4/16
*/
#pragma once
#include "Common.hpp"

namespace Luna
{
	namespace RHI
	{
		//! The global Vulkan instance.
		extern VkInstance g_vk_instance;

		RV create_vk_instance();
		void destroy_vk_instance();
	}
}