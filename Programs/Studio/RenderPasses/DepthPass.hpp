/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DepthPass.hpp
* @author JXMaster
* @date 2023/3/11
*/
#pragma once
#include <Luna/RG/RenderPass.hpp>
#include "CommonVertex.hpp"
#include "../Scene.hpp"
#include "../ModelRenderer.hpp"

namespace Luna
{
    struct DepthPassGlobalData
    {
        lustruct("DepthPassGlobalData", "{6fca7e1e-e878-4852-9ae2-bdd1c09b2b61}");

        Ref<RHI::IPipelineState> m_depth_pass_pso;
		Ref<RHI::IDescriptorSetLayout> m_depth_pass_dlayout;
		Ref<RHI::IPipelineLayout> m_depth_pass_playout;

        Ref<RHI::ITexture> m_default_base_color;	// 1.0f, 1.0f, 1.0f, 1.0f
    
        RV init(RHI::IDevice* device);
    };

    struct DepthPass : RG::IRenderPass
    {
        lustruct("DepthPass", "{f108cfc1-e026-4462-8db9-df259a7bc5e7}");
        luiimpl();

        Span<Ref<Entity>> ts;
		Span<Ref<ModelRenderer>> rs;
        Ref<RHI::IBuffer> camera_cb;
        Ref<RHI::IBuffer> model_matrices;

        RV init(DepthPassGlobalData* global_data);
        RV execute(RG::IRenderPassContext* ctx) override;

        private:
        Ref<DepthPassGlobalData> m_global_data;
    };

    RV register_depth_pass();
}