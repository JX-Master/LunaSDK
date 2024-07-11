/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file WidgetCommon.hpp
* @author JXMaster
* @date 2024/7/9
*/
#pragma once
#include "../../Widgets.hpp"
#include "../Context.hpp"

namespace Luna
{
    namespace GUI
    {
        inline struct nk_rect encode_rect(const RectF& r)
        {
            struct nk_rect rect;
            rect.x = r.offset_x;
            rect.y = r.offset_y;
            rect.w = r.width;
            rect.h = r.height;
            return rect;
        }
        inline RectF decode_rect(const struct nk_rect& rect)
        {
            return RectF(rect.x, rect.y, rect.w, rect.h);
        }
        inline struct nk_vec2 encode_vec2(const Float2U& vec)
        {
            struct nk_vec2 r;
            r.x = vec.x;
            r.y = vec.y;
            return r;   
        }
        inline Float2U decode_vec2(const struct nk_vec2& vec)
        {
            return Float2U(vec.x, vec.y);
        }
        inline struct nk_image encode_image(RHI::ITexture* image, const OffsetRectU& offsets)
        {
            struct nk_image img;
            img.handle.ptr = image;
            auto desc = image->get_desc();
            img.w = desc.width;
            img.h = desc.height;
            img.region[0] = offsets.left;
            img.region[1] = offsets.top;
            img.region[2] = offsets.right;
            img.region[3] = offsets.bottom;
            return img;
        }
    }
}