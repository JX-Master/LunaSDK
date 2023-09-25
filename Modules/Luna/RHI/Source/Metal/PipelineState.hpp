/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file PipelineState.hpp
* @author JXMaster
* @date 2023/7/25
*/
#pragma once
#include "Device.hpp"
namespace Luna
{
    namespace RHI
    {
        constexpr usize VERTEX_BUFFER_SLOT_OFFSET = 16;
        struct RenderPipelineState : IPipelineState
        {
            lustruct("RHI::RenderPipelineState", "{78f9f67e-c86f-4c84-bba5-9bf05dac905b}");
            luiimpl();

            Ref<Device> m_device;
            NSPtr<MTL::RenderPipelineState> m_pso;
            NSPtr<MTL::DepthStencilState> m_dss;
            f32 m_depth_bias;
            f32 m_slope_scaled_depth_bias;
            f32 m_depth_bias_clamp;
            MTL::TriangleFillMode m_fill_mode = MTL::TriangleFillModeFill;
            MTL::CullMode m_cull_mode = MTL::CullModeNone;
            MTL::PrimitiveType m_primitive_type = MTL::PrimitiveTypeTriangle;
            MTL::DepthClipMode m_depth_clip_mode = MTL::DepthClipModeClip;
            bool m_front_counter_clockwise = false;

            RV init(const GraphicsPipelineStateDesc& desc);

            virtual IDevice* get_device() override { return m_device; }
            virtual void set_name(const c8* name) override  { }
        };

        struct ComputePipelineState : IPipelineState
        {
            lustruct("RHI::ComputePipelineState", "{77517afd-158a-43e6-b762-3e132fc287a8}");
            luiimpl();

            Ref<Device> m_device;
            NSPtr<MTL::ComputePipelineState> m_pso;
            UInt3U m_num_threads_per_group;

            RV init(const ComputePipelineStateDesc& desc);

            virtual IDevice* get_device() override { return m_device; }
            virtual void set_name(const c8* name) override  { }
        };
    }
}
