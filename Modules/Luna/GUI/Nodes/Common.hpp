/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#pragma once
#include "../Description.hpp"

namespace Luna
{
    namespace GUI
    {
        Float4U smooth_color(const Float4U& a, const Float4U& b, f32 t);
        LayoutMetrics fixed_height_metrics(f32 min_width, f32 preferred_width, f32 height);
        LayoutMetrics numeric_edit_metrics(const Node& node);

        struct NumericBinding
        {
            f32* f32_value = nullptr;
            i32* i32_value = nullptr;
            u8 value_count = 1;
            f32 min_value = 0.0f;
            f32 max_value = 0.0f;
            f32 step_value = 0.0f;
            NumericEditFlag flags = NumericEditFlag::none;
            bool f32_color = false;
            id_t color_owner_id = 0;
            ColorChannelPart color_part = ColorChannelPart::none;
        };

        struct ColorBinding
        {
            f32* f32_value = nullptr;
            u8* u8_value = nullptr;
            u32* u32_value = nullptr;
            ColorValueType type = ColorValueType::f32;
            u8 value_count = 3;
            id_t owner_id = 0;
            ColorChannelPart part = ColorChannelPart::none;
        };
    }
}
