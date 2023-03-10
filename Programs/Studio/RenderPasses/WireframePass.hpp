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
#include <RG/RenderPass.hpp>
#include "CommonVertex.hpp"
#include "../Scene.hpp"
#include "../ModelRenderer.hpp"

namespace Luna
{
    struct WireframePassGlobalData
    {
        lustruct("WireframePassGlobalData", "{df0720b0-7ace-4cb4-94af-90260b82f8e7}");

        Ref<CommonVertex> m_common_vertex;

        Ref<RHI::IPipelineState> m_debug_mesh_renderer_pso;
		Ref<RHI::IDescriptorSetLayout> m_debug_mesh_renderer_dlayout;
		Ref<RHI::IShaderInputLayout> m_debug_mesh_renderer_slayout;

        RV init(RHI::IDevice* device);
    };

    struct WireframePass : RG::IRenderPass
    {
        lustruct("WireframePass", "{849e92d5-6407-4018-9ee7-4ffa34ab3044}");
        luiimpl();

        Span<Ref<Entity>> ts;
		Span<Ref<ModelRenderer>> rs;

        Ref<RHI::IResource> camera_cb;
        Ref<RHI::IResource> model_matrices;

        RV init(WireframePassGlobalData* global_data);
        RV execute(RG::IRenderPassContext* ctx) override;

        private:
        Ref<WireframePassGlobalData> m_global_data;
    };

    RV register_wireframe_pass();
}