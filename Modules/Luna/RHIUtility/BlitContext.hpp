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
#ifndef LUNA_RHI_UTILITY_API
#define LUNA_RHI_UTILITY_API
#endif
#include <Luna/Runtime/Ref.hpp>
#include <Luna/RHI/Texture.hpp>
#include <Luna/RHI/CommandBuffer.hpp>
#include <Luna/Runtime/Math/Vector.hpp>

namespace Luna
{
    namespace RHIUtility
    {
        //! @addtogroup RHIUtility
        //! @{

        struct IBlitContext : virtual RHI::IDeviceChild
        {
            luiid("ca143948-12d5-4205-8d1f-a93396e9c6c6");

            virtual void reset() = 0;

            //! Blits the source texture to the destination texture.
            //! @param[in] dst The destination texture.
            //! @param[in] dst_subresource The destination subresource layer.
            //! @param[in] src The source texture view.
            //! @param[in] sampler The sampler used when sampling source texture.
            //! @param[in] top_left The position of the top-left corner of the source texture in destination texture.
            //! @param[in] top_right The position of the top-right corner of the source texture in destination texture.
            //! @param[in] bottom_left The position of the bottom-left corner of the source texture in destination texture.
            //! @param[in] bottom_right The position of the bottom-right corner of the source texture in destination texture.
            virtual void blit(RHI::ITexture* dst, const RHI::SubresourceIndex& dst_subresource,
                const RHI::TextureViewDesc& src,
                const RHI::SamplerDesc& sampler,
                const Float2U& top_left, const Float2U& top_right,
                const Float2U& bottom_left, const Float2U& bottom_right) = 0;

            virtual RV commit(RHI::ICommandBuffer* graphics_cmdbuf, bool submit_and_wait) = 0;
        };

        //! Creates a new blit context.
        LUNA_RHI_UTILITY_API R<Ref<IBlitContext>> new_blit_context(RHI::IDevice* device, RHI::Format dst_format);

        //! @}
    }
}