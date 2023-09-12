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
        inline MTL::TextureType encode_descriptor_set_texture_type(TextureViewType type)
        {
            switch(type)
            {
                case TextureViewType::tex1d:
                    return MTL::TextureType1D;
                case TextureViewType::tex1darray:
                    return MTL::TextureType1DArray;
                case TextureViewType::tex2d:
                    return MTL::TextureType2D;
                case TextureViewType::tex2darray:
                    return MTL::TextureType2DArray;
                case TextureViewType::tex2dms:
                    return MTL::TextureType2DMultisample;
                case TextureViewType::tex2dmsarray:
                    return MTL::TextureType2DMultisampleArray;
                case TextureViewType::texcube:
                    return MTL::TextureTypeCube;
                case TextureViewType::texcubearray:
                    return MTL::TextureTypeCubeArray;
                case TextureViewType::tex3d:
                    return MTL::TextureType3D;
                default:
                    lupanic();
                    return MTL::TextureType2D;
            }
        }
        RV DescriptorSetLayout::init(const DescriptorSetLayoutDesc& desc)
        {
            AutoreleasePool pool;
            m_bindings.assign_n(desc.bindings.data(), desc.bindings.size());
            m_flags = desc.flags;
            if(m_device->m_support_metal_3_family)
            {
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
            }
            else
            {
                Array<NSPtr<MTL::ArgumentDescriptor>> argument_descriptors(m_bindings.size());
                for(usize i = 0; i < m_bindings.size(); ++i)
                {
                    argument_descriptors[i] = box(MTL::ArgumentDescriptor::alloc()->init());
                    MTL::ArgumentDescriptor* dst = argument_descriptors[i].get();
                    const DescriptorSetLayoutBinding& src = m_bindings[i];
                    dst->setIndex(src.binding_slot);
                    dst->setArrayLength(src.num_descs);
                    switch(src.type)
                    {
                        case DescriptorType::uniform_buffer_view:
                        case DescriptorType::read_buffer_view:
                            dst->setAccess(MTL::ArgumentAccessReadOnly);
                            dst->setDataType(MTL::DataTypePointer);
                            break;
                        case DescriptorType::read_texture_view:
                            dst->setAccess(MTL::ArgumentAccessReadOnly);
                            dst->setDataType(MTL::DataTypeTexture);
                            dst->setTextureType(encode_descriptor_set_texture_type(src.texture_view_type));
                            break;
                        case DescriptorType::sampler:
                            dst->setAccess(MTL::ArgumentAccessReadOnly);
                            dst->setDataType(MTL::DataTypeSampler);
                            break;
                        case DescriptorType::read_write_buffer_view:
                            dst->setAccess(MTL::ArgumentAccessReadWrite);
                            dst->setDataType(MTL::DataTypePointer);
                            break;
                        case DescriptorType::read_write_texture_view:
                            dst->setAccess(MTL::ArgumentAccessReadWrite);
                            dst->setDataType(MTL::DataTypeTexture);
                            dst->setTextureType(encode_descriptor_set_texture_type(src.texture_view_type));
                            break;
                    }
                }
                Array<NS::Object*> arguments(argument_descriptors.size());
                for(usize i = 0; i < m_bindings.size(); ++i)
                {
                    arguments[i] = argument_descriptors[i].get();
                }
                m_argument_descriptors = retain(NS::Array::array(arguments.data(), arguments.size()));
            }
            return ok;
        }
    }
}
