/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file CommandBuffer.cpp
* @author JXMaster
* @date 2022/7/13
*/
#include "CommandBuffer.hpp"

namespace Luna
{
    namespace RHI
    {
        RV CommandBuffer::init(u32 command_queue_index)
        {
            AutoreleasePool pool;
            m_command_queue_index = command_queue_index;
            m_buffer = retain(m_device->m_queues[command_queue_index].queue->commandBuffer());
            if(!m_buffer) return BasicError::bad_platform_call();
            return ok;
        }
        void CommandBuffer::wait()
        {
            m_buffer->waitUntilCompleted();
        }
        bool CommandBuffer::try_wait()
        {
            auto status = m_buffer->status();
            return status == MTL::CommandBufferStatusCompleted || status == MTL::CommandBufferStatusError;
        }
        RV CommandBuffer::reset()
        {
            AutoreleasePool pool;
            m_objs.clear();
            m_buffer = retain(m_device->m_queues[m_command_queue_index].queue->commandBuffer());
            if(!m_buffer) return BasicError::bad_platform_call();
            return ok;
        }
        void CommandBuffer::attach_device_object(IDeviceChild* obj)
        {
            m_objs.push_back(obj);
        }
        void CommandBuffer::begin_event(const Name& event_name)
        {
            AutoreleasePool pool;
            NS::String* string = NS::String::string(event_name.c_str(), NS::StringEncoding::UTF8StringEncoding);
            m_buffer->pushDebugGroup(string);
        }
        void CommandBuffer::end_event()
        {
            m_buffer->popDebugGroup();
        }
        
    }
}