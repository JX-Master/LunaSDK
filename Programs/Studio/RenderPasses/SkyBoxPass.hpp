/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SkyBoxPass.hpp
* @author JXMaster
* @date 2023/3/7
*/
#pragma once
#include <Luna/RG/RenderPass.hpp>
#include "../Camera.hpp"
namespace Luna
{
    struct SkyBoxPassGlobalData
    {
        lustruct("SkyBoxPassGlobalData", "{cc8658bc-ecbc-4659-9b10-e95b377c7581}");

        Ref<RHI::IPipelineState> m_skybox_pass_pso;
        Ref<RHI::IDescriptorSetLayout> m_skybox_pass_dlayout;
        Ref<RHI::IPipelineLayout> m_skybox_pass_playout;
        Name m_texture_name = "texture";
        Name m_depth_texture_name = "depth_texture";

        RV init(RHI::IDevice* device);
    };

    struct SkyBoxPass : RG::IRenderPass
    {
        lustruct("SkyBoxPass", "{2f57e8a4-1d4b-41b2-8c3c-9a619276e4a7}");
        luiimpl();

        Ref<RHI::ITexture> skybox;
        CameraType camera_type;
        Float4x4 view_to_world;
        f32 camera_fov;

        RV init(SkyBoxPassGlobalData* global_data);
        RV execute(RG::IRenderPassContext* ctx) override;

        private:
        Ref<SkyBoxPassGlobalData> m_global_data;
        Ref<RHI::IBuffer> m_skybox_params_cb;
        Ref<RHI::IDescriptorSet> m_ds;
    };

    RV register_sky_box_pass();
}