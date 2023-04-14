/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file LightingPass.hpp
* @author JXMaster
* @date 2023/3/7
*/
#pragma once
#include <RG/RenderPass.hpp>
#include <Runtime/Math/Matrix.hpp>
#include "CommonVertex.hpp"
#include "../Scene.hpp"
#include "../ModelRenderer.hpp"

namespace Luna
{
    struct LightingPassGlobalData
    {
        lustruct("LightingPassGlobalData", "{17fc5691-dc07-476e-ad53-d2837c8ffba7}");

        Ref<RHI::IPipelineState> m_lighting_pass_pso;
		Ref<RHI::IDescriptorSetLayout> m_lighting_pass_dlayout;
		Ref<RHI::IShaderInputLayout> m_lighting_pass_slayout;

		Ref<RHI::IResource> m_default_base_color;	// 1.0f, 1.0f, 1.0f, 1.0f
		Ref<RHI::IResource> m_default_roughness;	// 0.5f
		Ref<RHI::IResource> m_default_normal;		// 0.5f, 0.5f, 1.0f, 1.0f
		Ref<RHI::IResource> m_default_metallic;	// 0.0f
		Ref<RHI::IResource> m_default_emissive;	// 0.0f, 0.0f, 0.0f, 0.0f

        RV init(RHI::IDevice* device);
    };

    struct LightingPass : RG::IRenderPass
    {
        lustruct("LightingPass", "{e20a01d0-4ad0-40ce-8298-aae69514f015}");
        luiimpl();

        Ref<RHI::IResource> skybox;

		Span<Ref<Entity>> ts;
		Span<Ref<ModelRenderer>> rs;
		Span<Ref<Entity>> light_ts;
		Ref<RHI::IResource> camera_cb;
        Ref<RHI::IResource> model_matrices;
		Ref<RHI::IResource> light_params;

        RV init(LightingPassGlobalData* global_data);
        RV execute(RG::IRenderPassContext* ctx) override;

        private:
        Ref<LightingPassGlobalData> m_global_data;
    };

    RV register_lighting_pass();
}