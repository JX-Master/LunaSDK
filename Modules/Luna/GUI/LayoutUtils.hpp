/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file LayoutUtils.hpp
* @author JXMaster
* @date 2024/7/22
*/
#pragma once
#include "Widget.hpp"

#ifndef LUNA_GUI_API
#define LUNA_GUI_API
#endif

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API OffsetRectF calc_widget_bounding_rect(const OffsetRectF& parent_bounding_rect, const OffsetRectF& anthor, const OffsetRectF& offset);

        LUNA_GUI_API void calc_hlayout(Widget** widgets, usize num_widgets, 
            f32 total_size, const f32* total_size_other,
            f32* out_allocated_size,
            f32* out_required_size,
            f32* out_preferred_size,
            f32* out_filling_size);

        LUNA_GUI_API void calc_vlayout(Widget** widgets, usize num_widgets, 
            f32 total_size, const f32* total_size_other,
            f32* out_allocated_size,
            f32* out_required_size,
            f32* out_preferred_size,
            f32* out_filling_size);
    }
}