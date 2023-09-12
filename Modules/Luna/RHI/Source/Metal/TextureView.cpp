/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TextureView.cpp
* @author JXMaster
* @date 2023/7/24
*/
#include "TextureView.hpp"
#include "Resource.hpp"

namespace Luna
{
    namespace RHI
    {
        bool require_view_object(const TextureDesc& texture_desc, const TextureViewDesc& validated_desc)
        {
            if(validated_desc.mip_slice == 0 && validated_desc.mip_size == texture_desc.mip_levels &&
            validated_desc.array_slice == 0 && validated_desc.array_size == texture_desc.array_size &&
            validated_desc.format == texture_desc.format)
            {
                return false;
            }
            return true;
        }
        RV TextureView::init(const TextureViewDesc& validated_desc)
        {
            Texture* texture = cast_object<Texture>(validated_desc.texture);
            m_texture = box(texture->m_texture->newTextureView(encode_pixel_format(validated_desc.format), 
                encode_texture_view_type(validated_desc.type), 
                NS::Range(validated_desc.mip_slice, validated_desc.mip_size),
                NS::Range(validated_desc.array_slice, validated_desc.array_size)));
            if(!m_texture) return BasicError::bad_platform_call();
            return ok;
        }
    }
}
