/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TextureView.mm
* @author JXMaster
* @date 2023/7/24
*/
#include "TextureView.h"
#include "Resource.h"

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
            NSRange mip_range;
            mip_range.location = validated_desc.mip_slice;
            mip_range.length = validated_desc.mip_size;
            NSRange array_range;
            array_range.location = validated_desc.array_slice;
            array_range.length = validated_desc.array_size;
            m_texture = [texture->m_texture newTextureViewWithPixelFormat:encode_pixel_format(validated_desc.format) 
                textureType:encode_texture_view_type(validated_desc.type) 
                levels:mip_range 
                slices:array_range];
            if(!m_texture) return BasicError::bad_platform_call();
            return ok;
        }
    }
}
