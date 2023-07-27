/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file PipelineState.cpp
* @author JXMaster
* @date 2022/7/25
*/
#include "PipelineState.hpp"

namespace Luna
{
    namespace RHI
    {
        RV RenderPipelineState::init(const GraphicsPipelineStateDesc& desc)
        {
            AutoreleasePool pool;
            NS::Error* err = nullptr;
            NSPtr<MTL::Library> vs;
            NSPtr<MTL::Library> ps;
            NSPtr<MTL::Function> vs_func;
            NSPtr<MTL::Function> ps_func;
            if(!desc.vs.empty())
            {
                NSPtr<MTL::CompileOptions> options = box(MTL::CompileOptions::alloc()->init());
                String str((const c8*)desc.vs.data(), desc.vs.size());
                NS::String* source = NS::String::string(str.c_str(), NS::UTF8StringEncoding);
                vs = box(m_device->m_device->newLibrary(source, options.get(), &err));
                if(!vs)
                {
                    NS::String* err_desc = err->description();
                    return set_error(BasicError::bad_platform_call(), "%s", err_desc->cString(NS::UTF8StringEncoding));
                }
                NSPtr<MTL::FunctionConstantValues> values = box(MTL::FunctionConstantValues::alloc()->init());
                NS::String* name = NS::String::string("main", NS::StringEncoding::UTF8StringEncoding);
                vs_func = box(vs->newFunction(name, values.get(), &err));
                if(!vs_func)
                {
                    NS::String* err_desc = err->description();
                    return set_error(BasicError::bad_platform_call(), "%s", err_desc->cString(NS::UTF8StringEncoding));
                }
            }
            if(!desc.ps.empty())
            {
                NSPtr<MTL::CompileOptions> options = box(MTL::CompileOptions::alloc()->init());
                String str((const c8*)desc.ps.data(), desc.ps.size());
                NS::String* source = NS::String::string(str.c_str(), NS::UTF8StringEncoding);
                ps = box(m_device->m_device->newLibrary(source, options.get(), &err));
                if(!ps)
                {
                    NS::String* err_desc = err->description();
                    return set_error(BasicError::bad_platform_call(), "%s", err_desc->cString(NS::UTF8StringEncoding));
                }
                NSPtr<MTL::FunctionConstantValues> values = box(MTL::FunctionConstantValues::alloc()->init());
                NS::String* name = NS::String::string("main", NS::StringEncoding::UTF8StringEncoding);
                ps_func = box(ps->newFunction(name, values.get(), &err));
                if(!ps_func)
                {
                    NS::String* err_desc = err->description();
                    return set_error(BasicError::bad_platform_call(), "%s", err_desc->cString(NS::UTF8StringEncoding));
                }
            }
            NSPtr<MTL::RenderPipelineDescriptor> d = box(MTL::RenderPipelineDescriptor::alloc()->init());
            // Set shader.
            if(vs_func)
            {
                d->setVertexFunction(vs_func.get());
                d->setMaxVertexCallStackDepth(256);
            }
            if(ps_func)
            {
                d->setFragmentFunction(ps_func.get());
                d->setMaxFragmentCallStackDepth(256);
            }
            // Set vertex descriptor.
            NSPtr<MTL::VertexDescriptor> vertex_desc = box(MTL::VertexDescriptor::alloc()->init());
            {
                MTL::VertexAttributeDescriptorArray* attributes = vertex_desc->attributes();
                MTL::VertexBufferLayoutDescriptorArray* layouts = vertex_desc->layouts();
                for(u32 i = 0; i < (u32)desc.input_layout.attributes.size(); ++i)
                {
                    const InputAttributeDesc& src = desc.input_layout.attributes[i];
                    NSPtr<MTL::VertexAttributeDescriptor> dst = box(MTL::VertexAttributeDescriptor::alloc()->init());
                    dst->setFormat(encode_vertex_format(src.format));
                    dst->setOffset(src.offset);
                    dst->setBufferIndex(src.binding_slot + VERTEX_BUFFER_SLOT_OFFSET);
                    attributes->setObject(dst.get(), src.location);
                }
                for(u32 i = 0; i < (u32)desc.input_layout.bindings.size(); ++i)
                {
                    const InputBindingDesc& src = desc.input_layout.bindings[i];
                    NSPtr<MTL::VertexBufferLayoutDescriptor> dst = box(MTL::VertexBufferLayoutDescriptor::alloc()->init());
                    dst->setStride(src.element_size);
                    switch(src.input_rate)
                    {
                        case InputRate::per_vertex:
                        dst->setStepFunction(MTL::VertexStepFunctionPerVertex); break;
                        case InputRate::per_instance:
                        dst->setStepFunction(MTL::VertexStepFunctionPerInstance); break;
                    }
                    dst->setStepRate(1);
                    layouts->setObject(dst.get(), src.binding_slot + VERTEX_BUFFER_SLOT_OFFSET);
                }
            }
            d->setVertexDescriptor(vertex_desc.get());
            // Set attachments and blend factors.
            {
                MTL::RenderPipelineColorAttachmentDescriptorArray* color_attachments = d->colorAttachments();
                for(u32 i = 0; i < desc.num_color_attachments; ++i)
                {
                    NSPtr<MTL::RenderPipelineColorAttachmentDescriptor> attachment = box(MTL::RenderPipelineColorAttachmentDescriptor::alloc()->init());
                    attachment->setPixelFormat(encode_pixel_format(desc.color_formats[i]));
                    attachment->setWriteMask(MTL::ColorWriteMaskAll);
                    auto& blend = desc.blend_state.attachments[i];
                    attachment->setBlendingEnabled(blend.blend_enable);
                    attachment->setRgbBlendOperation(encode_blend_op(blend.blend_op_color));
                    attachment->setAlphaBlendOperation(encode_blend_op(blend.blend_op_alpha));
                    attachment->setSourceRGBBlendFactor(encode_blend_factor(blend.src_blend_color, true));
                    attachment->setSourceAlphaBlendFactor(encode_blend_factor(blend.src_blend_alpha, false));
                    attachment->setDestinationRGBBlendFactor(encode_blend_factor(blend.dst_blend_color, true));
                    attachment->setDestinationAlphaBlendFactor(encode_blend_factor(blend.dst_blend_alpha, false));
                    color_attachments->setObject(attachment.get(), i);
                }
            }
            d->setDepthAttachmentPixelFormat(encode_pixel_format(desc.depth_stencil_format));
            d->setStencilAttachmentPixelFormat(encode_pixel_format(desc.depth_stencil_format));
            // Rasterization and Visibility State
            d->setAlphaToCoverageEnabled(desc.blend_state.alpha_to_coverage_enable);
            d->setAlphaToOneEnabled(false);
            d->setRasterizationEnabled(true);
            d->setInputPrimitiveTopology(encode_primitive_topology(desc.primitive_topology));
            d->setRasterSampleCount(desc.sample_count);
            m_pso = box(m_device->m_device->newRenderPipelineState(d.get(), &err));
            if(!m_pso)
            {
                NS::String* err_desc = err->description();
                return set_error(BasicError::bad_platform_call(), "%s", err_desc->cString(NS::UTF8StringEncoding));
            }
            switch(desc.rasterizer_state.cull_mode)
            {
                case CullMode::none: m_cull_mode = MTL::CullModeNone; break;
                case CullMode::front: m_cull_mode = MTL::CullModeFront; break;
                case CullMode::back: m_cull_mode = MTL::CullModeBack; break;
            }
            m_primitive_type = encode_primitive_type(desc.primitive_topology);
            // Depth stencil state.
            {
                NSPtr<MTL::DepthStencilDescriptor> ds_desc = box(MTL::DepthStencilDescriptor::alloc()->init());
                ds_desc->setDepthWriteEnabled(desc.depth_stencil_state.depth_write_enable);
                ds_desc->setDepthCompareFunction(encode_compare_function(desc.depth_stencil_state.depth_func));
                NSPtr<MTL::StencilDescriptor> front_face = box(MTL::StencilDescriptor::alloc()->init());
                NSPtr<MTL::StencilDescriptor> back_face = box(MTL::StencilDescriptor::alloc()->init());
                front_face->setStencilFailureOperation(encode_stencil_operation(desc.depth_stencil_state.front_face.stencil_fail_op));
                front_face->setDepthFailureOperation(encode_stencil_operation(desc.depth_stencil_state.front_face.stencil_depth_fail_op));
                front_face->setDepthStencilPassOperation(encode_stencil_operation(desc.depth_stencil_state.front_face.stencil_pass_op));
                front_face->setStencilCompareFunction(encode_compare_function(desc.depth_stencil_state.front_face.stencil_func));
                front_face->setReadMask(desc.depth_stencil_state.stencil_read_mask);
                front_face->setWriteMask(desc.depth_stencil_state.stencil_write_mask);
                back_face->setStencilFailureOperation(encode_stencil_operation(desc.depth_stencil_state.back_face.stencil_fail_op));
                back_face->setDepthFailureOperation(encode_stencil_operation(desc.depth_stencil_state.back_face.stencil_depth_fail_op));
                back_face->setDepthStencilPassOperation(encode_stencil_operation(desc.depth_stencil_state.back_face.stencil_pass_op));
                back_face->setStencilCompareFunction(encode_compare_function(desc.depth_stencil_state.back_face.stencil_func));
                back_face->setReadMask(desc.depth_stencil_state.stencil_read_mask);
                back_face->setWriteMask(desc.depth_stencil_state.stencil_write_mask);
                ds_desc->setFrontFaceStencil(front_face.get());
                ds_desc->setBackFaceStencil(back_face.get());
                m_dss = box(m_device->m_device->newDepthStencilState(ds_desc.get()));
                if(!m_dss)
                {
                    return BasicError::bad_platform_call();
                }
            }
            return ok;
        }

        RV ComputePipelineState::init(const ComputePipelineStateDesc& desc)
        {
            AutoreleasePool pool;
            NS::Error* err = nullptr;
            NSPtr<MTL::Library> cs;
            NSPtr<MTL::Function> cs_func;
            NSPtr<MTL::CompileOptions> options = box(MTL::CompileOptions::alloc()->init());
            String str((const c8*)desc.cs.data(), desc.cs.size());
            NS::String* source = NS::String::string(str.c_str(), NS::UTF8StringEncoding);
            cs = box(m_device->m_device->newLibrary(source, options.get(), &err));
            if(!cs)
            {
                NS::String* err_desc = err->description();
                return set_error(BasicError::bad_platform_call(), "%s", err_desc->cString(NS::UTF8StringEncoding));
            }
            NSPtr<MTL::FunctionConstantValues> values = box(MTL::FunctionConstantValues::alloc()->init());
            NS::String* name = NS::String::string("main", NS::StringEncoding::UTF8StringEncoding);
            cs_func = box(cs->newFunction(name, values.get(), &err));
            if(!cs_func)
            {
                NS::String* err_desc = err->description();
                return set_error(BasicError::bad_platform_call(), "%s", err_desc->cString(NS::UTF8StringEncoding));
            }
            NSPtr<MTL::ComputePipelineDescriptor> d = box(MTL::ComputePipelineDescriptor::alloc()->init());
            d->setComputeFunction(cs_func.get());
            d->setMaxCallStackDepth(256);
            m_pso = box(m_device->m_device->newComputePipelineState(d.get(), MTL::PipelineOptionNone, nullptr, &err));
            if(!m_pso)
            {
                NS::String* err_desc = err->description();
                return set_error(BasicError::bad_platform_call(), "%s", err_desc->cString(NS::UTF8StringEncoding));
            }
            return ok;
        }
    }
}