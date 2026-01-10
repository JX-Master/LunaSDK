/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Fence.h
* @author JXMaster
* @date 2023/8/3
*/
#pragma once
#include "Device.h"

namespace Luna
{
    namespace RHI
    {
        struct Fence : IFence
        {
            lustruct("RHI::Fence", "{37e05539-048b-41e8-8315-aa2c40da32f4}");
            luiimpl();

            Ref<Device> m_device;
            id<MTLFence> m_fence;

            RV init();

            virtual IDevice* get_device() override { return m_device; }
            virtual void set_name(const c8* name) override;
        };
    }
}