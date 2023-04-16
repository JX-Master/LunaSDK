// Copyright 2018-2022 JXMaster. All rights reserved.
/*
* @file Device.hpp
* @author JXMaster
* @date 2022/10/27
*/
#pragma once
#include "VulkanRHI.hpp"
#include <Runtime/Mutex.hpp>
namespace Luna
{
	namespace RHI
	{
		//! Records all created command queue.
		struct CommandQueueHandle
		{
			VkQueue m_queue;
			bool m_used = false;
		};

		struct Device
		{
			lustruct("RHI::Device", "{9C0F7754-FA08-4FF3-BF66-B23125FA19F9}");

			VkDevice m_device;
			VkPhysicalDevice m_physical_device;
			// All created queues.
			Vector<CommandQueueHandle> m_graphic_queues;
			Vector<CommandQueueHandle> m_compute_queues;
			Vector<CommandQueueHandle> m_copy_queues;

			Ref<IMutex> m_mtx;

			RV init(VkPhysicalDevice physical_device, const Vector<QueueFamily>& queue_families);

			~Device()
			{
				if (m_device != VK_NULL_HANDLE)
				{
					vkDestroyDevice(m_device, nullptr);
					m_device = nullptr;
				}
			}
		};
	}
}