/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file PipelineState.mm
* @author JXMaster
* @date 2023/7/25
*/
#include "PipelineState.h"
#include <Luna/VariantUtils/JSON.hpp>

namespace Luna
{
    namespace RHI
    {
        RV RenderPipelineState::init(const GraphicsPipelineStateDesc& desc)
        {
            @autoreleasepool
            {
                lutry
                {
                    NSError* err = nil;
                    id<MTLLibrary> vs;
                    id<MTLLibrary> ps;
                    id<MTLFunction> vs_func;
                    id<MTLFunction> ps_func;
                    if(desc.vs.format != ShaderDataFormat::none)
                    {
                        if(desc.vs.format == ShaderDataFormat::msl)
                        {
                            MTLCompileOptions* options = [[MTLCompileOptions alloc]init];
                            String source_string((const c8*)desc.vs.data.data(), desc.vs.data.size());
                            NSString* source = [NSString stringWithUTF8String:source_string.c_str()];
                            vs = [m_device->m_device newLibraryWithSource:source options:options error:&err];
                        }
                        else if(desc.vs.format == ShaderDataFormat::metallib)
                        {
                            dispatch_data_t data = dispatch_data_create(desc.vs.data.data(), desc.vs.data.size(), nullptr, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
                            vs = [m_device->m_device newLibraryWithData:data error:&err];
                        }
                        else
                        {
                            return set_error(BasicError::bad_arguments(), "The vertex shader format must be ShaderDataFormat::msl for Metal backend.");
                        }
                        if(!vs)
                        {
                            NSString* err_desc = [err description];
                            return set_error(BasicError::bad_platform_call(), "%s", [err_desc cStringUsingEncoding:NSUTF8StringEncoding]);
                        }
                        MTLFunctionConstantValues* values = [[MTLFunctionConstantValues alloc]init];
                        NSString* name = [NSString stringWithUTF8String:desc.vs.entry_point.c_str()];
                        vs_func = [vs newFunctionWithName:name constantValues:values error:&err];
                        if(!vs_func)
                        {
                            NSString* err_desc = [err description];
                            return set_error(BasicError::bad_platform_call(), "%s", [err_desc cStringUsingEncoding:NSUTF8StringEncoding]);
                        }
                    }
                    if(desc.ps.format != ShaderDataFormat::none)
                    {
                        if(desc.ps.format == ShaderDataFormat::msl)
                        {
                            MTLCompileOptions* options = [[MTLCompileOptions alloc]init];
                            String source_string((const c8*)desc.ps.data.data(), desc.ps.data.size());
                            NSString* source = [NSString stringWithUTF8String:source_string.c_str()];
                            ps = [m_device->m_device newLibraryWithSource:source options:options error:&err];
                        }
                        else if(desc.ps.format == ShaderDataFormat::metallib)
                        {
                            dispatch_data_t data = dispatch_data_create(desc.ps.data.data(), desc.ps.data.size(), nullptr, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
                            ps = [m_device->m_device newLibraryWithData:data error:&err];
                        }
                        else
                        {
                            return set_error(BasicError::bad_arguments(), "The pixel shader format must be ShaderDataFormat::msl for Metal backend.");
                        }
                        if(!ps)
                        {
                            NSString* err_desc = [err description];
                            return set_error(BasicError::bad_platform_call(), "%s", [err_desc cStringUsingEncoding:NSUTF8StringEncoding]);
                        }
                        MTLFunctionConstantValues* values = [[MTLFunctionConstantValues alloc]init];
                        NSString* name = [NSString stringWithUTF8String:desc.ps.entry_point.c_str()];
                        ps_func = [ps newFunctionWithName:name constantValues:values error:&err];
                        if(!ps_func)
                        {
                            NSString* err_desc = [err description];
                            return set_error(BasicError::bad_platform_call(), "%s", [err_desc cStringUsingEncoding:NSUTF8StringEncoding]);
                        }
                    }
                    MTLRenderPipelineDescriptor* d = [[MTLRenderPipelineDescriptor alloc]init];
                    // Set shader.
                    if(vs_func)
                    {
                        d.vertexFunction = vs_func;
                        d.maxVertexCallStackDepth = 256;
                    }
                    if(ps_func)
                    {
                        d.fragmentFunction = ps_func;
                        d.maxFragmentCallStackDepth = 256;
                    }
                    // Set vertex descriptor.
                    MTLVertexDescriptor* vertex_desc = [[MTLVertexDescriptor alloc]init];
                    {
                        MTLVertexAttributeDescriptorArray* attributes = vertex_desc.attributes;
                        MTLVertexBufferLayoutDescriptorArray* layouts = vertex_desc.layouts;
                        for(u32 i = 0; i < (u32)desc.input_layout.attributes.size(); ++i)
                        {
                            const InputAttributeDesc& src = desc.input_layout.attributes[i];
                            MTLVertexAttributeDescriptor* dst = [[MTLVertexAttributeDescriptor alloc]init];
                            dst.format = encode_vertex_format(src.format);
                            dst.offset = src.offset;
                            dst.bufferIndex = src.binding_slot + VERTEX_BUFFER_SLOT_OFFSET;
                            attributes[src.location] = dst;
                        }
                        for(u32 i = 0; i < (u32)desc.input_layout.bindings.size(); ++i)
                        {
                            const InputBindingDesc& src = desc.input_layout.bindings[i];
                            MTLVertexBufferLayoutDescriptor* dst = [[MTLVertexBufferLayoutDescriptor alloc]init];
                            dst.stride = src.element_size;
                            switch(src.input_rate)
                            {
                                case InputRate::per_vertex:
                                    dst.stepFunction = MTLVertexStepFunctionPerVertex; break;
                                case InputRate::per_instance:
                                    dst.stepFunction = MTLVertexStepFunctionPerInstance; break;
                            }
                            dst.stepRate = 1;
                            layouts[src.binding_slot + VERTEX_BUFFER_SLOT_OFFSET] = dst;
                        }
                    }
                    d.vertexDescriptor = vertex_desc;
                    // Set attachments and blend factors.
                    {
                        MTLRenderPipelineColorAttachmentDescriptorArray* color_attachments = d.colorAttachments;
                        for(u32 i = 0; i < desc.num_color_attachments; ++i)
                        {
                            MTLRenderPipelineColorAttachmentDescriptor* attachment = [[MTLRenderPipelineColorAttachmentDescriptor alloc]init];
                            attachment.pixelFormat = encode_pixel_format(desc.color_formats[i]);
                            attachment.writeMask = MTLColorWriteMaskAll;
                            auto blend = &desc.blend_state.attachments[i];
                            if(!desc.blend_state.independent_blend_enable)
                            {
                                blend = &desc.blend_state.attachments[0];
                            }
                            attachment.blendingEnabled = blend->blend_enable;
                            attachment.rgbBlendOperation = encode_blend_op(blend->blend_op_color);
                            attachment.alphaBlendOperation = encode_blend_op(blend->blend_op_alpha);
                            attachment.sourceRGBBlendFactor = encode_blend_factor(blend->src_blend_color, true);
                            attachment.sourceAlphaBlendFactor = encode_blend_factor(blend->src_blend_alpha, false);
                            attachment.destinationRGBBlendFactor = encode_blend_factor(blend->dst_blend_color, true);
                            attachment.destinationAlphaBlendFactor = encode_blend_factor(blend->dst_blend_alpha, false);
                            MTLColorWriteMask mask = 0;
                            if(test_flags(blend->color_write_mask, ColorWriteMask::red)) mask |= MTLColorWriteMaskRed;
                            if(test_flags(blend->color_write_mask, ColorWriteMask::green)) mask |= MTLColorWriteMaskGreen;
                            if(test_flags(blend->color_write_mask, ColorWriteMask::blue)) mask |= MTLColorWriteMaskBlue;
                            if(test_flags(blend->color_write_mask, ColorWriteMask::alpha)) mask |= MTLColorWriteMaskAlpha;
                            attachment.writeMask = mask;
                            color_attachments[i] = attachment;
                        }
                    }
                    d.depthAttachmentPixelFormat = encode_pixel_format(desc.depth_stencil_format);
                    d.stencilAttachmentPixelFormat = is_stencil_format(desc.depth_stencil_format) ? encode_pixel_format(desc.depth_stencil_format) : MTLPixelFormatInvalid;
                    // Rasterization and Visibility State
                    d.alphaToCoverageEnabled = desc.blend_state.alpha_to_coverage_enable;
                    d.alphaToOneEnabled = NO;
                    d.rasterizationEnabled = YES;
                    d.inputPrimitiveTopology = encode_primitive_topology(desc.primitive_topology);
                    d.rasterSampleCount = desc.sample_count;
                    m_pso = [m_device->m_device newRenderPipelineStateWithDescriptor:d error:&err];
                    if(!m_pso)
                    {
                        NSString* err_desc = [err description];
                        return set_error(BasicError::bad_platform_call(), "%s", [err_desc cStringUsingEncoding:NSUTF8StringEncoding]);
                    }
                    switch(desc.rasterizer_state.fill_mode)
                    {
                        case FillMode::solid: m_fill_mode = MTLTriangleFillModeFill; break;
                        case FillMode::wireframe: m_fill_mode = MTLTriangleFillModeLines; break;
                    }
                    switch(desc.rasterizer_state.cull_mode)
                    {
                        case CullMode::none: m_cull_mode = MTLCullModeNone; break;
                        case CullMode::front: m_cull_mode = MTLCullModeFront; break;
                        case CullMode::back: m_cull_mode = MTLCullModeBack; break;
                    }
                    if(desc.rasterizer_state.depth_clamp_enable)
                    {
                        m_depth_clip_mode = MTLDepthClipModeClamp;
                    }
                    else
                    {
                        m_depth_clip_mode = MTLDepthClipModeClip;
                    }
                    m_primitive_type = encode_primitive_type(desc.primitive_topology);
                    m_front_counter_clockwise = desc.rasterizer_state.front_counter_clockwise;
                    // Depth stencil state.
                    {
                        MTLDepthStencilDescriptor* ds_desc = [[MTLDepthStencilDescriptor alloc]init];
                        ds_desc.depthWriteEnabled = desc.depth_stencil_state.depth_write_enable;
                        ds_desc.depthCompareFunction = desc.depth_stencil_state.depth_test_enable ? encode_compare_function(desc.depth_stencil_state.depth_func) : MTLCompareFunctionAlways;
                        MTLStencilDescriptor* front_face = [[MTLStencilDescriptor alloc]init];
                        MTLStencilDescriptor* back_face = [[MTLStencilDescriptor alloc]init];
                        front_face.stencilFailureOperation = encode_stencil_operation(desc.depth_stencil_state.front_face.stencil_fail_op);
                        front_face.depthFailureOperation = encode_stencil_operation(desc.depth_stencil_state.front_face.stencil_depth_fail_op);
                        front_face.depthStencilPassOperation = encode_stencil_operation(desc.depth_stencil_state.front_face.stencil_pass_op);
                        front_face.stencilCompareFunction = desc.depth_stencil_state.stencil_enable ? encode_compare_function(desc.depth_stencil_state.front_face.stencil_func) : MTLCompareFunctionAlways;
                        front_face.readMask = desc.depth_stencil_state.stencil_read_mask;
                        front_face.writeMask = desc.depth_stencil_state.stencil_write_mask;
                        back_face.stencilFailureOperation = encode_stencil_operation(desc.depth_stencil_state.back_face.stencil_fail_op);
                        back_face.depthFailureOperation = encode_stencil_operation(desc.depth_stencil_state.back_face.stencil_depth_fail_op);
                        back_face.depthStencilPassOperation = encode_stencil_operation(desc.depth_stencil_state.back_face.stencil_pass_op);
                        back_face.stencilCompareFunction = desc.depth_stencil_state.stencil_enable ? encode_compare_function(desc.depth_stencil_state.back_face.stencil_func) : MTLCompareFunctionAlways;
                        back_face.readMask = desc.depth_stencil_state.stencil_read_mask;
                        back_face.writeMask = desc.depth_stencil_state.stencil_write_mask;
                        ds_desc.frontFaceStencil = front_face;
                        ds_desc.backFaceStencil = back_face;
                        m_dss = [m_device->m_device newDepthStencilStateWithDescriptor:ds_desc];
                        if(!m_dss)
                        {
                            return BasicError::bad_platform_call();
                        }
                        m_depth_bias = desc.rasterizer_state.depth_bias;
                        m_slope_scaled_depth_bias = desc.rasterizer_state.slope_scaled_depth_bias;
                        m_depth_bias_clamp = desc.rasterizer_state.depth_bias_clamp;
                    }
                }
                lucatchret;
                return ok;
            }
        }

        RV ComputePipelineState::init(const ComputePipelineStateDesc& desc)
        {
            @autoreleasepool
            {
                lutry
                {
                    NSError* err = nullptr;
                    id<MTLLibrary> cs;
                    id<MTLFunction> cs_func;
                    if(desc.cs.format == ShaderDataFormat::msl)
                    {
                        MTLCompileOptions* options = [[MTLCompileOptions alloc]init];
                        String source_string((const c8*)desc.cs.data.data(), desc.cs.data.size());
                        NSString* source = [NSString  stringWithUTF8String:source_string.c_str()];
                        cs = [m_device->m_device newLibraryWithSource:source options:options error:&err];
                    }
                    else if(desc.cs.format == ShaderDataFormat::metallib)
                    {
                        dispatch_data_t data = dispatch_data_create(desc.cs.data.data(), desc.cs.data.size(), nullptr, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
                        cs = [m_device->m_device newLibraryWithData:data error:&err];
                    }
                    else
                    {
                        return set_error(BasicError::bad_arguments(), "The compute shader format must be ShaderDataFormat::msl for Metal backend.");
                    }
                    if(!cs)
                    {
                        NSString* err_desc = [err description];
                        return set_error(BasicError::bad_platform_call(), "%s", [err_desc cStringUsingEncoding:NSUTF8StringEncoding]);
                    }
                    MTLFunctionConstantValues* values = [[MTLFunctionConstantValues alloc]init];
                    NSString* name = [NSString stringWithUTF8String:desc.cs.entry_point.c_str()];
                    cs_func = [cs newFunctionWithName:name constantValues:values error:&err];
                    if(!cs_func)
                    {
                        NSString* err_desc = [err description];
                        return set_error(BasicError::bad_platform_call(), "%s", [err_desc cStringUsingEncoding:NSUTF8StringEncoding]);
                    }
                    MTLComputePipelineDescriptor* d = [[MTLComputePipelineDescriptor alloc]init];
                    d.computeFunction = cs_func;
                    d.maxCallStackDepth = 256;
                    m_pso = [m_device->m_device newComputePipelineStateWithDescriptor:d options:MTLPipelineOptionNone reflection:nil error:&err];
                    if(!m_pso)
                    {
                        NSString* err_desc = [err description];
                        return set_error(BasicError::bad_platform_call(), "%s", [err_desc cStringUsingEncoding:NSUTF8StringEncoding]);
                    }
                    m_num_threads_per_group = UInt3U(desc.metal_numthreads_x, desc.metal_numthreads_y, desc.metal_numthreads_z);
                }
                lucatchret;
                return ok;
            }
        }
    }
}
