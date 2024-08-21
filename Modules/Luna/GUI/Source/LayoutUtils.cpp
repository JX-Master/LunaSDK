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
        LUNA_GUI_API void calc_hlayout(IWidget** widgets, usize num_widgets, 
            f32 total_size, const f32* total_size_other,
            f32* out_allocated_size,
            f32* out_required_size,
            f32* out_preferred_size,
            f32* out_filling_size)
        {
            // Arrange in X.
            f32 allocated = 0;
            // Allocate required size.
            for(usize i = 0; i < num_widgets; ++i)
            {
                out_required_size[i] = widgets[i]->get_desired_size_x(DesiredSizeType::required, total_size_other);
                out_allocated_size[i] = out_required_size[i];
                allocated += out_required_size[i];
            }
            // Allocate preferred size.
            if(total_size > allocated)
            {
                f32 preferred_size_sum = 0;
                for(usize i = 0; i < num_widgets; ++i)
                {
                    out_preferred_size[i] = max(widgets[i]->get_desired_size_x(DesiredSizeType::preferred, total_size_other), out_required_size[i]);
                    preferred_size_sum += out_preferred_size[i];
                }
                if(preferred_size_sum <= total_size)
                {
                    for(usize i = 0; i < num_widgets; ++i)
                    {
                        out_allocated_size[i] = out_preferred_size[i];
                    }
                    allocated = preferred_size_sum;
                }
                else
                {
                    f32 ratio = total_size / preferred_size_sum;
                    for(usize i = 0; i < num_widgets; ++i)
                    {
                        out_allocated_size[i] = out_preferred_size[i] * ratio;
                    }
                    allocated = total_size;
                }
            }
            else
            {
                memzero(out_preferred_size, sizeof(f32) * num_widgets);
            }
            // Allocate filling size.
            if(total_size > allocated)
            {
                f32 total_filling_size = total_size - allocated;
                f32 filling_size_weight = 0;
                for(usize i = 0; i < num_widgets; ++i)
                {
                    out_filling_size[i] = widgets[i]->get_desired_size_x(DesiredSizeType::filling, total_size_other);
                    filling_size_weight += out_filling_size[i];
                }
                if(filling_size_weight > 0)
                {
                    f32 filling_size_per_unit = total_filling_size / filling_size_weight;
                    for(usize i = 0; i < num_widgets; ++i)
                    {
                        out_allocated_size[i] += filling_size_per_unit * out_filling_size[i];
                    }
                    allocated = total_size;
                }
            }
            else
            {
                memzero(out_filling_size, sizeof(f32) * num_widgets);
            }
        }
        LUNA_GUI_API void calc_vlayout(IWidget** widgets, usize num_widgets, 
            f32 total_size, const f32* total_size_other,
            f32* out_allocated_size,
            f32* out_required_size,
            f32* out_preferred_size,
            f32* out_filling_size)
        {
            // Arrange in Y.
            f32 allocated = 0;
            // Allocate required size.
            for(usize i = 0; i < num_widgets; ++i)
            {
                out_required_size[i] = widgets[i]->get_desired_size_y(DesiredSizeType::required, total_size_other);
                out_allocated_size[i] = out_required_size[i];
                allocated += out_required_size[i];
            }
            // Allocate preferred size.
            if(total_size > allocated)
            {
                f32 preferred_size_sum = 0;
                for(usize i = 0; i < num_widgets; ++i)
                {
                    out_preferred_size[i] = max(widgets[i]->get_desired_size_y(DesiredSizeType::preferred, total_size_other), out_required_size[i]);
                    preferred_size_sum += out_preferred_size[i];
                }
                if(preferred_size_sum <= total_size)
                {
                    for(usize i = 0; i < num_widgets; ++i)
                    {
                        out_allocated_size[i] = out_preferred_size[i];
                    }
                    allocated = preferred_size_sum;
                }
                else
                {
                    f32 ratio = total_size / preferred_size_sum;
                    for(usize i = 0; i < num_widgets; ++i)
                    {
                        out_allocated_size[i] = out_preferred_size[i] * ratio;
                    }
                    allocated = total_size;
                }
            }
            else
            {
                memzero(out_preferred_size, sizeof(f32) * num_widgets);
            }
            // Allocate filling size.
            if(total_size > allocated)
            {
                f32 total_filling_size = total_size - allocated;
                f32 filling_size_weight = 0;
                for(usize i = 0; i < num_widgets; ++i)
                {
                    out_filling_size[i] = widgets[i]->get_desired_size_y(DesiredSizeType::filling, total_size_other);
                    filling_size_weight += out_filling_size[i];
                }
                if(filling_size_weight > 0)
                {
                    f32 filling_size_per_unit = total_filling_size / filling_size_weight;
                    for(usize i = 0; i < num_widgets; ++i)
                    {
                        out_allocated_size[i] += filling_size_per_unit * out_filling_size[i];
                    }
                    allocated = total_size;
                }
            }
            else
            {
                memzero(out_filling_size, sizeof(f32) * num_widgets);
            }
        }
    }
}