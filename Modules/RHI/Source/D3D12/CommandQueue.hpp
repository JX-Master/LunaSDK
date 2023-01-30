/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file CommandQueue.hpp
* @author JXMaster
* @date 2019/1/30
* @brief d3d12 implementation of ICommandQueue interface.
*/
#pragma once

#ifdef LUNA_RHI_D3D12
#include "D3D12Common.hpp"
#include "Device.hpp"
#include <Runtime/Mutex.hpp>
namespace Luna
{
	namespace RHI
	{
		inline D3D12_COMMAND_LIST_TYPE encode_command_list_type(CommandQueueType t)
		{
			switch (t)
			{
			case CommandQueueType::graphic:
				return D3D12_COMMAND_LIST_TYPE_DIRECT;
				break;
			case CommandQueueType::compute:
				return D3D12_COMMAND_LIST_TYPE_COMPUTE;
				break;
			case CommandQueueType::copy:
				return D3D12_COMMAND_LIST_TYPE_COPY;
				break;
			default:
				lupanic();
				break;
			}
			return D3D12_COMMAND_LIST_TYPE_DIRECT;
		}

		struct CommandQueue : ICommandQueue
		{
			lustruct("RHI::D3D12::CommandQueue", "{681926d0-8aaf-4766-ad37-591cf6ef428b}");
			luiimpl();

			Ref<Device> m_device;
			ComPtr<ID3D12CommandQueue> m_queue;
			CommandQueueType m_type;

			// Used by command buffers to execute in order.
			Ref<IMutex> m_mtx;

			CommandQueue(Device* device) :
				m_device(device) {}

			RV init(CommandQueueType type);
			IDevice* get_device()
			{
				return m_device.as<IDevice>();
			}
			CommandQueueType get_type()
			{
				return m_type;
			}
			RV wait_command_buffer(ICommandBuffer* command_buffer);
			R<Ref<ICommandBuffer>> new_command_buffer();
		};
	}
}

#endif