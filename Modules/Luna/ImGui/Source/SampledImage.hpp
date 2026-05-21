/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#pragma once
#include "../ImGui.hpp"
#include "SampledImage.generated.hpp"

namespace Luna
{
    namespace ImGuiUtils
    {
        struct [[luna::struct("29378bf1-b58e-4c8a-a30f-d29239f9a713")]] SampledImage : ISampledImage
        {
            luiimpl();

            Ref<RHI::ITexture> m_texture;
            RHI::SamplerDesc m_sampler;

            virtual RHI::ITexture* get_texture() override { return m_texture; }
            virtual void set_texture(RHI::ITexture* texture) override { m_texture = texture; }
            virtual RHI::SamplerDesc get_sampler() override { return m_sampler; }
            virtual void set_sampler(const RHI::SamplerDesc& desc) override { m_sampler = desc; }
        };
    }
}
