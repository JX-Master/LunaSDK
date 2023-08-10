/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DescriptorSetLayout.cpp
* @author JXMaster
* @date 2023/7/13
*/
#include "DescriptorSetLayout.hpp"

namespace Luna
{
    namespace RHI
    {
        RV DescriptorSetLayout::init(const DescriptorSetLayoutDesc& desc)
        {
            m_bindings.assign_n(desc.bindings.data(), desc.bindings.size());
            m_flags = desc.flags;
            m_argument_offsets.assign(m_bindings.size());
            m_num_arguments = 0;
            for(usize i = 0; i < m_bindings.size(); ++i)
            {
                m_argument_offsets[i] = m_num_arguments;
                if(!(test_flags(m_flags, DescriptorSetLayoutFlag::variable_descriptors) && i == m_bindings.size() - 1))
                {
                    m_num_arguments += m_bindings[i].num_descs;
                }
            }
            return ok;
        }
    }
}
