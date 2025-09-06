/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MipmapGenerationContext.hpp
* @author JXMaster
* @date 2025/9/6
*/
#pragma once
#include "../MipmapGenerationContext.hpp"

namespace Luna
{
    namespace RHIUtility
    {
        // constant state that can be shared in the same device.
        struct MipmapGenerationPipelineState
        {
            lustruct("RHIUtility::MipmapGenerationPipelineState", "294d027f-a80f-4be9-9fbf-65815e605f2e");

            Ref<RHI::IDescriptorSetLayout> m_dlayout_1d;
            Ref<RHI::IPipelineLayout> m_playout_1d;
            Ref<RHI::IPipelineState> m_pso_1d;
            Ref<RHI::IDescriptorSetLayout> m_dlayout_2d;
            Ref<RHI::IPipelineLayout> m_playout_2d;
            Ref<RHI::IPipelineState> m_pso_2d;
            Ref<RHI::IDescriptorSetLayout> m_dlayout_3d;
            Ref<RHI::IPipelineLayout> m_playout_3d;
            Ref<RHI::IPipelineState> m_pso_3d;

            RV init(RHI::IDevice* device);

            static R<Ref<MipmapGenerationPipelineState>> get_by_device(RHI::IDevice* device);
        };

        void cleanup_mipmap_generation_states();

        struct MipmapGenerationOp
        {
            Ref<RHI::ITexture> tex;
            u32 source_mip;
            u32 end_mip;
        };
        
        struct MipmapGenerationContext : IMipmapGenerationContext
        {
            lustruct("RHIUtility::MipmapGenerationContext", "6b36094d-97f3-47ab-abab-c3b3fe71ed36");
            luiimpl();

            Ref<RHI::IDevice> m_device;
            Name m_name;
            Ref<MipmapGenerationPipelineState> m_ps;
            Vector<MipmapGenerationOp> m_ops;
            Vector<Ref<RHI::IDescriptorSet>> m_dss_1d;
            Vector<Ref<RHI::IDescriptorSet>> m_dss_2d;
            Vector<Ref<RHI::IDescriptorSet>> m_dss_3d;
            Ref<RHI::IBuffer> m_cb;

            RV init(RHI::IDevice* device);
            virtual RHI::IDevice* get_device() override
            {
                return m_device.get();
            }
            virtual void set_name(const c8* name) override
            {
                m_name = name;
            }
            virtual void reset() override;
            virtual void generate_mipmaps(RHI::ITexture* tex, u32 source_mip, u32 num_gen_mips) override;
            virtual RV commit(RHI::ICommandBuffer* compute_cmdbuf, bool submit_and_wait) override;
        };
    }
}