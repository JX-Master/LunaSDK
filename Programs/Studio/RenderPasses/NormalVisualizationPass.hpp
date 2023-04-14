/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file NormalVisualizationPass.hpp
* @author JXMaster
* @date 2023/4/14
*/
#pragma once
#include <RG/RenderPass.hpp>
#include "CommonVertex.hpp"
#include "../Scene.hpp"
#include "../ModelRenderer.hpp"

namespace Luna
{
    struct NormalVisualizationPassGlobalData
    {
        lustruct("NormalVisualizationPassGlobalData", "{267ed768-cd50-407c-a585-942d5021af21}");

        Ref<RHI::IPipelineState> m_normal_visualization_pass_pso;
		Ref<RHI::IDescriptorSetLayout> m_normal_visualization_pass_dlayout;
		Ref<RHI::IShaderInputLayout> m_normal_visualization_pass_slayout;

        RV init(RHI::IDevice* device);
    };

    struct NormalVisualizationPass : RG::IRenderPass
    {
        lustruct("NormalVisualizationPass", "{6ad084b8-2107-4856-a862-b060d4d6141a}");
        luiimpl();

        Span<Ref<Entity>> ts;
		Span<Ref<ModelRenderer>> rs;
        Ref<RHI::IResource> camera_cb;
        Ref<RHI::IResource> model_matrices;

        RV init(NormalVisualizationPassGlobalData* global_data);
        RV execute(RG::IRenderPassContext* ctx) override;

        private:
        Ref<NormalVisualizationPassGlobalData> m_global_data;
    };

    RV register_normal_visualization_pass();
}