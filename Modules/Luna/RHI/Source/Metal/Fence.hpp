/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Fence.hpp
* @author JXMaster
* @date 2023/8/3
*/
#pragma once
#include "Device.hpp"

namespace Luna
{
    namespace RHI
    {
        struct Fence : IFence
        {
            lustruct("RHI::Fence", "{37e05539-048b-41e8-8315-aa2c40da32f4}");
            luiimpl();

            Ref<Device> m_device;
            NSPtr<MTL::Fence> m_fence;

            RV init();

            virtual IDevice* get_device() override { return m_device; }
            virtual void set_name(const c8* name) override  { set_object_name(m_fence.get(), name); }
        };
    }
}