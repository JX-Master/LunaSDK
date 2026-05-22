/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUILayout.cpp
* @author JXMaster
* @date 2026/5/21
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "GUI.hpp"

namespace Luna
{
    namespace GUI
    {
        void GUIContext::measure_table_tracks(u32 node_index, Vector<f32>& out_column_widths, Vector<f32>& out_row_heights, bool preferred)
        {
            const GUINode& node = m_submitted_desc.nodes[node_index];
            u32 columns = table_columns(node);
            u32 rows = table_rows(m_submitted_desc, node);
            out_column_widths.assign(columns, 1.0f);
            out_row_heights.assign(rows, 1.0f);

            PersistentItemState& persistent = get_or_create_persistent_state(node.id);
            for(u32 col = 0; col < columns; ++col)
            {
                const GUITableTrackSize& size = table_track_size(node, true, col);
                if(size.policy == GUITableTrackSizePolicy::fixed)
                {
                    f32 value = size.value;
                    if(col < persistent.table_column_sizes.size() && persistent.table_column_sizes[col] > 0.0f)
                    {
                        value = persistent.table_column_sizes[col];
                    }
                    out_column_widths[col] = max(value, 1.0f);
                }
            }
            for(u32 row = 0; row < rows; ++row)
            {
                const GUITableTrackSize& size = table_track_size(node, false, row);
                if(size.policy == GUITableTrackSizePolicy::fixed)
                {
                    f32 value = size.value;
                    if(row < persistent.table_row_sizes.size() && persistent.table_row_sizes[row] > 0.0f)
                    {
                        value = persistent.table_row_sizes[row];
                    }
                    out_row_heights[row] = max(value, 1.0f);
                }
            }

            u32 cell_index = 0;
            for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling, ++cell_index)
            {
                u32 row = cell_index / columns;
                u32 col = cell_index % columns;
                if(row >= rows) break;
                GUILayoutMetrics child_metrics = measure_node(child);
                Float2U child_size = preferred ? child_metrics.preferred_size : child_metrics.min_size;
                f32 cell_width = child_size.x + node.table_desc.style.padding.left + node.table_desc.style.padding.right;
                f32 cell_height = child_size.y + node.table_desc.style.padding.top + node.table_desc.style.padding.bottom;
                if(!table_track_is_fixed(node, true, col))
                {
                    out_column_widths[col] = max(out_column_widths[col], cell_width);
                }
                if(!table_track_is_fixed(node, false, row))
                {
                    out_row_heights[row] = max(out_row_heights[row], cell_height);
                }
            }
        }

        GUILayoutMetrics GUIContext::measure_node(u32 node_index)
        {
            if(m_layouts[node_index].metrics_valid)
            {
                return m_layouts[node_index].metrics;
            }

            const GUINode& node = m_submitted_desc.nodes[node_index];
            GUILayoutMetrics metrics;
            f32 font_size = 16.0f;
            f32 text_width = (f32)node.text.size() * font_size * 0.52f;
            switch(node.kind)
            {
            case GUINodeKind::text:
            {
                f32 w = max(text_width, 1.0f);
                metrics.min_size = Float2U(min(w, 32.0f), font_size + 4.0f);
                metrics.preferred_size = Float2U(w, font_size + 4.0f);
                metrics.max_size = Float2U(F32_MAX, font_size + 4.0f);
                break;
            }
            case GUINodeKind::button:
            {
                f32 w = max(text_width + 24.0f, 72.0f);
                metrics.min_size = Float2U(72.0f, 30.0f);
                metrics.preferred_size = Float2U(w, 30.0f);
                metrics.max_size = Float2U(F32_MAX, 30.0f);
                break;
            }
            case GUINodeKind::checkbox:
            {
                f32 w = max(text_width + 30.0f, 80.0f);
                metrics.min_size = Float2U(26.0f, 26.0f);
                metrics.preferred_size = Float2U(w, 26.0f);
                metrics.max_size = Float2U(F32_MAX, 26.0f);
                break;
            }
            case GUINodeKind::input_text:
                metrics.min_size = Float2U(80.0f, 30.0f);
                metrics.preferred_size = Float2U(240.0f, 30.0f);
                metrics.max_size = Float2U(F32_MAX, 30.0f);
                break;
            case GUINodeKind::image:
            {
                Float2U image_size(max(node.requested_size.width, 1.0f), max(node.requested_size.height, 1.0f));
                metrics.min_size = image_size;
                metrics.preferred_size = image_size;
                metrics.max_size = image_size;
                break;
            }
            case GUINodeKind::collapsing_header:
            {
                f32 w = max(text_width + 32.0f, 120.0f);
                metrics.min_size = Float2U(120.0f, 30.0f);
                metrics.preferred_size = Float2U(w, 30.0f);
                metrics.max_size = Float2U(F32_MAX, 30.0f);
                break;
            }
            case GUINodeKind::combo:
            {
                f32 w = max(text_width + 160.0f, 220.0f);
                metrics.min_size = Float2U(140.0f, 30.0f);
                metrics.preferred_size = Float2U(w, 30.0f);
                metrics.max_size = Float2U(F32_MAX, 30.0f);
                break;
            }
            case GUINodeKind::slider_float:
            case GUINodeKind::drag_float:
            {
                f32 w = max(text_width + 220.0f, 280.0f);
                metrics.min_size = Float2U(180.0f, 30.0f);
                metrics.preferred_size = Float2U(w, 30.0f);
                metrics.max_size = Float2U(F32_MAX, 30.0f);
                break;
            }
            case GUINodeKind::table_layout:
            {
                Vector<f32> min_columns;
                Vector<f32> min_rows;
                Vector<f32> preferred_columns;
                Vector<f32> preferred_rows;
                measure_table_tracks(node_index, min_columns, min_rows, false);
                measure_table_tracks(node_index, preferred_columns, preferred_rows, true);
                f32 min_width = node.table_desc.style.border_size * 2.0f;
                f32 preferred_width = node.table_desc.style.border_size * 2.0f;
                for(f32 v : min_columns) min_width += v;
                for(f32 v : preferred_columns) preferred_width += v;
                f32 min_height = node.table_desc.style.border_size * 2.0f;
                f32 preferred_height = node.table_desc.style.border_size * 2.0f;
                for(f32 v : min_rows) min_height += v;
                for(f32 v : preferred_rows) preferred_height += v;
                f32 separator_size = node.table_desc.style.separator_size;
                if(node.table_desc.style.column_separators && min_columns.size() > 1)
                {
                    f32 separators = separator_size * (f32)(min_columns.size() - 1);
                    min_width += separators;
                    preferred_width += separators;
                }
                if(node.table_desc.style.row_separators && min_rows.size() > 1)
                {
                    f32 separators = separator_size * (f32)(min_rows.size() - 1);
                    min_height += separators;
                    preferred_height += separators;
                }
                metrics.min_size = Float2U(max(min_width, 1.0f), max(min_height, 1.0f));
                metrics.preferred_size = Float2U(max(preferred_width, 1.0f), max(preferred_height, 1.0f));
                metrics.max_size = Float2U(F32_MAX, F32_MAX);
                break;
            }
            default:
            {
                bool horizontal = node.kind == GUINodeKind::h_layout;
                const GUIEdgeInsets& padding = node.layout_desc.padding;
                f32 gap = node.layout_desc.gap;
                f32 min_main = 0.0f;
                f32 preferred_main = 0.0f;
                f32 min_cross = 0.0f;
                f32 preferred_cross = 0.0f;
                u32 child_count = 0;
                for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
                {
                    GUILayoutMetrics child_metrics = measure_node(child);
                    const GUINode& child_node = m_submitted_desc.nodes[child];
                    f32 child_min_main = axis_value(child_metrics.min_size, horizontal);
                    f32 child_preferred_main = resolve_base_axis_size(child_node, child_metrics, horizontal);
                    f32 child_min_cross = axis_value(child_metrics.min_size, !horizontal);
                    f32 child_preferred_cross = resolve_base_axis_size(child_node, child_metrics, !horizontal);
                    min_main += child_min_main;
                    preferred_main += child_preferred_main;
                    min_cross = max(min_cross, child_min_cross);
                    preferred_cross = max(preferred_cross, child_preferred_cross);
                    ++child_count;
                }
                if(child_count > 1)
                {
                    min_main += gap * (f32)(child_count - 1);
                    preferred_main += gap * (f32)(child_count - 1);
                }
                f32 padding_main = horizontal ? padding.left + padding.right : padding.top + padding.bottom;
                f32 padding_cross = horizontal ? padding.top + padding.bottom : padding.left + padding.right;
                min_main += padding_main;
                preferred_main += padding_main;
                min_cross += padding_cross;
                preferred_cross += padding_cross;
                Float2U min_size;
                Float2U preferred_size;
                if(horizontal)
                {
                    min_size = Float2U(max(min_main, 1.0f), max(min_cross, 1.0f));
                    preferred_size = Float2U(max(preferred_main, 1.0f), max(preferred_cross, 1.0f));
                }
                else
                {
                    min_size = Float2U(max(min_cross, 1.0f), max(min_main, 1.0f));
                    preferred_size = Float2U(max(preferred_cross, 1.0f), max(preferred_main, 1.0f));
                }
                metrics.min_size = min_size;
                metrics.preferred_size = preferred_size;
                metrics.max_size = Float2U(F32_MAX, F32_MAX);
                break;
            }
            }

            metrics = apply_layout_style(node, metrics);
            m_layouts[node_index].metrics = metrics;
            m_layouts[node_index].metrics_valid = true;
            return metrics;
        }

        void GUIContext::arrange_table_node(u32 node_index, const RectF& rect, const RectF& clip_rect)
        {
            GUINode& node = m_submitted_desc.nodes[node_index];
            NodeLayout& layout = m_layouts[node_index];
            Vector<f32> column_widths;
            Vector<f32> row_heights;
            measure_table_tracks(node_index, column_widths, row_heights, true);
            u32 columns = (u32)column_widths.size();
            u32 rows = (u32)row_heights.size();
            layout.table_columns = columns;
            layout.table_rows = rows;
            layout.table_column_offsets.assign(columns, 0.0f);
            layout.table_column_widths = column_widths;
            layout.table_row_offsets.assign(rows, 0.0f);
            layout.table_row_heights = row_heights;
            if(!columns || !rows) return;

            f32 border_size = node.table_desc.style.border_size;
            f32 separator_size = node.table_desc.style.separator_size;
            f32 column_separator = node.table_desc.style.column_separators ? separator_size : 0.0f;
            f32 row_separator = node.table_desc.style.row_separators ? separator_size : 0.0f;
            f32 cursor_x = rect.offset_x + border_size;
            for(u32 col = 0; col < columns; ++col)
            {
                layout.table_column_offsets[col] = cursor_x;
                cursor_x += column_widths[col] + (col + 1 < columns ? column_separator : 0.0f);
            }
            f32 cursor_y = rect.offset_y + border_size;
            for(u32 row = 0; row < rows; ++row)
            {
                layout.table_row_offsets[row] = cursor_y;
                cursor_y += row_heights[row] + (row + 1 < rows ? row_separator : 0.0f);
            }

            u32 cell_index = 0;
            for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling, ++cell_index)
            {
                u32 row = cell_index / columns;
                u32 col = cell_index % columns;
                if(row >= rows) break;
                RectF cell_rect(layout.table_column_offsets[col], layout.table_row_offsets[row], column_widths[col], row_heights[row]);
                RectF child_rect(
                    cell_rect.offset_x + node.table_desc.style.padding.left,
                    cell_rect.offset_y + node.table_desc.style.padding.top,
                    max(cell_rect.width - node.table_desc.style.padding.left - node.table_desc.style.padding.right, 1.0f),
                    max(cell_rect.height - node.table_desc.style.padding.top - node.table_desc.style.padding.bottom, 1.0f));
                layout_node(child, child_rect, intersect_rect(cell_rect, clip_rect));
            }
        }

        RectF GUIContext::layout_node(u32 node_index, const RectF& rect, const RectF& clip_rect)
        {
            GUINode& node = m_submitted_desc.nodes[node_index];
            RectF effective_clip = intersect_rect(rect, clip_rect);
            m_layouts[node_index].rect = rect;
            m_layouts[node_index].clip_rect = effective_clip;

            if(node.kind != GUINodeKind::root)
            {
                ItemResult& result = get_or_create_current_result(node.id);
                result.states.insert_or_assign(Name("gui.rect"), Any(rect));
                result.states.insert_or_assign(Name("gui.clip_rect"), Any(effective_clip));
            }

            if(node.kind == GUINodeKind::table_layout)
            {
                arrange_table_node(node_index, rect, effective_clip);
                return rect;
            }

            if(node.first_child == U32_MAX) return rect;

            bool horizontal = node.kind == GUINodeKind::h_layout;
            const GUIEdgeInsets& padding = node.layout_desc.padding;
            RectF content_rect(
                rect.offset_x + padding.left,
                rect.offset_y + padding.top,
                max(rect.width - padding.left - padding.right, 0.0f),
                max(rect.height - padding.top - padding.bottom, 0.0f));
            if(node.kind == GUINodeKind::scroll_view)
            {
                PersistentItemState& persistent = get_or_create_persistent_state(node.id);
                content_rect.offset_y -= persistent.scroll_y;
            }

            Vector<u32> children;
            Vector<GUILayoutMetrics> child_metrics;
            Vector<f32> main_sizes;
            f32 total_base_main = 0.0f;
            f32 total_fill_weight = 0.0f;
            f32 total_shrink_capacity = 0.0f;
            for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
            {
                GUILayoutMetrics metrics = measure_node(child);
                GUINode& child_node = m_submitted_desc.nodes[child];
                f32 base_main = resolve_base_axis_size(child_node, metrics, horizontal);
                f32 min_main = axis_value(metrics.min_size, horizontal);
                base_main = max(base_main, min_main);
                children.push_back(child);
                child_metrics.push_back(metrics);
                main_sizes.push_back(base_main);
                total_base_main += base_main;
                total_shrink_capacity += max(base_main - min_main, 0.0f);
                if(axis_policy(child_node.layout_style, horizontal) == GUISizePolicy::fill)
                {
                    total_fill_weight += max(axis_fill_weight(child_node.layout_style, horizontal), 0.0f);
                }
            }

            if(children.empty()) return rect;

            f32 gap = node.layout_desc.gap;
            f32 total_gap = gap * (f32)(children.size() - 1);
            f32 available_main = horizontal ? content_rect.width : content_rect.height;
            if(node.kind == GUINodeKind::scroll_view)
            {
                available_main = max(available_main, total_base_main + total_gap);
            }
            f32 remaining = available_main - total_base_main - total_gap;
            if(remaining > 0.0f && total_fill_weight > 0.0f)
            {
                for(usize i = 0; i < children.size(); ++i)
                {
                    GUINode& child_node = m_submitted_desc.nodes[children[i]];
                    if(axis_policy(child_node.layout_style, horizontal) != GUISizePolicy::fill) continue;
                    f32 weight = max(axis_fill_weight(child_node.layout_style, horizontal), 0.0f);
                    f32 max_main = axis_value(child_metrics[i].max_size, horizontal);
                    main_sizes[i] = min(main_sizes[i] + remaining * (weight / total_fill_weight), max_main);
                }
            }
            else if(remaining < 0.0f && total_shrink_capacity > 0.0f)
            {
                f32 deficit = -remaining;
                for(usize i = 0; i < children.size(); ++i)
                {
                    f32 min_main = axis_value(child_metrics[i].min_size, horizontal);
                    f32 capacity = max(main_sizes[i] - min_main, 0.0f);
                    main_sizes[i] -= min(capacity, deficit * (capacity / total_shrink_capacity));
                }
            }

            f32 used_main = total_gap;
            for(f32 size : main_sizes)
            {
                used_main += size;
            }
            f32 free_main = max(available_main - used_main, 0.0f);
            f32 main_offset = 0.0f;
            if(node.layout_desc.main_axis_alignment == GUILayoutMainAxisAlignment::center)
            {
                main_offset = free_main * 0.5f;
            }
            else if(node.layout_desc.main_axis_alignment == GUILayoutMainAxisAlignment::end)
            {
                main_offset = free_main;
            }
            else if(node.layout_desc.main_axis_alignment == GUILayoutMainAxisAlignment::space_between && children.size() > 1)
            {
                gap += free_main / (f32)(children.size() - 1);
                main_offset = 0.0f;
            }

            f32 main_cursor = (horizontal ? content_rect.offset_x : content_rect.offset_y) + main_offset;
            f32 cross_start = horizontal ? content_rect.offset_y : content_rect.offset_x;
            f32 available_cross = horizontal ? content_rect.height : content_rect.width;
            for(usize i = 0; i < children.size(); ++i)
            {
                GUINode& child_node = m_submitted_desc.nodes[children[i]];
                bool cross_x_axis = !horizontal;
                f32 cross_size;
                if(node.layout_desc.cross_axis_alignment == GUILayoutCrossAxisAlignment::stretch &&
                    axis_policy(child_node.layout_style, cross_x_axis) != GUISizePolicy::fixed)
                {
                    cross_size = available_cross;
                }
                else
                {
                    cross_size = resolve_base_axis_size(child_node, child_metrics[i], cross_x_axis);
                }
                cross_size = clamp(cross_size,
                    axis_value(child_metrics[i].min_size, cross_x_axis),
                    axis_value(child_metrics[i].max_size, cross_x_axis));
                f32 cross_offset = 0.0f;
                if(node.layout_desc.cross_axis_alignment == GUILayoutCrossAxisAlignment::center)
                {
                    cross_offset = max(available_cross - cross_size, 0.0f) * 0.5f;
                }
                else if(node.layout_desc.cross_axis_alignment == GUILayoutCrossAxisAlignment::end)
                {
                    cross_offset = max(available_cross - cross_size, 0.0f);
                }
                RectF child_rect;
                if(horizontal)
                {
                    child_rect = RectF(main_cursor, cross_start + cross_offset, main_sizes[i], cross_size);
                }
                else
                {
                    child_rect = RectF(cross_start + cross_offset, main_cursor, cross_size, main_sizes[i]);
                }
                layout_node(children[i], child_rect, effective_clip);
                main_cursor += main_sizes[i] + gap;
            }
            return rect;
        }
    }
}
