/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file LayoutUtils.cpp
* @author JXMaster
* @date 2024/7/22
*/
#include "../LayoutUtils.hpp"

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API OffsetRectF calc_widget_bounding_rect(const OffsetRectF& parent_bounding_rect, const OffsetRectF& anthor, const OffsetRectF& offset)
        {
            // Calculate anthor point.
            f32 ax1 = lerp(parent_bounding_rect.left, parent_bounding_rect.right, anthor.left);
            f32 ax2 = lerp(parent_bounding_rect.left, parent_bounding_rect.right, anthor.right);
            f32 ay1 = lerp(parent_bounding_rect.top, parent_bounding_rect.bottom, anthor.top);
            f32 ay2 = lerp(parent_bounding_rect.top, parent_bounding_rect.bottom, anthor.bottom);
            // Calculate size.
            OffsetRectF r;
            r.left = ax1 + offset.left;
            r.right = ax2 + offset.right;
            r.top = ay1 + offset.top;
            r.bottom = ay2 + offset.bottom;
            return r;
        }
    }
}