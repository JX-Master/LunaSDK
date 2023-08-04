/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TextureView.hpp
* @author JXMaster
* @date 2023/7/24
*/
#pragma once
#include "Device.hpp"

namespace Luna
{
    namespace RHI
    {
        // Checks whether one view requries dedicated view object.
        bool require_view_object(const TextureDesc& texture_desc, const TextureViewDesc& validated_desc);

        struct TextureView
        {
            lustruct("RHI::TextureView", "{bca7d328-7c90-4c11-97b2-146814fd460b}");

            NSPtr<MTL::Texture> m_texture;

            RV init(const TextureViewDesc& validated_desc);
        };
    }
}