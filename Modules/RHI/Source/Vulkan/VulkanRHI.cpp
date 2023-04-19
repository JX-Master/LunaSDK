/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file VulkanRHI.cpp
* @author JXMaster
* @date 2022/10/27
*/
#include "VulkanRHI.hpp"
#include <Runtime/HashSet.hpp>
#include "../RHI.hpp"
#include "Device.hpp"
#include <Window/Window.hpp>
#include "Instance.hpp"
#include "Adapter.hpp"

namespace Luna
{
	namespace RHI
	{
		RV render_api_init()
		{
			VkSurfaceKHR dummy_surface = VK_NULL_HANDLE;
			Ref<Window::IWindow> dummy_window;
			lutry
			{
				luexp(create_vk_instance());
				luexp(init_physical_devices());
				lulet(main_physical_device, select_main_physical_device());
				Ref<Device> dev = new_object<Device>();
				luexp(dev->init(g_physical_devices[main_physical_device], g_physical_device_queue_families[main_physical_device]));
				g_main_device = dev;
			}
			lucatchret;
			return ok;
		}
		void render_api_close()
		{
			g_main_device.reset();
			destroy_vk_instance();
		}
	}
}