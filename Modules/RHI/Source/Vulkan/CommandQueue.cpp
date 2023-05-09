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
		bool CommandQueue::acquire_queue(CommandQueueType type, CommandQueueFlags flags)
		{
			for (auto& pool : m_device->m_queue_pools)
			{
				if (pool.desc.type == type && !pool.free_queues.empty())
				{
					if (!test_flags(flags, CommandQueueFlags::presenting) ||
						test_flags(pool.desc.flags, CommandQueueFlags::presenting))
					{
						m_queue = pool.free_queues.back();
						pool.free_queues.pop_back();
						m_desc = pool.desc;
						m_queue_family_index = pool.queue_family_index;
						return true;
					}
				}
			}
			return false;
		}
		RV CommandQueue::init(const CommandQueueDesc& desc)
		{
			m_mtx = new_mutex();
			MutexGuard guard(m_device->m_queue_pool_mtx);
			bool queue_acquired = acquire_queue(desc.type, desc.flags);
			if (!queue_acquired)
			{
				if (desc.type == CommandQueueType::compute)
				{
					// fallback to graphics.
					queue_acquired = acquire_queue(CommandQueueType::graphics, desc.flags);
				}
				else if (desc.type == CommandQueueType::copy)
				{
					// fallback to graphics and compute.
					// graphics queue have better copy performance than compute queue, so we choose graphics first.
					queue_acquired = acquire_queue(CommandQueueType::graphics, desc.flags);
					if (!queue_acquired) queue_acquired = acquire_queue(CommandQueueType::compute, desc.flags);
				}
			}
			return queue_acquired ? ok : set_error(BasicError::out_of_resource(), "Command Queue allocation failed because all VkQueues are in use.");
		}
		void CommandQueue::init_as_internal(const QueuePool& queue_pool)
		{
			m_mtx = queue_pool.internal_queue_mtx;
			m_queue = queue_pool.internal_queue;
			m_queue_family_index = queue_pool.queue_family_index;
			m_desc = queue_pool.desc;
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