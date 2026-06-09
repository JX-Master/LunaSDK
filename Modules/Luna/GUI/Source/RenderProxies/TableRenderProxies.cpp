/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#include "TableRenderProxies.hpp"
#include "../Nodes/LayoutNodes.hpp"
#include "../GUI.hpp"

namespace Luna
{
    namespace GUI
    {
        static ColorOverride table_cell_color(const Node& node, const TableStyle& style, u32 child_index, u32 row, u32 col, u32 columns)
        {
            usize cell_index = (usize)row * columns + col;
            const TableCellAttachment* cell_attachment = child_index != U32_MAX ? table_cell_attachment(node, child_index) : nullptr;
            if(cell_attachment && cell_attachment->color_enabled)
            {
                ColorOverride color;
                color.enabled = true;
                color.color = cell_attachment->color;
                return color;
            }
            if(cell_index < style.cell_colors.size() && style.cell_colors[cell_index].enabled)
            {
                return style.cell_colors[cell_index];
            }
            if(row < style.row_colors.size() && style.row_colors[row].enabled)
            {
                return style.row_colors[row];
            }
            if(col < style.column_colors.size() && style.column_colors[col].enabled)
            {
                return style.column_colors[col];
            }
            if(style.background_mode == TableBackgroundMode::solid)
            {
                ColorOverride color;
                color.enabled = true;
                color.color = style.background_color;
                return color;
            }
            if(style.background_mode == TableBackgroundMode::alternate_rows)
            {
                ColorOverride color;
                color.enabled = true;
                color.color = (row % 2) ? style.alternate_background_color : style.background_color;
                return color;
            }
            if(style.background_mode == TableBackgroundMode::alternate_columns)
            {
                ColorOverride color;
                color.enabled = true;
                color.color = (col % 2) ? style.alternate_background_color : style.background_color;
                return color;
            }
            return ColorOverride();
        }

        static bool axis_range_visible(f32 offset, f32 size, f32 clip_begin, f32 clip_end)
        {
            return size > 0.0f && offset < clip_end && offset + size > clip_begin;
        }

        static void visible_table_range(const Vector<f32>& offsets, const Vector<f32>& sizes, f32 clip_begin, f32 clip_end,
            u32& out_begin, u32& out_end)
        {
            out_begin = (u32)offsets.size();
            out_end = out_begin;
            for(u32 i = 0; i < offsets.size(); ++i)
            {
                if(axis_range_visible(offsets[i], sizes[i], clip_begin, clip_end))
                {
                    if(out_begin == offsets.size())
                    {
                        out_begin = i;
                    }
                    out_end = i + 1;
                }
                else if(out_begin != offsets.size() && offsets[i] >= clip_end)
                {
                    break;
                }
            }
            if(out_begin == offsets.size())
            {
                out_begin = 0;
                out_end = 0;
            }
        }

        static void draw_default_table_layout(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState&, void*)
        {
            NodeRenderLayout layout;
            if(!ctx.get_node_render_layout(ctx.current_node_index(), layout)) return;
            const TableStyle& style = table_desc(node).style;
            u32 columns = layout.table_columns;
            u32 rows = layout.table_rows;
            if(!columns || !rows) return;

            f32 clip_left = clip_rect.offset_x;
            f32 clip_right = clip_rect.offset_x + clip_rect.width;
            f32 clip_top = clip_rect.offset_y;
            f32 clip_bottom = clip_rect.offset_y + clip_rect.height;
            u32 visible_col_begin = 0;
            u32 visible_col_end = 0;
            u32 visible_row_begin = 0;
            u32 visible_row_end = 0;
            visible_table_range(layout.table_column_offsets, layout.table_column_widths, clip_left, clip_right, visible_col_begin, visible_col_end);
            visible_table_range(layout.table_row_offsets, layout.table_row_heights, clip_top, clip_bottom, visible_row_begin, visible_row_end);

            for(u32 row = visible_row_begin; row < visible_row_end; ++row)
            {
                for(u32 col = visible_col_begin; col < visible_col_end; ++col)
                {
                    const TableCellAttachment* cell = table_cell_attachment(node, row, col);
                    u32 child = cell ? cell->child_index : U32_MAX;
                    ColorOverride color = table_cell_color(node, style, child, row, col, columns);
                    if(color.enabled)
                    {
                        ctx.draw_rect(RectF(layout.table_column_offsets[col], layout.table_row_offsets[row],
                            layout.table_column_widths[col], layout.table_row_heights[row]), clip_rect, color.color, 0.0f);
                    }
                }
            }

            f32 table_top = layout.table_row_offsets[0];
            f32 table_bottom = layout.table_row_offsets.back() + layout.table_row_heights.back();
            f32 table_left = layout.table_column_offsets[0];
            f32 table_right = layout.table_column_offsets.back() + layout.table_column_widths.back();
            if(style.column_separators && style.separator_size > 0.0f)
            {
                for(u32 col = visible_col_begin; col < visible_col_end && col + 1 < columns; ++col)
                {
                    f32 x = layout.table_column_offsets[col] + layout.table_column_widths[col];
                    if(!axis_range_visible(x, style.separator_size, clip_left, clip_right)) continue;
                    ctx.draw_rect(RectF(x, table_top, style.separator_size, max(table_bottom - table_top, 1.0f)),
                        clip_rect, style.separator_color, 0.0f);
                }
            }
            if(style.row_separators && style.separator_size > 0.0f)
            {
                for(u32 row = visible_row_begin; row < visible_row_end && row + 1 < rows; ++row)
                {
                    f32 y = layout.table_row_offsets[row] + layout.table_row_heights[row];
                    if(!axis_range_visible(y, style.separator_size, clip_top, clip_bottom)) continue;
                    ctx.draw_rect(RectF(table_left, y, max(table_right - table_left, 1.0f), style.separator_size),
                        clip_rect, style.separator_color, 0.0f);
                }
            }
            if(style.border_size > 0.0f)
            {
                f32 b = style.border_size;
                ctx.draw_rect(RectF(rect.offset_x, rect.offset_y, rect.width, b), clip_rect, style.border_color, 0.0f);
                ctx.draw_rect(RectF(rect.offset_x, rect.offset_y + rect.height - b, rect.width, b), clip_rect, style.border_color, 0.0f);
                ctx.draw_rect(RectF(rect.offset_x, rect.offset_y, b, rect.height), clip_rect, style.border_color, 0.0f);
                ctx.draw_rect(RectF(rect.offset_x + rect.width - b, rect.offset_y, b, rect.height), clip_rect, style.border_color, 0.0f);
            }
        }

        RenderProxyDesc default_table_layout_render_proxy()
        {
            RenderProxyDesc desc;
            desc.draw = draw_default_table_layout;
            return desc;
        }
    }
}
