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
            if(cell_attachment)
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

        static void draw_default_table_layout(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState&, void*)
        {
            NodeRenderLayout layout;
            if(!ctx.get_node_render_layout(ctx.current_node_index(), layout)) return;
            const TableStyle& style = table_desc(node).style;
            u32 columns = layout.table_columns;
            u32 rows = layout.table_rows;
            if(!columns || !rows) return;

            u32 child = node.first_child;
            for(u32 row = 0; row < rows; ++row)
            {
                for(u32 col = 0; col < columns; ++col)
                {
                    ColorOverride color = table_cell_color(node, style, child, row, col, columns);
                    if(color.enabled)
                    {
                        ctx.draw_rect(RectF(layout.table_column_offsets[col], layout.table_row_offsets[row],
                            layout.table_column_widths[col], layout.table_row_heights[row]), clip_rect, color.color, 0.0f);
                    }
                    if(child != U32_MAX)
                    {
                        const Node* child_node = ctx.get_node(child);
                        child = child_node ? child_node->next_sibling : U32_MAX;
                    }
                }
            }

            f32 table_top = layout.table_row_offsets[0];
            f32 table_bottom = layout.table_row_offsets.back() + layout.table_row_heights.back();
            f32 table_left = layout.table_column_offsets[0];
            f32 table_right = layout.table_column_offsets.back() + layout.table_column_widths.back();
            if(style.column_separators && style.separator_size > 0.0f)
            {
                for(u32 col = 0; col + 1 < columns; ++col)
                {
                    f32 x = layout.table_column_offsets[col] + layout.table_column_widths[col];
                    ctx.draw_rect(RectF(x, table_top, style.separator_size, max(table_bottom - table_top, 1.0f)),
                        clip_rect, style.separator_color, 0.0f);
                }
            }
            if(style.row_separators && style.separator_size > 0.0f)
            {
                for(u32 row = 0; row + 1 < rows; ++row)
                {
                    f32 y = layout.table_row_offsets[row] + layout.table_row_heights[row];
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
