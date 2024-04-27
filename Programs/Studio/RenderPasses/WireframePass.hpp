/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file WireframePass.hpp
* @author JXMaster
* @date 2023/3/7
*/
#pragma once
#include <Luna/RG/RenderPass.hpp>
#include "CommonVertex.hpp"
#include "../Scene.hpp"
#include "../ModelRenderer.hpp"
#include "../SceneRenderer.hpp"

namespace Luna
{
    struct WireframePassGlobalData
    {
        lustruct("WireframePassGlobalData", "{df0720b0-7ace-4cb4-94af-90260b82f8e7}");

        Ref<RHI::IPipelineState> m_debug_mesh_renderer_pso;
        Ref<RHI::IDescriptorSetLayout> m_debug_mesh_renderer_dlayout;
        Ref<RHI::IPipelineLayout> m_debug_mesh_renderer_playout;

        u32 m_model_matrices_stride;

        RV init(RHI::IDevice* device);
    };

    struct WireframePass : RG::IRenderPass
    {
        lustruct("WireframePass", "{849e92d5-6407-4018-9ee7-4ffa34ab3044}");
        luiimpl();

        Span<MeshRenderParams> mesh_render_params;

        Ref<RHI::IBuffer> camera_cb;
        Ref<RHI::IBuffer> model_matrices;

        RV init(WireframePassGlobalData* global_data);
        RV execute(RG::IRenderPassContext* ctx) override;

        private:
        Ref<WireframePassGlobalData> m_global_data;
    };

    RV register_wireframe_pass();
}