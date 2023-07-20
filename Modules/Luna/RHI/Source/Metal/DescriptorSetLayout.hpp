/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DescriptorSetLayout.hpp
* @author JXMaster
* @date 2022/7/13
*/
#pragma once
#include "Device.hpp"
namespace Luna
{
    namespace RHI
    {
        struct DescriptorSetLayout : IDescriptorSetLayout
        {
            lustruct("RHI::DescriptorSetLayout", "{14d4d247-2ff3-4361-bd29-8a6b83241ead}");
            luiimpl();

            Array<DescriptorSetLayoutBinding> m_bindings;
            DescriptorSetLayoutFlag m_flags;
            // Map from binding number to argument index.
            Array<Pair<u64, u64>> m_argument_offsets;
            usize m_num_arguments;

            RV init(const DescriptorSetLayoutDesc& desc);
        };
    }
}