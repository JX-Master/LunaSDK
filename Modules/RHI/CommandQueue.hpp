/*
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
		//! @interface ICommandQueue
		//! @threadsafe
		struct ICommandQueue : virtual IDeviceChild
		{
			luiid("{7d6f857a-daa4-40ee-b635-90d251a58b78}");

			virtual CommandQueueType get_type() = 0;

			//! Tells the command queue to wait for the specified command buffer being finished by another 
			//! command queue before proceeding to the next submitted command buffer.
			//! 
			//! The command buffer can be in recording state or submitted state. The client code should never waits for a 
			//! command buffer submitted to the same queue to prevent dead locking the queue.
			virtual RV wait_command_buffer(ICommandBuffer* command_buffer) = 0;

			//! Creates a new command buffer that is bound on this queue.
			virtual R<Ref<ICommandBuffer>> new_command_buffer() = 0;
		};
	}
}