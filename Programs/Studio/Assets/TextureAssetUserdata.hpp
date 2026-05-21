/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#pragma once
#include "Texture.hpp"
#include <Luna/RHIUtility/MipmapGenerationContext.hpp>
#include <Luna/Runtime/SpinLock.hpp>
#include "TextureAssetUserdata.generated.hpp"

namespace Luna
{
    struct [[luna::struct("{816CDA20-AB1C-4E24-A7CE-59E2EFE9BE1E}")]] TextureAssetUserdata
    {
        Ref<RHIUtility::IMipmapGenerationContext> m_mipmap_generation_context;
        SpinLock m_lock;

        RV init();

        RV generate_mipmaps(RHI::ITexture* resource_with_most_detailed_mip, RHI::ICommandBuffer* compute_cmdbuf);
    };
}
