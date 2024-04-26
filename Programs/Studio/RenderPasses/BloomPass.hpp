/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file BloomPass.hpp
* @author JXMaster
* @date 2024/4/24
*/
#pragma once
#include <Luna/RG/RenderPass.hpp>

namespace Luna
{
    struct BloomPassGlobalData
    {
        lustruct("BloomPassGlobalData", "{539c5e18-85b8-4c5e-8554-450b17cabd1f}");
        
        Ref<RHI::IDescriptorSetLayout> m_bloom_setup_pass_dlayout;
        Ref<RHI::IPipelineLayout> m_bloom_setup_pass_playout;
        Ref<RHI::IPipelineState> m_bloom_setup_pass_pso;
        
        Ref<RHI::IDescriptorSetLayout> m_bloom_downsample_pass_dlayout;
        Ref<RHI::IPipelineLayout> m_bloom_downsample_pass_playout;
        Ref<RHI::IPipelineState> m_bloom_downsample_pass_pso;

        Ref<RHI::IDescriptorSetLayout> m_bloom_upsample_pass_dlayout;
        Ref<RHI::IPipelineLayout> m_bloom_upsample_pass_playout;
        Ref<RHI::IPipelineState> m_bloom_upsample_pass_pso;

        RV init(RHI::IDevice* device);
    };

    struct BloomPass : RG::IRenderPass
    {
        lustruct("BloomPass", "{5c2480a4-23b0-49d8-95ae-c5ae73248c11}");
        luiimpl();

        f32 lum_threshold = 1.0f;
        f32 up_sample_radius = 1.0f;

        RV init(BloomPassGlobalData* global_data);
        RV execute(RG::IRenderPassContext* ctx) override;

        private:
        Ref<BloomPassGlobalData> m_global_data;

        struct SamplePassData
        {
            Ref<RHI::IDescriptorSet> m_ds;
            Ref<RHI::IBuffer> m_params;
        };

        SamplePassData m_setup_pass;
        Vector<SamplePassData> m_downsample_passes;
        Vector<SamplePassData> m_upsample_passes;
    };

    RV register_bloom_pass();
}