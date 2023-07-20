/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DescriptorSet.cpp
* @author JXMaster
* @date 2022/7/17
*/
#include "DescriptorSet.hpp"
namespace Luna
{
    namespace RHI
    {
        RV DescriptorSet::init(const DescriptorSetDesc& desc)
        {
            m_layout = cast_object<DescriptorSetLayout>(desc.layout);
            usize num_arguments = m_layout->m_num_arguments;
            if(test_flags(m_layout->m_flags, DescriptorSetLayoutFlag::variable_descriptors))
            {
                num_arguments += desc.num_variable_descriptors;
            }
            m_buffer = box(m_device->m_device->newBuffer(sizeof(u64) * num_arguments, encode_resource_options(MemoryType::upload)));
            if(!m_buffer)
            {
                return BasicError::bad_platform_call();
            }
            
        }
    }
}