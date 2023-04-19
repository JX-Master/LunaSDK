/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
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

		extern bool g_enable_validation_layers;

		constexpr const c8* VK_ENABLED_LAYERS[] = {
			"VK_LAYER_KHRONOS_validation"
		};
		constexpr usize NUM_VK_ENABLED_LAYERS = 1;

		RV create_vk_instance();
		void destroy_vk_instance();
	}
}