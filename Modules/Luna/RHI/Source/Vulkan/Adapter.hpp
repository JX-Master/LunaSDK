/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Adapter.hpp
* @author JXMaster
* @date 2023/4/16
*/
#pragma once
#include "Common.hpp"
#include "../../Adapter.hpp"

namespace Luna
{
	namespace RHI
	{
		struct QueueFamily
		{
			// The index of the queue family.
			u32 index;
			// The queue desc of the queue family.
			CommandQueueDesc desc;
			// Number of physical queues in the queue family.
			u32 num_queues;
		};

		extern Vector<VkPhysicalDevice> g_physical_devices;
		extern Vector<Vector<QueueFamily>> g_physical_device_queue_families;
		extern Vector<Ref<IAdapter>> g_adapters;

		RV init_physical_devices();
		void clear_physical_devices();

		struct Adapter : IAdapter
		{
			lustruct("RHI::Adapter", "{72cf1eb1-41b1-4465-a9ca-326d1817e13e}");
			luiimpl();

			VkPhysicalDevice m_physical_device;
			VkPhysicalDeviceProperties m_device_properties;
			Vector<QueueFamily> m_queue_families;

			void init(const Vector<QueueFamily>& queue_families);
			virtual const c8* get_name() override
			{
				return m_device_properties.deviceName;
			}
		};

		R<Ref<IAdapter>> select_main_physical_device();

		struct PhysicalDeviceSurfaceInfo
		{
			VkSurfaceCapabilitiesKHR capabilities;
			Vector<VkSurfaceFormatKHR> formats;
			Vector<VkPresentModeKHR> present_modes;
		};
		PhysicalDeviceSurfaceInfo get_physical_device_surface_info(VkPhysicalDevice device, VkSurfaceKHR surface);
	}
}