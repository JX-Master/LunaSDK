/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Layout.hpp
* @author JXMaster
* @date 2026/5/22
*/
#pragma once
#include "Base.hpp"

namespace Luna
{
    namespace GUI
    {
        struct GUISize
        {
            f32 width = 0.0f;
            f32 height = 0.0f;

            static GUISize fixed(f32 width, f32 height)
            {
                GUISize r;
                r.width = width;
                r.height = height;
                return r;
            }
        };

        enum class GUISizePolicy : u8
        {
            fixed,
            hug,
            fill
        };

        enum class GUILayoutMainAxisAlignment : u8
        {
            begin,
            center,
            end,
            space_between
        };

        enum class GUILayoutCrossAxisAlignment : u8
        {
            begin,
            center,
            end,
            stretch
        };

        struct GUIEdgeInsets
        {
            f32 left = 0.0f;
            f32 top = 0.0f;
            f32 right = 0.0f;
            f32 bottom = 0.0f;

            static GUIEdgeInsets all(f32 value)
            {
                GUIEdgeInsets r;
                r.left = value;
                r.top = value;
                r.right = value;
                r.bottom = value;
                return r;
            }

            static GUIEdgeInsets xy(f32 x, f32 y)
            {
                GUIEdgeInsets r;
                r.left = x;
                r.right = x;
                r.top = y;
                r.bottom = y;
                return r;
            }
        };

        struct GUILayoutMetrics
        {
            Float2U min_size = Float2U(0.0f);
            Float2U preferred_size = Float2U(0.0f);
            Float2U max_size = Float2U(F32_MAX, F32_MAX);
        };

        struct GUILayoutStyle
        {
            GUISizePolicy width_policy = GUISizePolicy::hug;
            GUISizePolicy height_policy = GUISizePolicy::hug;
            f32 fixed_width_value = 0.0f;
            f32 fixed_height_value = 0.0f;
            f32 fill_weight_x = 1.0f;
            f32 fill_weight_y = 1.0f;
            Float2U min_size = Float2U(0.0f);
            Float2U max_size = Float2U(F32_MAX, F32_MAX);

            static GUILayoutStyle hug()
            {
                return GUILayoutStyle();
            }

            static GUILayoutStyle fill(f32 weight = 1.0f)
            {
                GUILayoutStyle r;
                r.width_policy = GUISizePolicy::fill;
                r.height_policy = GUISizePolicy::fill;
                r.fill_weight_x = weight;
                r.fill_weight_y = weight;
                return r;
            }

            static GUILayoutStyle fill_width(f32 weight = 1.0f)
            {
                GUILayoutStyle r;
                r.width_policy = GUISizePolicy::fill;
                r.fill_weight_x = weight;
                return r;
            }

            static GUILayoutStyle fill_height(f32 weight = 1.0f)
            {
                GUILayoutStyle r;
                r.height_policy = GUISizePolicy::fill;
                r.fill_weight_y = weight;
                return r;
            }

            static GUILayoutStyle fixed(f32 width, f32 height)
            {
                GUILayoutStyle r;
                r.width_policy = GUISizePolicy::fixed;
                r.height_policy = GUISizePolicy::fixed;
                r.fixed_width_value = width;
                r.fixed_height_value = height;
                return r;
            }

            static GUILayoutStyle fixed_width(f32 width)
            {
                GUILayoutStyle r;
                r.width_policy = GUISizePolicy::fixed;
                r.fixed_width_value = width;
                return r;
            }

            static GUILayoutStyle fixed_height(f32 height)
            {
                GUILayoutStyle r;
                r.height_policy = GUISizePolicy::fixed;
                r.fixed_height_value = height;
                return r;
            }
        };

        struct GUILayoutDesc
        {
            GUIEdgeInsets padding;
            f32 gap = 6.0f;
            GUILayoutMainAxisAlignment main_axis_alignment = GUILayoutMainAxisAlignment::begin;
            GUILayoutCrossAxisAlignment cross_axis_alignment = GUILayoutCrossAxisAlignment::stretch;
        };

        enum class GUIDockPanelMode : u8
        {
            docking,
            floating
        };

        struct GUIDockPanelStyle
        {
            bool title_bar = true;
            bool close_button = true;
            bool resize_border = true;
            f32 title_bar_height = 28.0f;
            f32 border_size = 1.0f;
            f32 resize_border_size = 6.0f;
            GUIDockPanelMode initial_mode = GUIDockPanelMode::docking;
            Float2U floating_position = Float2U(24.0f, 24.0f);
            Float2U floating_size = Float2U(320.0f, 220.0f);
            Float2U min_floating_size = Float2U(120.0f, 80.0f);
            Float4U background_color = Float4U(0.09f, 0.11f, 0.14f, 0.96f);
            Float4U title_bar_color = Float4U(0.13f, 0.17f, 0.22f, 1.0f);
            Float4U active_title_bar_color = Float4U(0.16f, 0.24f, 0.36f, 1.0f);
            Float4U border_color = Float4U(0.24f, 0.29f, 0.36f, 1.0f);
        };

        enum class GUITableTrackSizePolicy : u8
        {
            hug,
            fixed
        };

        struct GUITableTrackSize
        {
            GUITableTrackSizePolicy policy = GUITableTrackSizePolicy::hug;
            f32 value = 0.0f;

            static GUITableTrackSize hug()
            {
                return GUITableTrackSize();
            }

            static GUITableTrackSize fixed(f32 value)
            {
                GUITableTrackSize r;
                r.policy = GUITableTrackSizePolicy::fixed;
                r.value = value;
                return r;
            }
        };

        enum class GUITableBackgroundMode : u8
        {
            none,
            solid,
            alternate_rows,
            alternate_columns
        };

        struct GUIColorOverride
        {
            bool enabled = false;
            Float4U color = Float4U(0.0f);

            static GUIColorOverride none()
            {
                return GUIColorOverride();
            }

            static GUIColorOverride make(const Float4U& value)
            {
                GUIColorOverride r;
                r.enabled = true;
                r.color = value;
                return r;
            }
        };

        struct GUITableStyle
        {
            GUIEdgeInsets padding = GUIEdgeInsets::xy(6.0f, 4.0f);
            f32 border_size = 0.0f;
            Float4U border_color = Float4U(0.25f, 0.28f, 0.32f, 1.0f);
            GUITableBackgroundMode background_mode = GUITableBackgroundMode::none;
            Float4U background_color = Float4U(0.10f, 0.12f, 0.14f, 0.72f);
            Float4U alternate_background_color = Float4U(0.13f, 0.15f, 0.18f, 0.72f);
            Vector<GUIColorOverride> row_colors;
            Vector<GUIColorOverride> column_colors;
            Vector<GUIColorOverride> cell_colors;
            bool row_separators = false;
            bool column_separators = false;
            f32 separator_size = 1.0f;
            Float4U separator_color = Float4U(0.28f, 0.32f, 0.36f, 1.0f);
            bool resize_fixed_rows = false;
            bool resize_fixed_columns = false;
            f32 resize_hit_size = 6.0f;
        };

        struct GUITableDesc
        {
            u32 columns = 1;
            Vector<GUITableTrackSize> column_sizes;
            Vector<GUITableTrackSize> row_sizes;
            GUITableStyle style;
        };
    }
}
