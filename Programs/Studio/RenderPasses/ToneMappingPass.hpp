/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ToneMappingPass.hpp
* @author JXMaster
* @date 2023/3/7
*/
#pragma once
#include <RG/RenderPass.hpp>

namespace Luna
{
    struct ToneMappingPassGlobalData
    {
        lustruct("ToneMappingPassGlobalData", "{83957a6a-f27c-44d5-8b74-a83d8050db08}");

        Ref<RHI::IDescriptorSetLayout> m_first_lum_pass_dlayout;
		Ref<RHI::IShaderInputLayout> m_first_lum_pass_slayout;
		Ref<RHI::IPipelineState> m_first_lum_pass_pso;
		Ref<RHI::IDescriptorSetLayout> m_lum_pass_dlayout;
		Ref<RHI::IShaderInputLayout> m_lum_pass_slayout;
		Ref<RHI::IPipelineState> m_lum_pass_pso;
		Ref<RHI::IDescriptorSetLayout> m_tone_mapping_pass_dlayout;
		Ref<RHI::IShaderInputLayout> m_tone_mapping_pass_slayout;
		Ref<RHI::IPipelineState> m_tone_mapping_pass_pso;

        RV init(RHI::IDevice* device);
    };

    struct ToneMappingPass : RG::IRenderPass
    {
        lustruct("ToneMappingPass", "{66b97075-111b-4915-bc03-7a0f4c477d0b}");
        luiimpl();

        f32 exposure;

        RV init(ToneMappingPassGlobalData* global_data);
        RV execute(RG::IRenderPassContext* ctx) override;

        private:
        Ref<ToneMappingPassGlobalData> m_global_data;

        Ref<RHI::IResource> m_tone_mapping_offset;
		Ref<RHI::IResource> m_tone_mapping_params;
        Ref<RHI::IDescriptorSet> m_first_lum_pass_ds;
        Ref<RHI::IDescriptorSet> m_lum_pass_dss[10];
        Ref<RHI::IDescriptorSet> m_tone_mapping_pass_ds;
        Ref<RHI::IResource> m_lighting_accms[11];		// R32_FLOAT from 1024.v
    };

    RV register_tone_mapping_pass();
}