/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TextureView.h
* @author JXMaster
* @date 2023/7/24
*/
#pragma once
#include "Device.h"

namespace Luna
{
    namespace RHI
    {
        // Checks whether one view requries dedicated view object.
        bool require_view_object(const TextureDesc& texture_desc, const TextureViewDesc& validated_desc);

        struct TextureView
        {
            lustruct("RHI::TextureView", "{bca7d328-7c90-4c11-97b2-146814fd460b}");

            id<MTLTexture> m_texture;

            RV init(const TextureViewDesc& validated_desc);
        };
    }
}