/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file CommandBuffer.hpp
* @author JXMaster
* @date 2022/7/13
*/
#pragma once
#include "Device.hpp"

namespace Luna
{
    namespace RHI
    {
        struct CommandBuffer : ICommandBuffer
        {
            lustruct("RHI::CommandBuffer", "{da3d7c91-2ae4-407e-81c6-276089faeb40}");
            luiimpl();

            Ref<Device> m_device;
            NSPtr<MTL::CommandBuffer> m_buffer;
            u32 m_command_queue_index;

            // The attached graphic objects.
			Vector<Ref<IDeviceChild>> m_objs;

            NSPtr<MTL::RenderCommandEncoder> m_render;
            NSPtr<MTL::ComputeCommandEncoder> m_compute;
            NSPtr<MTL::BlitCommandEncoder> m_blit;

            RV init(u32 command_queue_index);

            virtual IDevice* get_device() override { return m_device; }
            virtual void set_name(const Name& name) override  { set_object_name(m_buffer.get(), name); }
            virtual void wait() override;
			virtual bool try_wait() override;
			virtual u32 get_command_queue_index() override { return m_command_queue_index; }
            virtual RV reset() override;
			virtual void attach_device_object(IDeviceChild* obj) override;
			virtual void begin_event(const Name& event_name) override;
			virtual void end_event() override;
            
        };
    }
}