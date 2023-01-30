/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file CommandQueue.cpp
* @author JXMaster
* @date 2019/1/30
* @brief d3d12 implementation of ICommandQueue interface.
*/
#include "CommandQueue.hpp"

#ifdef LUNA_RHI_D3D12

#include "CommandBuffer.hpp"

namespace Luna
{
	namespace RHI
	{
		RV CommandQueue::init(CommandQueueType type)
		{
			m_type = type;
			ID3D12Device* dev = m_device->m_device.Get();
			D3D12_COMMAND_QUEUE_DESC d = {};
			d.NodeMask = 0;
			d.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
			d.Type = encode_command_list_type(type);
			if (FAILED(dev->CreateCommandQueue(&d, IID_PPV_ARGS(&m_queue))))
			{
				return BasicError::failure();
			}
			m_mtx = new_mutex();
			return ok;
		}
		RV CommandQueue::wait_command_buffer(ICommandBuffer* command_buffer)
		{
			CommandBuffer* buffer = static_cast<CommandBuffer*>(((IDeviceChild*)command_buffer)->get_object());
			HRESULT hr = m_queue->Wait(buffer->m_fence.Get(), buffer->m_wait_value);
			return SUCCEEDED(hr) ? ok : BasicError::bad_platform_call();
		}
		R<Ref<ICommandBuffer>> CommandQueue::new_command_buffer()
		{
			Ref<CommandBuffer> buffer = new_object<CommandBuffer>();
			lutry
			{
				buffer->m_device = m_device;
				buffer->m_queue = this;
				luexp(buffer->init());
			}
			lucatchret;
			return buffer;
		}
	}
}
#endif