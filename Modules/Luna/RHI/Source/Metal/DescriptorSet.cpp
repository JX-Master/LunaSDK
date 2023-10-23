/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DescriptorSet.cpp
* @author JXMaster
* @date 2023/7/17
*/
#include "DescriptorSet.hpp"
#include "Resource.hpp"
namespace Luna
{
    namespace RHI
    {
        RV DescriptorSet::init(const DescriptorSetDesc& desc)
        {
            AutoreleasePool pool;
            m_layout = cast_object<DescriptorSetLayout>(desc.layout);
            // Construct bindings.
            m_bindings.assign(m_layout->m_bindings.size());
            for(usize i = 0; i < m_bindings.size(); ++i)
            {
                auto& dst = m_bindings[i];
                auto& src = m_layout->m_bindings[i];
                if(src.type != DescriptorType::sampler)
                {
                    dst.m_resources.assign(src.num_descs, nullptr);
                    switch(src.type)
                    {
                        case DescriptorType::uniform_buffer_view:
                        case DescriptorType::read_buffer_view:
                        case DescriptorType::read_texture_view:
                            dst.m_usages = MTL::ResourceUsageRead;
                            break;
                        case DescriptorType::read_write_buffer_view:
                        case DescriptorType::read_write_texture_view:
                            dst.m_usages = MTL::ResourceUsageRead | MTL::ResourceUsageWrite;
                            break;
                        default:
                            break;
                    }
                    if(test_flags(src.shader_visibility_flags, ShaderVisibilityFlag::vertex))
                    {
                        dst.m_render_stages |= MTL::RenderStageVertex;
                    }
                    if(test_flags(src.shader_visibility_flags, ShaderVisibilityFlag::pixel))
                    {
                        dst.m_render_stages |= MTL::RenderStageFragment;
                    }
                }
            }
            // Create buffer.
            if (m_device->m_support_metal_3_family)
            {
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
            else
            {
                m_encoder = box(m_device->m_device->newArgumentEncoder(m_layout->m_argument_descriptors.get()));
                if(!m_encoder)
                {
                    return BasicError::bad_platform_call();
                }
                NS::UInteger length = m_encoder->encodedLength();
                m_buffer = box(m_device->m_device->newBuffer(length, encode_resource_options(MemoryType::upload)));
                if(!m_buffer)
                {
                    return BasicError::bad_platform_call();
                }
                m_encoder->setArgumentBuffer(m_buffer.get(), 0);
            }
            return ok;
        }
        inline NSPtr<MTL::SamplerState> new_sampler_state(MTL::Device* device, const SamplerDesc& view)
        {
            NSPtr<MTL::SamplerDescriptor> sampler_desc = box(MTL::SamplerDescriptor::alloc()->init());
            sampler_desc->setMinFilter(encode_min_mag_filter(view.min_filter));
            sampler_desc->setMagFilter(encode_min_mag_filter(view.mag_filter));
            sampler_desc->setMipFilter(encode_mip_filter(view.mip_filter));
            if(view.anisotropy_enable)
            {
                sampler_desc->setMaxAnisotropy(view.max_anisotropy);
            }
            else
            {
                sampler_desc->setMaxAnisotropy(1);
            }
            if(view.compare_enable)
            {
                sampler_desc->setCompareFunction(encode_compare_function(view.compare_function));
            }
            else
            {
                sampler_desc->setCompareFunction(MTL::CompareFunctionNever);
            }
            sampler_desc->setLodMinClamp(view.min_lod);
            sampler_desc->setLodMaxClamp(view.max_lod);
            sampler_desc->setLodAverage(false);
            switch(view.border_color)
            {
                case BorderColor::float_0000:
                case BorderColor::int_0000:
                sampler_desc->setBorderColor(MTL::SamplerBorderColorTransparentBlack);
                break;
                case BorderColor::float_0001:
                case BorderColor::int_0001:
                sampler_desc->setBorderColor(MTL::SamplerBorderColorOpaqueBlack);
                break;
                case BorderColor::float_1111:
                case BorderColor::int_1111:
                sampler_desc->setBorderColor(MTL::SamplerBorderColorOpaqueWhite);
                break;
            }
            sampler_desc->setNormalizedCoordinates(true);
            sampler_desc->setSAddressMode(encode_address_mode(view.address_u));
            sampler_desc->setTAddressMode(encode_address_mode(view.address_v));
            sampler_desc->setRAddressMode(encode_address_mode(view.address_w));
            sampler_desc->setSupportArgumentBuffers(true);
            NSPtr<MTL::SamplerState> sampler = box(device->newSamplerState(sampler_desc.get()));
            return sampler;
        }
        usize DescriptorSet::calc_binding_index(u32 binding_slot) const
        {
            usize binding_index = 0;
            for(; binding_index < m_layout->m_bindings.size(); ++binding_index)
            {
                if(binding_slot == m_layout->m_bindings[binding_index].binding_slot)
                {
                    break;
                }
            }
            return binding_index;
        }
        RV DescriptorSet::update_descriptors(Span<const WriteDescriptorSet> writes)
        {
            lutry
            {
                u64* data = nullptr;
                if (m_device->m_support_metal_3_family)
                {
                    data = (u64*)m_buffer->contents();
                }
                for(auto& write : writes)
                {
                    // Find the binding record index for this write.
                    usize binding_index = calc_binding_index(write.binding_slot);
                    if(binding_index == m_layout->m_bindings.size())
                    {
                        return set_error(BasicError::bad_arguments(), "The specified binding number %d is not specified in the descriptor set layout.", write.binding_slot);
                    }
                    auto& binding = m_bindings[binding_index];
                    if (m_device->m_support_metal_3_family)
                    {
                        u64 argument_offset = m_layout->m_argument_offsets[binding_index] + write.first_array_index;
                        switch(write.type)
                        {
                            case DescriptorType::uniform_buffer_view:
                            for(usize i = 0; i < write.num_descs; ++i)
                            {
                                auto& view = write.buffer_views[i];
                                Buffer* buffer = cast_object<Buffer>(view.buffer->get_object());
                                u64 data_offset = view.first_element;
                                data[argument_offset + i] = buffer->m_buffer->gpuAddress() + data_offset;
                                binding.m_resources[write.first_array_index + i] = buffer->m_buffer.get();
                            }
                            break;
                            case DescriptorType::read_buffer_view:
                            case DescriptorType::read_write_buffer_view:
                            for(usize i = 0; i < write.num_descs; ++i)
                            {
                                auto& view = write.buffer_views[i];
                                Buffer* buffer = cast_object<Buffer>(view.buffer->get_object());
                                u64 data_offset = view.element_size * view.first_element;
                                data[argument_offset + i] = buffer->m_buffer->gpuAddress() + data_offset;
                                binding.m_resources[write.first_array_index + i] = buffer->m_buffer.get();
                            }
                            break;
                            case DescriptorType::read_texture_view:
                            case DescriptorType::read_write_texture_view:
                            for(usize i = 0; i < write.num_descs; ++i)
                            {
                                auto view = write.texture_views[i];
                                Texture* tex = cast_object<Texture>(view.texture->get_object());
                                validate_texture_view_desc(tex->m_desc, view);
                                if(require_view_object(tex->m_desc, view))
                                {
                                    lulet(tex_view, tex->get_texture_view(view));
                                    MTL::ResourceID id = tex_view->m_texture->gpuResourceID();
                                    ((MTL::ResourceID*)data)[argument_offset + i] = id;
                                }
                                else
                                {
                                    MTL::ResourceID id = tex->m_texture->gpuResourceID();
                                    ((MTL::ResourceID*)data)[argument_offset + i] = id;
                                }
                                binding.m_resources[write.first_array_index + i] = tex->m_texture.get();
                            }
                            break;
                            case DescriptorType::sampler:
                            for(usize i = 0; i < write.num_descs; ++i)
                            {
                                const SamplerDesc& view = write.samplers[i];
                                NSPtr<MTL::SamplerState> sampler = new_sampler_state(m_device->m_device.get(), view);
                                if(!sampler) return BasicError::bad_platform_call();
                                m_samplers.insert_or_assign(write.binding_slot + write.first_array_index + (u32)i, sampler);
                                ((MTL::ResourceID*)data)[argument_offset + i] = sampler->gpuResourceID();
                            }
                            break;
                        }
                    }
                    else
                    {
                        switch(write.type)
                        {
                            case DescriptorType::uniform_buffer_view:
                            if(write.num_descs == 1)
                            {
                                auto& view = write.buffer_views[0];
                                Buffer* buffer = cast_object<Buffer>(view.buffer->get_object());
                                m_encoder->setBuffer(buffer->m_buffer.get(), view.first_element, write.binding_slot + write.first_array_index);
                                binding.m_resources[write.first_array_index] = buffer->m_buffer.get();
                            }
                            else
                            {
                                MTL::Buffer** buffers = (MTL::Buffer**)alloca(sizeof(MTL::Buffer*) * write.num_descs);
                                NS::UInteger* offsets = (NS::UInteger*)alloca(sizeof(NS::UInteger) * write.num_descs);
                                for(usize i = 0; i < write.num_descs; ++i)
                                {
                                    auto& view = write.buffer_views[i];
                                    Buffer* buffer = cast_object<Buffer>(view.buffer->get_object());
                                    buffers[i] = buffer->m_buffer.get();
                                    offsets[i] = view.first_element;
                                    binding.m_resources[write.first_array_index + i] = buffer->m_buffer.get();
                                }
                                m_encoder->setBuffers(buffers, offsets, NS::Range::Make(write.binding_slot + write.first_array_index, write.num_descs));
                            }
                            break;
                            case DescriptorType::read_buffer_view:
                            case DescriptorType::read_write_buffer_view:
                            if(write.num_descs == 1)
                            {
                                auto& view = write.buffer_views[0];
                                Buffer* buffer = cast_object<Buffer>(view.buffer->get_object());
                                u64 data_offset = view.element_size * view.first_element;
                                m_encoder->setBuffer(buffer->m_buffer.get(), data_offset, write.binding_slot + write.first_array_index);
                                binding.m_resources[write.first_array_index] = buffer->m_buffer.get();
                            }
                            else
                            {
                                MTL::Buffer** buffers = (MTL::Buffer**)alloca(sizeof(MTL::Buffer*) * write.num_descs);
                                NS::UInteger* offsets = (NS::UInteger*)alloca(sizeof(NS::UInteger) * write.num_descs);
                                for(usize i = 0; i < write.num_descs; ++i)
                                {
                                    auto& view = write.buffer_views[i];
                                    Buffer* buffer = cast_object<Buffer>(view.buffer->get_object());
                                    u64 data_offset = view.element_size * view.first_element;
                                    buffers[i] = buffer->m_buffer.get();
                                    offsets[i] = data_offset;
                                    binding.m_resources[write.first_array_index + i] = buffer->m_buffer.get();
                                }
                                m_encoder->setBuffers(buffers, offsets, NS::Range::Make(write.binding_slot + write.first_array_index, write.num_descs));
                            }
                            break;
                            case DescriptorType::read_texture_view:
                            case DescriptorType::read_write_texture_view:
                            if(write.num_descs == 1)
                            {
                                auto view = write.texture_views[0];
                                Texture* tex = cast_object<Texture>(view.texture->get_object());
                                validate_texture_view_desc(tex->m_desc, view);
                                if(require_view_object(tex->m_desc, view))
                                {
                                    lulet(tex_view, tex->get_texture_view(view));
                                    m_encoder->setTexture(tex_view->m_texture.get(), write.binding_slot + write.first_array_index);
                                }
                                else
                                {
                                    m_encoder->setTexture(tex->m_texture.get(), write.binding_slot + write.first_array_index);
                                }
                                binding.m_resources[write.first_array_index] = tex->m_texture.get();
                            }
                            else
                            {
                                MTL::Texture** textures = (MTL::Texture**)alloca(sizeof(MTL::Texture*) * write.num_descs);
                                for(usize i = 0; i < write.num_descs; ++i)
                                {
                                    auto view = write.texture_views[i];
                                    Texture* tex = cast_object<Texture>(view.texture->get_object());
                                    validate_texture_view_desc(tex->m_desc, view);
                                    if(require_view_object(tex->m_desc, view))
                                    {
                                        lulet(tex_view, tex->get_texture_view(view));
                                        textures[i] = tex_view->m_texture.get();
                                    }
                                    else
                                    {
                                        textures[i] = tex->m_texture.get();
                                    }
                                    binding.m_resources[write.first_array_index + i] = tex->m_texture.get();
                                }
                                m_encoder->setTextures(textures, NS::Range::Make(write.binding_slot + write.first_array_index, write.num_descs));
                            }
                            break;
                            case DescriptorType::sampler:
                            if(write.num_descs == 1)
                            {
                                const SamplerDesc& view = write.samplers[0];
                                NSPtr<MTL::SamplerState> sampler = new_sampler_state(m_device->m_device.get(), view);
                                if(!sampler) return BasicError::bad_platform_call();
                                m_samplers.insert_or_assign(write.binding_slot + write.first_array_index, sampler);
                                m_encoder->setSamplerState(sampler.get(), write.binding_slot + write.first_array_index);
                            }
                            else
                            {
                                MTL::SamplerState** samplers = (MTL::SamplerState**)alloca(sizeof(MTL::SamplerState*) * write.num_descs);
                                for(usize i = 0; i < write.num_descs; ++i)
                                {
                                    const SamplerDesc& view = write.samplers[i];
                                    NSPtr<MTL::SamplerState> sampler = new_sampler_state(m_device->m_device.get(), view);
                                    if(!sampler) return BasicError::bad_platform_call();
                                    m_samplers.insert_or_assign(write.binding_slot + write.first_array_index + (u32)i, sampler);
                                    samplers[i] = sampler.get();
                                }
                                m_encoder->setSamplerStates(samplers, NS::Range::Make(write.binding_slot + write.first_array_index, write.num_descs));
                            }
                            break;
                        }
                    }
                }
            }
            lucatchret;
            return ok;
        }
    }
}
