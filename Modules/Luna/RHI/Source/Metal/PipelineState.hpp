/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file PipelineState.hpp
* @author JXMaster
* @date 2022/7/25
*/
#pragma once
#include "Device.hpp"
namespace Luna
{
    namespace RHI
    {
        struct RenderPipelineState : IPipelineState
        {
            lustruct("RHI::RenderPipelineState", "{78f9f67e-c86f-4c84-bba5-9bf05dac905b}");
            luiimpl();

            Ref<Device> m_device;
            NSPtr<MTL::RenderPipelineState> m_pso;

            RV init(const GraphicsPipelineStateDesc& desc);

            virtual IDevice* get_device() override { return m_device; }
            virtual void set_name(const Name& name) override  { set_object_name(m_pso.get(), name); }
        };

        struct ComputePipelineState : IPipelineState
        {
            lustruct("RHI::ComputePipelineState", "{77517afd-158a-43e6-b762-3e132fc287a8}");
            luiimpl();

            Ref<Device> m_device;
            NSPtr<MTL::ComputePipelineState> m_pso;

            RV init(const ComputePipelineStateDesc& desc);

            virtual IDevice* get_device() override { return m_device; }
            virtual void set_name(const Name& name) override  { set_object_name(m_pso.get(), name); }
        };
    }
}