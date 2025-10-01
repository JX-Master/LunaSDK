/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DeferredLightingPass.hpp
* @author JXMaster
* @date 2023/3/18
*/
#pragma once
#include <Luna/RG/RenderPass.hpp>
#include "../Scene.hpp"

namespace Luna
{
    struct DeferredLightingPassGlobalData
    {
        lustruct("DeferredLightingPassGlobalData", "{b2d383e3-34ee-4f7e-a100-07b07301a4b3}");

        Ref<RHI::IPipelineState> m_deferred_lighting_pass_pso;
        Ref<RHI::IDescriptorSetLayout> m_deferred_lighting_pass_dlayout;
        Ref<RHI::IPipelineLayout> m_deferred_lighting_pass_playout;

        Ref<RHI::ITexture> m_default_skybox;

        Ref<RHI::ITexture> m_integrate_brdf;

        RV init(RHI::IDevice* device);
    };

    struct DeferredLightingPass : RG::IRenderPass
    {
        lustruct("DeferredLightingPass", "{baae11d9-29ed-46ab-b369-cc80b9c5c073}");
        luiimpl();

        Ref<RHI::ITexture> skybox;
        u32 lighting_mode;

        Span<Ref<Entity>> light_ts;
        Ref<RHI::IBuffer> camera_cb;
        Ref<RHI::IBuffer> light_params;

        RV init(DeferredLightingPassGlobalData* global_data);
        RV execute(RG::IRenderPassContext* ctx) override;

        private:
        Ref<RHI::IBuffer> m_lighting_params_cb;
        Ref<DeferredLightingPassGlobalData> m_global_data;
        Ref<RHI::IDescriptorSet> m_ds;
    };

    RV register_deferred_lighting_pass();
}