/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DescriptorSetLayout.hpp
* @author JXMaster
* @date 2023/7/13
*/
#pragma once
#include "Device.hpp"
#include <Luna/Runtime/Array.hpp>
namespace Luna
{
    namespace RHI
    {
        struct DescriptorSetLayout : IDescriptorSetLayout
        {
            lustruct("RHI::DescriptorSetLayout", "{14d4d247-2ff3-4361-bd29-8a6b83241ead}");
            luiimpl();

            Ref<Device> m_device;
            Array<DescriptorSetLayoutBinding> m_bindings;
            DescriptorSetLayoutFlag m_flags;
            // The argument offfset for every binding. Used for metal 3.
            Array<u64> m_argument_offsets;
            usize m_num_arguments;
            // The argument descriptors. Used for metal 2.
            NSPtr<NS::Array> m_argument_descriptors;

            RV init(const DescriptorSetLayoutDesc& desc);

            virtual IDevice* get_device() override { return m_device; }
            virtual void set_name(const c8* name) override  { }
        };
    }
}
