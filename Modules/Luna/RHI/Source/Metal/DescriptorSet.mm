/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DescriptorSet.mm
* @author JXMaster
* @date 2023/7/17
*/
#include "DescriptorSet.h"
#include "Resource.h"
#include <Luna/Runtime/StackAllocator.hpp>

namespace Luna
{
    namespace RHI
    {
        RV DescriptorSet::init(const DescriptorSetDesc& desc)
        {
            @autoreleasepool
            {
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
                                dst.m_usages = MTLResourceUsageRead;
                                break;
                            case DescriptorType::read_write_buffer_view:
                            case DescriptorType::read_write_texture_view:
                                dst.m_usages = MTLResourceUsageRead | MTLResourceUsageWrite;
                                break;
                            default:
                                break;
                        }
                        if(test_flags(src.shader_visibility_flags, ShaderVisibilityFlag::vertex))
                        {
                            dst.m_render_stages |= MTLRenderStageVertex;
                        }
                        if(test_flags(src.shader_visibility_flags, ShaderVisibilityFlag::pixel))
                        {
                            dst.m_render_stages |= MTLRenderStageFragment;
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
                    m_buffer = [m_device->m_device newBufferWithLength:sizeof(u64) * num_arguments 
                        options:encode_resource_options(MemoryType::upload)];
                    if(!m_buffer)
                    {
                        return BasicError::bad_platform_call();
                    }
                }
                else
                {
                    m_encoder = [m_device->m_device newArgumentEncoderWithArguments:m_layout->m_argument_descriptors];
                    if(!m_encoder)
                    {
                        return BasicError::bad_platform_call();
                    }
                    NSUInteger length = [m_encoder encodedLength];
                    m_buffer = [m_device->m_device newBufferWithLength:length options:encode_resource_options(MemoryType::upload)];
                    if(!m_buffer)
                    {
                        return BasicError::bad_platform_call();
                    }
                    [m_encoder setArgumentBuffer:m_buffer offset:0];
                }
                return ok;
            }
        }
        inline id<MTLSamplerState> new_sampler_state(id<MTLDevice> device, const SamplerDesc& view)
        {
            @autoreleasepool
            {
                MTLSamplerDescriptor* sampler_desc = [[MTLSamplerDescriptor alloc]init];
                sampler_desc.minFilter = encode_min_mag_filter(view.min_filter);
                sampler_desc.magFilter = encode_min_mag_filter(view.mag_filter);
                sampler_desc.mipFilter = encode_mip_filter(view.mip_filter);
                if(view.anisotropy_enable)
                {
                    sampler_desc.maxAnisotropy = view.max_anisotropy;
                }
                else
                {
                    sampler_desc.maxAnisotropy = 1;
                }
                if(view.compare_enable)
                {
                    sampler_desc.compareFunction = encode_compare_function(view.compare_function);
                }
                else
                {
                    sampler_desc.compareFunction = MTLCompareFunctionNever;
                }
                sampler_desc.lodMinClamp = view.min_lod;
                sampler_desc.lodMaxClamp = view.max_lod;
                sampler_desc.lodAverage = NO;
                switch(view.border_color)
                {
                    case BorderColor::float_0000:
                    case BorderColor::int_0000:
                    sampler_desc.borderColor = MTLSamplerBorderColorTransparentBlack;
                    break;
                    case BorderColor::float_0001:
                    case BorderColor::int_0001:
                    sampler_desc.borderColor = MTLSamplerBorderColorOpaqueBlack;
                    break;
                    case BorderColor::float_1111:
                    case BorderColor::int_1111:
                    sampler_desc.borderColor = MTLSamplerBorderColorOpaqueWhite;
                    break;
                }
                sampler_desc.normalizedCoordinates = YES;
                sampler_desc.sAddressMode = encode_address_mode(view.address_u);
                sampler_desc.tAddressMode = encode_address_mode(view.address_v);
                sampler_desc.rAddressMode = encode_address_mode(view.address_w);
                sampler_desc.supportArgumentBuffers = YES;
                id<MTLSamplerState> sampler = [device newSamplerStateWithDescriptor:sampler_desc];
                return sampler;
            }
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
        void DescriptorSet::set_name(const c8* name)
        {
            @autoreleasepool
            {
                NSString* label = [NSString stringWithUTF8String:name];
                m_buffer.label = label;
            }
        }
        RV DescriptorSet::update_descriptors(Span<const WriteDescriptorSet> writes)
        {
            StackAllocator salloc;
            lutry
            {
                u64* data = nullptr;
                if (m_device->m_support_metal_3_family)
                {
                    data = (u64*)[m_buffer contents];
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
                                data[argument_offset + i] = buffer->m_buffer.gpuAddress + data_offset;
                                binding.m_resources[write.first_array_index + i] = buffer->m_buffer;
                            }
                            break;
                            case DescriptorType::read_buffer_view:
                            case DescriptorType::read_write_buffer_view:
                            for(usize i = 0; i < write.num_descs; ++i)
                            {
                                auto& view = write.buffer_views[i];
                                Buffer* buffer = cast_object<Buffer>(view.buffer->get_object());
                                u64 data_offset = view.element_size * view.first_element;
                                data[argument_offset + i] = buffer->m_buffer.gpuAddress + data_offset;
                                binding.m_resources[write.first_array_index + i] = buffer->m_buffer;
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
                                    MTLResourceID id = tex_view->m_texture.gpuResourceID;
                                    ((MTLResourceID*)data)[argument_offset + i] = id;
                                }
                                else
                                {
                                    MTLResourceID id = tex->m_texture.gpuResourceID;
                                    ((MTLResourceID*)data)[argument_offset + i] = id;
                                }
                                binding.m_resources[write.first_array_index + i] = tex->m_texture;
                            }
                            break;
                            case DescriptorType::sampler:
                            for(usize i = 0; i < write.num_descs; ++i)
                            {
                                const SamplerDesc& view = write.samplers[i];
                                id<MTLSamplerState> sampler = new_sampler_state(m_device->m_device, view);
                                if(!sampler) return BasicError::bad_platform_call();
                                m_samplers.insert_or_assign(write.binding_slot + write.first_array_index + (u32)i, sampler);
                                ((MTLResourceID*)data)[argument_offset + i] = sampler.gpuResourceID;
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
                                [m_encoder setBuffer:buffer->m_buffer offset:view.first_element atIndex:write.binding_slot + write.first_array_index];
                                binding.m_resources[write.first_array_index] = buffer->m_buffer;
                            }
                            else
                            {
                                id<MTLBuffer> __unsafe_unretained* buffers = (id<MTLBuffer> __unsafe_unretained*)salloc.allocate(sizeof(id<MTLBuffer>) * write.num_descs);
                                NSUInteger* offsets = (NSUInteger*)salloc.allocate(sizeof(NSUInteger) * write.num_descs);
                                for(usize i = 0; i < write.num_descs; ++i)
                                {
                                    auto& view = write.buffer_views[i];
                                    Buffer* buffer = cast_object<Buffer>(view.buffer->get_object());
                                    buffers[i] = buffer->m_buffer;
                                    offsets[i] = view.first_element;
                                    binding.m_resources[write.first_array_index + i] = buffer->m_buffer;
                                }
                                NSRange range;
                                range.location = write.binding_slot + write.first_array_index;
                                range.length = write.num_descs;
                                [m_encoder setBuffers:buffers offsets:offsets withRange:range];
                            }
                            break;
                            case DescriptorType::read_buffer_view:
                            case DescriptorType::read_write_buffer_view:
                            if(write.num_descs == 1)
                            {
                                auto& view = write.buffer_views[0];
                                Buffer* buffer = cast_object<Buffer>(view.buffer->get_object());
                                u64 data_offset = view.element_size * view.first_element;
                                [m_encoder setBuffer:buffer->m_buffer offset:data_offset atIndex:write.binding_slot + write.first_array_index];
                                binding.m_resources[write.first_array_index] = buffer->m_buffer;
                            }
                            else
                            {
                                id<MTLBuffer> __unsafe_unretained* buffers = (id<MTLBuffer> __unsafe_unretained*)salloc.allocate(sizeof(id<MTLBuffer>) * write.num_descs);
                                NSUInteger* offsets = (NSUInteger*)salloc.allocate(sizeof(NSUInteger) * write.num_descs);
                                for(usize i = 0; i < write.num_descs; ++i)
                                {
                                    auto& view = write.buffer_views[i];
                                    Buffer* buffer = cast_object<Buffer>(view.buffer->get_object());
                                    u64 data_offset = view.element_size * view.first_element;
                                    buffers[i] = buffer->m_buffer;
                                    offsets[i] = data_offset;
                                    binding.m_resources[write.first_array_index + i] = buffer->m_buffer;
                                }
                                NSRange range;
                                range.location = write.binding_slot + write.first_array_index;
                                range.length = write.num_descs;
                                [m_encoder setBuffers:buffers offsets:offsets withRange:range];
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
                                    [m_encoder setTexture:tex_view->m_texture atIndex:write.binding_slot + write.first_array_index];
                                }
                                else
                                {
                                    [m_encoder setTexture:tex->m_texture atIndex:write.binding_slot + write.first_array_index];
                                }
                                binding.m_resources[write.first_array_index] = tex->m_texture;
                            }
                            else
                            {
                                id<MTLTexture> __unsafe_unretained* textures = (id<MTLTexture> __unsafe_unretained*)salloc.allocate(sizeof(id<MTLTexture>) * write.num_descs);
                                for(usize i = 0; i < write.num_descs; ++i)
                                {
                                    auto view = write.texture_views[i];
                                    Texture* tex = cast_object<Texture>(view.texture->get_object());
                                    validate_texture_view_desc(tex->m_desc, view);
                                    if(require_view_object(tex->m_desc, view))
                                    {
                                        lulet(tex_view, tex->get_texture_view(view));
                                        textures[i] = tex_view->m_texture;
                                    }
                                    else
                                    {
                                        textures[i] = tex->m_texture;
                                    }
                                    binding.m_resources[write.first_array_index + i] = tex->m_texture;
                                }
                                NSRange range;
                                range.location = write.binding_slot + write.first_array_index;
                                range.length = write.num_descs;
                                [m_encoder setTextures:textures withRange:range];
                            }
                            break;
                            case DescriptorType::sampler:
                            if(write.num_descs == 1)
                            {
                                const SamplerDesc& view = write.samplers[0];
                                id<MTLSamplerState> sampler = new_sampler_state(m_device->m_device, view);
                                if(!sampler) return BasicError::bad_platform_call();
                                m_samplers.insert_or_assign(write.binding_slot + write.first_array_index, sampler);
                                [m_encoder setSamplerState:sampler atIndex:write.binding_slot + write.first_array_index];
                            }
                            else
                            {
                                id<MTLSamplerState> __unsafe_unretained* samplers = (id<MTLSamplerState> __unsafe_unretained*)salloc.allocate(sizeof(id<MTLSamplerState>) * write.num_descs);
                                for(usize i = 0; i < write.num_descs; ++i)
                                {
                                    const SamplerDesc& view = write.samplers[i];
                                    id<MTLSamplerState> sampler = new_sampler_state(m_device->m_device, view);
                                    if(!sampler) return BasicError::bad_platform_call();
                                    m_samplers.insert_or_assign(write.binding_slot + write.first_array_index + (u32)i, sampler);
                                    samplers[i] = sampler;
                                }
                                NSRange range;
                                range.location = write.binding_slot + write.first_array_index;
                                range.length = write.num_descs;
                                [m_encoder setSamplerStates:samplers withRange:range];
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
