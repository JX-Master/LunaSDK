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
			MutexGuard guard(m_device->m_queue_pool_mtx);
			m_queue = VK_NULL_HANDLE;
			if (desc.type == CommandQueueType::copy)
			{
				// Check copy queue.
				for (auto& pool : m_device->m_queue_pools)
				{
					if (pool.desc.type == CommandQueueType::copy && !pool.free_queues.empty())
					{
						if (!test_flags(desc.flags, CommandQueueFlags::presenting) ||
							test_flags(pool.desc.flags, CommandQueueFlags::presenting))
						{
							m_queue = pool.free_queues.back();
							pool.free_queues.pop_back();
							m_desc = pool.desc;
							m_queue_family_index = pool.queue_family_index;
							return ok;
						}
					}
				}
			}
			if (desc.type == CommandQueueType::copy || desc.type == CommandQueueType::compute)
			{
				// Check compute queue.
				for (auto& pool : m_device->m_queue_pools)
				{
					if (pool.desc.type == CommandQueueType::compute && !pool.free_queues.empty())
					{
						if (!test_flags(desc.flags, CommandQueueFlags::presenting) ||
							test_flags(pool.desc.flags, CommandQueueFlags::presenting))
						{
							m_queue = pool.free_queues.back();
							pool.free_queues.pop_back();
							m_desc = pool.desc;
							m_queue_family_index = pool.queue_family_index;
							return ok;
						}
					}
				}
			}
			{
				// Check graphics queue.
				for (auto& pool : m_device->m_queue_pools)
				{
					if (pool.desc.type == CommandQueueType::graphics && !pool.free_queues.empty())
					{
						if (!test_flags(desc.flags, CommandQueueFlags::presenting) ||
							test_flags(pool.desc.flags, CommandQueueFlags::presenting))
						{
							m_queue = pool.free_queues.back();
							pool.free_queues.pop_back();
							m_desc = pool.desc;
							m_queue_family_index = pool.queue_family_index;
							return ok;
						}
					}
				}
			}
			return set_error(BasicError::out_of_resource(), "Command Queue allocation failed because all VkQueues are in use.");
		}
		CommandQueue::~CommandQueue()
		{
			MutexGuard guard(m_device->m_queue_pool_mtx);
			for (auto& pool : m_device->m_queue_pools)
			{
				if (m_queue_family_index == pool.queue_family_index)
				{
					pool.free_queues.push_back(m_queue);
					break;
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