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
#include <Luna/RHI/Device.hpp>

#ifndef LUNA_RHI_UTILITY_API
#define LUNA_RHI_UTILITY_API
#endif

namespace Luna
{
    namespace RHIUtility
    {
        //! @addtogroup RHIUtility
        //! @{

        struct IMipmapGenerationContext : virtual RHI::IDeviceChild
        {
            luiid("beb3c88c-1c7d-4de6-af48-15364a89b5e2");

            virtual void reset() = 0;

            virtual void generate_mipmaps(RHI::ITexture* tex, u32 source_mip = 0, u32 num_gen_mips = U32_MAX) = 0;
        
            virtual RV commit(RHI::ICommandBuffer* compute_cmdbuf, bool submit_and_wait) = 0;
        };

        LUNA_RHI_UTILITY_API R<Ref<IMipmapGenerationContext>> new_mipmap_generation_context(RHI::IDevice* device);

        //! @}
    }
}