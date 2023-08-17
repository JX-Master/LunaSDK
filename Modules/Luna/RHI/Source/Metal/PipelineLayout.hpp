/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file PipelineLayout.hpp
* @author JXMaster
* @date 2023/8/1
*/
#pragma once
#include "Device.hpp"

namespace Luna
{
    namespace RHI
    {
        struct PipelineLayout : IPipelineLayout
        {
            lustruct("RHI::PipelineLayout", "{34da167c-867d-424a-bc6d-52c219c283eb}");
            luiimpl();

            Ref<Device> m_device;

            RV init(const PipelineLayoutDesc& desc)
            {
                return ok;
            }

            virtual IDevice* get_device() override { return m_device; }
            virtual void set_name(const c8* name) override  {}

            // Metal does not have pipeline layout object, so we left this object empty.
        };
    }
}