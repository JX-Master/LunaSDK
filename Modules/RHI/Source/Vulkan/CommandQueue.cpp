/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file CommandQueue.cpp
* @author JXMaster
* @date 2022/10/29
*/
#include "CommandQueue.hpp"
#include "CommandBuffer.hpp"
namespace Luna
{
	namespace RHI
	{
		RV CommandQueue::init(const CommandQueueDesc& desc)
		{
			m_mtx = new_mutex();
			MutexGuard guard(m_device->m_mtx);
			m_queue = VK_NULL_HANDLE;
			m_desc = desc;
			for (usize i = 0; i < m_device->m_queues.size(); ++i)
			{
				if (!m_device->m_queue_allocated[i])
				{
					if (!test_flags(desc.flags, CommandQueueFlags::presenting) ||
						test_flags(m_device->m_queues[i].desc.flags, CommandQueueFlags::presenting))
					{
						m_queue = m_device->m_queues[i].queue;
						m_device->m_queue_allocated[i] = true;
						return ok;
					}
				}
			}
			return set_error(BasicError::out_of_resource(), "Command Queue allocation failed because all VkQueues are in use.");
		}
		CommandQueue::~CommandQueue()
		{
			MutexGuard guard(m_device->m_mtx);
			for (usize i = 0; i < m_device->m_queues.size(); ++i)
			{
				if (m_queue == m_device->m_queues[i].queue)
				{
					m_device->m_queue_allocated[i] = false;
				}
			}
		}
		R<Ref<ICommandBuffer>> CommandQueue::new_command_buffer()
		{
			Ref<ICommandBuffer> ret;
			lutry
			{
				auto buf = new_object<CommandBuffer>();
				luexp(buf->init(this));
				ret = buf;
			}
			lucatchret;
			return ret;
		}
		R<f64> CommandQueue::get_timestamp_frequency()
		{
			// nanoseconds per tick.
			f64 period = m_device->m_physical_device_properties.limits.timestampPeriod;
			return 1000000000.0 / period;
		}
	}
}