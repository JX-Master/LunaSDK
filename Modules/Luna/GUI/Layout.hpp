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
        struct Size
        {
            f32 width = 0.0f;
            f32 height = 0.0f;

            static Size fixed(f32 width, f32 height)
            {
                Size r;
                r.width = width;
                r.height = height;
                return r;
            }
        };

        enum class SizePolicy : u8
        {
            fixed,
            hug,
            fill
        };

        enum class LayoutMainAxisAlignment : u8
        {
            begin,
            center,
            end,
            space_between
        };

        enum class LayoutCrossAxisAlignment : u8
        {
            begin,
            center,
            end,
            stretch
        };

        struct EdgeInsets
        {
            f32 left = 0.0f;
            f32 top = 0.0f;
            f32 right = 0.0f;
            f32 bottom = 0.0f;

            static EdgeInsets all(f32 value)
            {
                EdgeInsets r;
                r.left = value;
                r.top = value;
                r.right = value;
                r.bottom = value;
                return r;
            }

            static EdgeInsets xy(f32 x, f32 y)
            {
                EdgeInsets r;
                r.left = x;
                r.right = x;
                r.top = y;
                r.bottom = y;
                return r;
            }
        };

        struct LayoutMetrics
        {
            Float2U min_size = Float2U(0.0f);
            Float2U preferred_size = Float2U(0.0f);
            Float2U max_size = Float2U(F32_MAX, F32_MAX);
        };

        struct LayoutStyle
        {
            SizePolicy width_policy = SizePolicy::hug;
            SizePolicy height_policy = SizePolicy::hug;
            f32 fixed_width_value = 0.0f;
            f32 fixed_height_value = 0.0f;
            f32 fill_weight_x = 1.0f;
            f32 fill_weight_y = 1.0f;
            Float2U min_size = Float2U(0.0f);
            Float2U max_size = Float2U(F32_MAX, F32_MAX);

            static LayoutStyle hug()
            {
                return LayoutStyle();
            }

            static LayoutStyle fill(f32 weight = 1.0f)
            {
                LayoutStyle r;
                r.width_policy = SizePolicy::fill;
                r.height_policy = SizePolicy::fill;
                r.fill_weight_x = weight;
                r.fill_weight_y = weight;
                return r;
            }

            static LayoutStyle fill_width(f32 weight = 1.0f)
            {
                LayoutStyle r;
                r.width_policy = SizePolicy::fill;
                r.fill_weight_x = weight;
                return r;
            }

            static LayoutStyle fill_height(f32 weight = 1.0f)
            {
                LayoutStyle r;
                r.height_policy = SizePolicy::fill;
                r.fill_weight_y = weight;
                return r;
            }

            static LayoutStyle fixed(f32 width, f32 height)
            {
                LayoutStyle r;
                r.width_policy = SizePolicy::fixed;
                r.height_policy = SizePolicy::fixed;
                r.fixed_width_value = width;
                r.fixed_height_value = height;
                return r;
            }

            static LayoutStyle fixed_width(f32 width)
            {
                LayoutStyle r;
                r.width_policy = SizePolicy::fixed;
                r.fixed_width_value = width;
                return r;
            }

            static LayoutStyle fixed_height(f32 height)
            {
                LayoutStyle r;
                r.height_policy = SizePolicy::fixed;
                r.fixed_height_value = height;
                return r;
            }
        };

        struct LayoutDesc
        {
            EdgeInsets padding;
            f32 gap = 6.0f;
            LayoutMainAxisAlignment main_axis_alignment = LayoutMainAxisAlignment::begin;
            LayoutCrossAxisAlignment cross_axis_alignment = LayoutCrossAxisAlignment::stretch;
        };

        enum class GridSizingMode : u8
        {
            fixed_cell_size,
            fixed_columns
        };

        struct GridLayoutDesc
        {
            GridSizingMode sizing_mode = GridSizingMode::fixed_cell_size;
            Float2U cell_size = Float2U(96.0f, 118.0f);
            u32 columns = 4;
            EdgeInsets padding = EdgeInsets::all(6.0f);
            Float2U gap = Float2U(8.0f, 8.0f);
            LayoutCrossAxisAlignment cell_cross_axis_alignment = LayoutCrossAxisAlignment::stretch;
        };

        struct CanvasLayoutDesc
        {
            EdgeInsets padding;
            bool clip_children = true;
        };

        struct CanvasItemLayout
        {
            Float2U anchor_min = Float2U(0.0f, 0.0f);
            Float2U anchor_max = Float2U(0.0f, 0.0f);
            Float2U offset_min = Float2U(0.0f, 0.0f);
            Float2U offset_max = Float2U(0.0f, 0.0f);

            static CanvasItemLayout fixed(const Float2U& position, const Float2U& size)
            {
                CanvasItemLayout r;
                r.offset_min = position;
                r.offset_max = position + size;
                return r;
            }

            static CanvasItemLayout anchored(const Float2U& anchor, const Float2U& anchored_position, const Float2U& size, const Float2U& pivot = Float2U(0.5f, 0.5f))
            {
                CanvasItemLayout r;
                r.anchor_min = anchor;
                r.anchor_max = anchor;
                Float2U min_offset = anchored_position - size * pivot;
                r.offset_min = min_offset;
                r.offset_max = min_offset + size;
                return r;
            }

            static CanvasItemLayout stretch(const EdgeInsets& insets = EdgeInsets())
            {
                CanvasItemLayout r;
                r.anchor_max = Float2U(1.0f, 1.0f);
                r.offset_min = Float2U(insets.left, insets.top);
                r.offset_max = Float2U(-insets.right, -insets.bottom);
                return r;
            }
        };

        enum class TreeNodeFlag : u32
        {
            none = 0x00,
            selected = 0x01,
            leaf = 0x02,
            default_open = 0x04,
            open_on_arrow = 0x08
        };

        enum class TabBarFlag : u32
        {
            none = 0x00,
            reorderable = 0x01,
            fitting_shrink = 0x02,
            fitting_scroll = 0x04,
            auto_select_new_tabs = 0x08
        };

        enum class TabItemFlag : u32
        {
            none = 0x00,
            selected = 0x01,
            no_close_button = 0x02,
            unsaved_document = 0x04,
            no_reorder = 0x08,
            button = 0x10
        };

        enum class PopupFlag : u32
        {
            none = 0x00,
            managed = 0x01,
            close_on_outside_click = 0x02,
            close_on_escape = 0x04,
            close_on_blur = 0x08,
            modal = 0x10
        };

        enum class NumericEditFlag : u32
        {
            none = 0x00,
            input_on_double_click = 0x01
        };

        struct PopupDesc
        {
            Float2U position = Float2U(0.0f);
            Size size;
            PopupFlag flags = PopupFlag::managed | PopupFlag::close_on_outside_click | PopupFlag::close_on_escape | PopupFlag::close_on_blur;
        };

        struct TooltipDesc
        {
            Float2U offset = Float2U(14.0f, 18.0f);
            Size size;
            f32 delay = 0.35f;
            f32 max_width = 360.0f;
        };

        enum class DockPanelMode : u8
        {
            docking,
            floating
        };

        struct DockPanelStyle
        {
            bool title_bar = true;
            bool close_button = true;
            bool resize_border = true;
            f32 title_bar_height = 28.0f;
            f32 border_size = 1.0f;
            f32 resize_border_size = 6.0f;
            DockPanelMode initial_mode = DockPanelMode::docking;
            Float2U floating_position = Float2U(24.0f, 24.0f);
            Float2U floating_size = Float2U(320.0f, 220.0f);
            Float2U min_floating_size = Float2U(120.0f, 80.0f);
            Float4U background_color = Float4U(0.09f, 0.11f, 0.14f, 0.96f);
            Float4U title_bar_color = Float4U(0.13f, 0.17f, 0.22f, 1.0f);
            Float4U active_title_bar_color = Float4U(0.16f, 0.24f, 0.36f, 1.0f);
            Float4U border_color = Float4U(0.24f, 0.29f, 0.36f, 1.0f);
        };

        enum class TableTrackSizePolicy : u8
        {
            hug,
            fixed
        };

        struct TableTrackSize
        {
            TableTrackSizePolicy policy = TableTrackSizePolicy::hug;
            f32 value = 0.0f;

            static TableTrackSize hug()
            {
                return TableTrackSize();
            }

            static TableTrackSize fixed(f32 value)
            {
                TableTrackSize r;
                r.policy = TableTrackSizePolicy::fixed;
                r.value = value;
                return r;
            }
        };

        enum class TableBackgroundMode : u8
        {
            none,
            solid,
            alternate_rows,
            alternate_columns
        };

        struct ColorOverride
        {
            bool enabled = false;
            Float4U color = Float4U(0.0f);

            static ColorOverride none()
            {
                return ColorOverride();
            }

            static ColorOverride make(const Float4U& value)
            {
                ColorOverride r;
                r.enabled = true;
                r.color = value;
                return r;
            }
        };

        struct TableStyle
        {
            EdgeInsets padding = EdgeInsets::xy(6.0f, 4.0f);
            f32 border_size = 0.0f;
            Float4U border_color = Float4U(0.25f, 0.28f, 0.32f, 1.0f);
            TableBackgroundMode background_mode = TableBackgroundMode::none;
            Float4U background_color = Float4U(0.10f, 0.12f, 0.14f, 0.72f);
            Float4U alternate_background_color = Float4U(0.13f, 0.15f, 0.18f, 0.72f);
            Vector<ColorOverride> row_colors;
            Vector<ColorOverride> column_colors;
            Vector<ColorOverride> cell_colors;
            bool row_separators = false;
            bool column_separators = false;
            f32 separator_size = 1.0f;
            Float4U separator_color = Float4U(0.28f, 0.32f, 0.36f, 1.0f);
            bool resize_fixed_rows = false;
            bool resize_fixed_columns = false;
            f32 resize_hit_size = 6.0f;
        };

        struct TableDesc
        {
            u32 columns = 1;
            Vector<TableTrackSize> column_sizes;
            Vector<TableTrackSize> row_sizes;
            TableStyle style;
        };
    }
}
