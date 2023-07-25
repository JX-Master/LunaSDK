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
        RV DescriptorSet::update_descriptors(Span<const WriteDescriptorSet> writes)
        {
            lutry
            {
                u64* data = (u64*)m_buffer->contents();
                for(auto& write : writes)
                {
                    auto iter = m_layout->m_argument_offsets.find(write.binding_slot);
                    if(iter == m_layout->m_argument_offsets.end())
                    {
                        return set_error(BasicError::bad_arguments(), "The specified binding number %d is not specified in the descriptor set layout.", write.binding_slot);
                    }
                    u64 offset = iter->second + write.first_array_index;
                    switch(type)
                    {
                        case DescriptorType::uniform_buffer_view:
                        case DescriptorType::read_buffer_view:
                        case DescriptorType::read_write_buffer_view:
                        for(usize i = 0; i < write.buffer_views.size(); ++i)
                        {
                            auto& view = write.buffer_views[i];
                            Buffer* buffer = cast_object<Buffer>(view.buffer->get_object());
                            u64 data_offset = view.format == Format::unknown ? view.element_size * view.first_element : bits_per_pixel(view.format) * view.first_element / 8;
                            data[offset + i] = buffer->m_buffer->gpuAddress() + data_offset;
                        }
                        break;
                        case DescriptorType::read_texture_view:
                        case DescriptorType::read_write_texture_view:
                        for(usize i = 0; i < write.texture_views.size(); ++i)
                        {
                            auto view = write.texture_views[i];
                            Texture* tex = cast_object<Texture>(view.texture->get_object());
                            validate_texture_view_desc(tex->m_desc, view);
                            if(require_view_object(tex->m_desc, view))
                            {
                                Ref<TextureView> tex_view = tex->get_texture_view(view);
                                MTL::ResourceID id = tex_view->m_texture->gpuResourceID();
                                ((MTL::ResourceID*)data)[offset + i] = id;
                            }
                            else
                            {
                                MTL::ResourceID id = tex->m_texture->gpuResourceID();
                                ((MTL::ResourceID*)data)[offset + i] = id;
                            }
                        }
                        break;
                        case DescriptorType::sampler:
                        for(usize i = 0; i < write.samplers.size(); ++i)
                        {
                            
                        }
                        break;
                    }

                }
            }
            lucatchret;
            return ok;
        }
    }
}