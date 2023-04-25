/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file CommandQueue.hpp
* @author JXMaster
* @date 2019/8/25
*/
#pragma once
#include "CommandBuffer.hpp"
#include <Runtime/Ref.hpp>

namespace Luna
{
	namespace RHI
	{
		enum class CommandQueueFlags : u8
		{
			none = 0,
			// This command queue supports swap chain presenting.
			presenting = 0x01,
		};

		struct CommandQueueDesc
		{
			CommandQueueType type;
			CommandQueueFlags flags;
		};

		//! @interface ICommandQueue
		//! @threadsafe
		struct ICommandQueue : virtual IDeviceChild
		{
			luiid("{7d6f857a-daa4-40ee-b635-90d251a58b78}");

			virtual CommandQueueDesc get_desc() = 0;

			//! Creates a new command buffer that is bound on this queue.
			virtual R<Ref<ICommandBuffer>> new_command_buffer() = 0;

			//! Gets the GPU timestamp frequency of this queue. The timestamp frequency is measured in ticks per second.
			virtual R<f64> get_timestamp_frequency() = 0;
		};
	}
}