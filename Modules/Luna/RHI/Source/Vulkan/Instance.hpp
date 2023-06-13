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

		//! Whether the Vulkan validation layer is enabled.
		extern bool g_enable_validation_layer;

		//! The enabled Vulkan layers.
		extern Vector<const c8*> g_enabled_layers;

		RV create_vk_instance();
		void destroy_vk_instance();
	}
}