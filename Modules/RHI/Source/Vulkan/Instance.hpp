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
		//! The vulkan instance version.
		extern u32 g_vk_version;
		//! The global Vulkan instance.
		extern VkInstance g_vk_instance;

		constexpr u32 VULKAN_1_0 = VK_MAKE_API_VERSION(0, 1, 0, 0);
		constexpr u32 VULKAN_1_1 = VK_MAKE_API_VERSION(0, 1, 1, 0);
		constexpr u32 VULKAN_1_2 = VK_MAKE_API_VERSION(0, 1, 2, 0);
		constexpr u32 VULKAN_1_3 = VK_MAKE_API_VERSION(0, 1, 3, 0);

		//! Whether the Vulkan validation layer is enabled.
		extern bool g_enable_validation_layer;

		//! The enabled Vulkan layers.
		extern Vector<const c8*> g_enabled_layers;

		RV create_vk_instance();
		void destroy_vk_instance();
	}
}