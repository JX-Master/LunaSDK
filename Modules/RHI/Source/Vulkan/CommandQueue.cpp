// Copyright 2018-2022 JXMaster. All rights reserved.
/*
* @file CommandQueue.cpp
* @author JXMaster
* @date 2022/10/29
*/
#include "CommandQueue.hpp"
namespace Luna
{
	namespace RHI
	{
		RV CommandQueue::init(CommandQueueType type)
		{
			MutexGuard guard(m_device->m_mtx);
			m_queue = VK_NULL_HANDLE;
			if (type == CommandQueueType::graphic)
			{
				for (auto& i : m_device->m_graphic_queues)
				{
					if (!i.m_used)
					{
						m_queue = i.m_queue;
						i.m_used = true;
						break;
					}
				}
			}
			else if (type == CommandQueueType::compute)
			{
				for (auto& i : m_device->m_compute_queues)
				{
					if (!i.m_used)
					{
						m_queue = i.m_queue;
						i.m_used = true;
						break;
					}
				}
				if (m_queue == VK_NULL_HANDLE)
				{
					// Fallback to graphic queue.
					for (auto& i : m_device->m_graphic_queues)
					{
						if (!i.m_used)
						{
							m_queue = i.m_queue;
							i.m_used = true;
							break;
						}
					}
				}
			}
			else if (type == CommandQueueType::copy)
			{
				for (auto& i : m_device->m_copy_queues)
				{
					if (!i.m_used)
					{
						m_queue = i.m_queue;
						i.m_used = true;
						break;
					}
				}
				if (m_queue == VK_NULL_HANDLE)
				{
					// Fallback to compute queue.
					for (auto& i : m_device->m_compute_queues)
					{
						if (!i.m_used)
						{
							m_queue = i.m_queue;
							i.m_used = true;
							break;
						}
					}
				}
				if (m_queue == VK_NULL_HANDLE)
				{
					// Fallback to graphic queue.
					for (auto& i : m_device->m_graphic_queues)
					{
						if (!i.m_used)
						{
							m_queue = i.m_queue;
							i.m_used = true;
							break;
						}
					}
				}
			}
			if (m_queue == VK_NULL_HANDLE)
			{
				return set_error(BasicError::out_of_resource(), "Command Queue allocation failed because all VkQueues are in use.");
			}
			return ok;
		}
	}
}