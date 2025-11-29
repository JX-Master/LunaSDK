/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file BlitContext.hpp
* @author JXMaster
* @date 2025/11/27
*/
#pragma once
#include "../BlitContext.hpp"
#include <Luna/RHI/RHI.hpp>

namespace Luna 
{
    namespace RHIUtility
    {
        struct BlitDrawCall
        {
            RHI::ITexture* dst;
            RHI::SubresourceIndex dst_subresource;
            RHI::TextureViewDesc src;
            RHI::SamplerDesc sampler;
            Float2U top_left;
            Float2U top_right;
            Float2U bottom_left;
            Float2U bottom_right;
        };
        struct BlitContext : IBlitContext
        {
            lustruct("RHIUtility::BlitContext", "2bd56f72-ffe6-4475-a4f3-11fd235d2685");
            luiimpl();

            Ref<RHI::IDevice> m_device;
            Ref<RHI::IDescriptorSetLayout> m_dlayout;
            Ref<RHI::IPipelineLayout> m_playout;
            Ref<RHI::IPipelineState> m_pso;
            Ref<RHI::IBuffer> m_ib;

            Ref<RHI::IBuffer> m_vb;
            usize m_vb_capacity = 0;
            Vector<Ref<RHI::IDescriptorSet>> m_desc_sets;

            Vector<BlitDrawCall> m_draw_calls;

            RV init(RHI::IDevice* device, RHI::Format dst_format);

            virtual RHI::IDevice* get_device() override
            {
                return m_device;
            }
            virtual void set_name(const c8* name) override
            {
                // Left blank.
            }
            virtual void reset() override
            {
                m_draw_calls.clear();
            }
            virtual void blit(RHI::ITexture* dst, const RHI::SubresourceIndex& dst_subresource,
                const RHI::TextureViewDesc& src,
                const RHI::SamplerDesc& sampler,
                const Float2U& top_left, const Float2U& top_right,
                const Float2U& bottom_left, const Float2U& bottom_right) override;
            virtual RV commit(RHI::ICommandBuffer* graphics_cmdbuf, bool submit_and_wait) override;
        };
    }
}