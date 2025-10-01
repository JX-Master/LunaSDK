/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file GeometryPass.hpp
* @author JXMaster
* @date 2023/3/11
*/
#pragma once
#include <Luna/RG/RenderPass.hpp>
#include "CommonVertex.hpp"
#include "../Scene.hpp"
#include "../ModelRenderer.hpp"
#include "../SceneRenderer.hpp"

namespace Luna
{
    struct GeometryPassGlobalData
    {
        lustruct("GeometryPassGlobalData", "{8e00d9f0-e920-45e2-a9fc-c7e51644d286}");

        Ref<RHI::IPipelineState> m_geometry_pass_pso;
        Ref<RHI::IDescriptorSetLayout> m_geometry_pass_dlayout;
        Ref<RHI::IPipelineLayout> m_geometry_pass_playout;

        Ref<RHI::ITexture> m_default_base_color;    // 1.0f, 1.0f, 1.0f, 1.0f
        Ref<RHI::ITexture> m_default_roughness;    // 0.5f
        Ref<RHI::ITexture> m_default_normal;        // 0.5f, 0.5f, 1.0f, 1.0f
        Ref<RHI::ITexture> m_default_metallic;    // 0.0f
        Ref<RHI::ITexture> m_default_emissive;    // 0.0f, 0.0f, 0.0f, 0.0f

        u32 m_model_matrices_stride;
        u32 m_material_parameter_stride;
    
        RV init(RHI::IDevice* device);
    };

    struct GeometryPass : RG::IRenderPass
    {
        lustruct("GeometryPass", "{addf4399-72e6-4855-83a9-457153a2c5a1}");
        luiimpl();

        Span<MeshRenderParams> mesh_render_params;
        Ref<RHI::IBuffer> camera_cb;
        Ref<RHI::IBuffer> model_matrices;
        Ref<RHI::IBuffer> material_parameters;

        RV init(GeometryPassGlobalData* global_data);
        RV execute(RG::IRenderPassContext* ctx) override;

        private:
        Ref<GeometryPassGlobalData> m_global_data;

    };

    RV register_geometry_pass();
}