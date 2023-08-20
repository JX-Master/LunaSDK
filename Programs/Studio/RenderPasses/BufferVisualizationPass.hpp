/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file BufferVisualizationPass.hpp
* @author JXMaster
* @date 2023/4/5
*/
#pragma once
#include <Luna/RG/RenderPass.hpp>
#include "../Scene.hpp"

namespace Luna
{
    struct BufferVisualizationPassGlobalData
    {
        lustruct("BufferVisualizationPassGlobalData", "{1f810587-a8fe-48a6-9c25-a803b9c9801e}");

        Ref<RHI::IPipelineState> m_buffer_visualization_pass_pso;
		Ref<RHI::IDescriptorSetLayout> m_buffer_visualization_pass_dlayout;
		Ref<RHI::IPipelineLayout> m_buffer_visualization_pass_playout;

        RV init(RHI::IDevice* device);
    };

    struct BufferVisualizationPass : RG::IRenderPass
    {
        lustruct("BufferVisualizationPass", "{7bbc14b1-f5fb-4966-aa3f-36c2dc7e058d}");
        luiimpl();

        u32 vis_type;

        RV init(BufferVisualizationPassGlobalData* global_data);
        RV execute(RG::IRenderPassContext* ctx) override;

        private:
        Ref<BufferVisualizationPassGlobalData> m_global_data;
        Ref<RHI::IBuffer> m_vis_params;
        Ref<RHI::IDescriptorSet> m_ds;
    };

    RV register_buffer_visualization_pass();
}