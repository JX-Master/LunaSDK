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
            Vector<Pair<u64, u64>> argument_offsets;
            argument_offsets.reserve(m_bindings.size());
            usize m_num_arguments = 0;
            for(usize i = 0; i < m_bindings.size(); ++i)
            {
                Pair<u64, u64> b;
                b.first = m_bindings[i].binding_slot;
                b.second = m_num_arguments;
                argument_offsets.push_back(b);
                if(!(test_flags(m_flags, DescriptorSetLayoutFlag::variable_descriptors) && i == m_bindings.size() - 1))
                {
                    m_num_arguments += m_bindings[i].num_descs;
                }
            }
            m_argument_offsets.assign_n(argument_offsets.data(), argument_offsets.size());
            return ok;
        }
    }
}