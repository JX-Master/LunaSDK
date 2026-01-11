/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DescriptorSetLayout.mm
* @author JXMaster
* @date 2023/7/13
*/
#include "DescriptorSetLayout.h"

namespace Luna
{
    namespace RHI
    {
        inline MTLTextureType encode_descriptor_set_texture_type(TextureViewType type)
        {
            switch(type)
            {
                case TextureViewType::tex1d:
                    return MTLTextureType1D;
                case TextureViewType::tex1darray:
                    return MTLTextureType1DArray;
                case TextureViewType::tex2d:
                    return MTLTextureType2D;
                case TextureViewType::tex2darray:
                    return MTLTextureType2DArray;
                case TextureViewType::tex2dms:
                    return MTLTextureType2DMultisample;
                case TextureViewType::tex2dmsarray:
                    return MTLTextureType2DMultisampleArray;
                case TextureViewType::texcube:
                    return MTLTextureTypeCube;
                case TextureViewType::texcubearray:
                    return MTLTextureTypeCubeArray;
                case TextureViewType::tex3d:
                    return MTLTextureType3D;
                default:
                    lupanic();
                    return MTLTextureType2D;
            }
        }
        RV DescriptorSetLayout::init(const DescriptorSetLayoutDesc& desc)
        {
            @autoreleasepool
            {
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
                    NSMutableArray<MTLArgumentDescriptor*>* argument_descriptors = [NSMutableArray arrayWithCapacity: m_bindings.size()];
                    for(usize i = 0; i < m_bindings.size(); ++i)
                    {
                        MTLArgumentDescriptor* dst = [[MTLArgumentDescriptor alloc]init];
                        const DescriptorSetLayoutBinding& src = m_bindings[i];
                        dst.index = src.binding_slot;
                        dst.arrayLength = src.num_descs;
                        switch(src.type)
                        {
                            case DescriptorType::uniform_buffer_view:
                            case DescriptorType::read_buffer_view:
                                dst.access = MTLBindingAccessReadOnly;
                                dst.dataType = MTLDataTypePointer;
                                break;
                            case DescriptorType::read_texture_view:
                                dst.access = MTLBindingAccessReadOnly;
                                dst.dataType = MTLDataTypeTexture;
                                dst.textureType = encode_descriptor_set_texture_type(src.texture_view_type);
                                break;
                            case DescriptorType::sampler:
                                dst.access = MTLBindingAccessReadOnly;
                                dst.dataType = MTLDataTypeSampler;
                                break;
                            case DescriptorType::read_write_buffer_view:
                                dst.access = MTLBindingAccessReadWrite;
                                dst.dataType = MTLDataTypePointer;
                                break;
                            case DescriptorType::read_write_texture_view:
                                dst.access = MTLBindingAccessReadWrite;
                                dst.dataType = MTLDataTypeTexture;
                                dst.textureType = encode_descriptor_set_texture_type(src.texture_view_type);
                                break;
                        }
                        [argument_descriptors addObject:dst];
                    }
                    m_argument_descriptors = argument_descriptors;
                }
                return ok;
            }
        }
    }
}
