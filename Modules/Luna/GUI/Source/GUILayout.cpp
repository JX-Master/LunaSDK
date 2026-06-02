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
        static u32 grid_child_count(const Description& desc, const Node& node)
        {
            u32 count = 0;
            for(u32 child = node.first_child; child != U32_MAX; child = desc.nodes[child].next_sibling)
            {
                if(absolute_node(desc.nodes[child])) continue;
                ++count;
            }
            return count;
        }

        static u32 grid_row_count(u32 child_count, u32 columns)
        {
            columns = max(columns, 1u);
            return child_count ? (child_count + columns - 1) / columns : 1;
        }

        static u32 grid_columns_for_width(const GridLayoutDesc& desc, f32 content_width, u32 child_count)
        {
            if(desc.sizing_mode == GridSizingMode::fixed_columns)
            {
                return max(desc.columns, 1u);
            }
            f32 cell_width = max(desc.cell_size.x, 1.0f);
            f32 gap = max(desc.gap.x, 0.0f);
            f32 available = max(content_width, cell_width);
            u32 columns = (u32)((available + gap) / max(cell_width + gap, 1.0f));
            columns = max(columns, 1u);
            return child_count ? min(columns, child_count) : columns;
        }

        struct ContextNodeMeasureContext : NodeMeasureContext
        {
            Context* context = nullptr;
            u32 node_index = U32_MAX;

            virtual const Node* parent() const override
            {
                if(!context || node_index >= context->m_submitted_desc.nodes.size()) return nullptr;
                const Node& node = context->m_submitted_desc.nodes[node_index];
                if(node.parent == U32_MAX || node.parent >= context->m_submitted_desc.nodes.size()) return nullptr;
                return &context->m_submitted_desc.nodes[node.parent];
            }

            virtual Float2U surface_size() const override
            {
                return context ? context->m_frame_desc.surface_size : Float2U(0.0f);
            }

            virtual LayoutMetrics measure_text(const c8* text, usize text_size, f32 font_size, f32 max_width) const override
            {
                f32 w = max((f32)text_size * font_size * 0.52f, 1.0f);
                f32 h = font_size + 4.0f;
                if(max_width < F32_MAX * 0.5f)
                {
                    VG::TextArrangeSection section;
                    section.font_file = Font::get_default_font();
                    section.font_index = 0;
                    section.font_size = font_size;
                    section.num_chars = text_size;
                    auto arranged = VG::arrange_text(text, text_size, {&section, 1},
                        RectF(0.0f, 0.0f, max_width, 100000.0f),
                        VG::TextAlignment::begin, VG::TextAlignment::begin);
                    w = max(arranged.bounding_rect.width, 1.0f);
                    h = max(arranged.bounding_rect.height, h);
                }
                LayoutMetrics metrics;
                metrics.min_size = Float2U(min(w, 32.0f), h);
                metrics.preferred_size = Float2U(min(w, max_width), h);
                metrics.max_size = Float2U(max_width, h);
                return metrics;
            }
        };

        void Context::measure_table_tracks(u32 node_index, Vector<f32>& out_column_widths, Vector<f32>& out_row_heights, bool preferred)
        {
            const Node& node = m_submitted_desc.nodes[node_index];
            u32 columns = table_columns(node);
            u32 rows = table_rows(m_submitted_desc, node);
            out_column_widths.assign(columns, 1.0f);
            out_row_heights.assign(rows, 1.0f);

            TableLayoutState* persistent = get_widget_state<TableLayoutState>(node.id);
            if(persistent)
            {
                touch_widget_state<TableLayoutState>(node.id);
            }
            for(u32 col = 0; col < columns; ++col)
            {
                const TableTrackSize& size = table_track_size(node, true, col);
                if(size.policy == TableTrackSizePolicy::fixed)
                {
                    f32 value = size.value;
                    if(persistent && col < persistent->table_column_sizes.size() && persistent->table_column_sizes[col] > 0.0f)
                    {
                        value = persistent->table_column_sizes[col];
                    }
                    out_column_widths[col] = max(value, 1.0f);
                }
            }
            for(u32 row = 0; row < rows; ++row)
            {
                const TableTrackSize& size = table_track_size(node, false, row);
                if(size.policy == TableTrackSizePolicy::fixed)
                {
                    f32 value = size.value;
                    if(persistent && row < persistent->table_row_sizes.size() && persistent->table_row_sizes[row] > 0.0f)
                    {
                        value = persistent->table_row_sizes[row];
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
                LayoutMetrics child_metrics = measure_node(child);
                Float2U child_size = preferred ? child_metrics.preferred_size : child_metrics.min_size;
                const TableStyle& style = table_desc(node).style;
                f32 cell_width = child_size.x + style.padding.left + style.padding.right;
                f32 cell_height = child_size.y + style.padding.top + style.padding.bottom;
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

        LayoutMetrics Context::measure_grid_node(u32 node_index, f32 available_width)
        {
            const Node& node = m_submitted_desc.nodes[node_index];
            const GridLayoutDesc& desc = grid_desc(node);
            f32 padding_x = desc.padding.left + desc.padding.right;
            f32 padding_y = desc.padding.top + desc.padding.bottom;
            f32 gap_x = max(desc.gap.x, 0.0f);
            f32 gap_y = max(desc.gap.y, 0.0f);
            u32 child_count = grid_child_count(m_submitted_desc, node);
            LayoutMetrics metrics;

            if(desc.sizing_mode == GridSizingMode::fixed_cell_size)
            {
                f32 cell_width = max(desc.cell_size.x, 1.0f);
                f32 cell_height = max(desc.cell_size.y, 1.0f);
                bool constrained = available_width < F32_MAX * 0.5f;
                f32 content_width = constrained ? max(available_width - padding_x, cell_width) : cell_width * (f32)max(child_count, 1u) + gap_x * (f32)max((i32)child_count - 1, 0);
                u32 columns = grid_columns_for_width(desc, content_width, child_count);
                u32 rows = grid_row_count(child_count, columns);
                f32 width = padding_x + cell_width * (f32)columns + gap_x * (f32)max((i32)columns - 1, 0);
                f32 height = padding_y + cell_height * (f32)rows + gap_y * (f32)max((i32)rows - 1, 0);
                metrics.min_size = Float2U(padding_x + cell_width, padding_y + cell_height);
                metrics.preferred_size = Float2U(max(width, 1.0f), max(height, 1.0f));
                metrics.max_size = Float2U(F32_MAX, F32_MAX);
                return metrics;
            }

            u32 columns = max(desc.columns, 1u);
            u32 rows = grid_row_count(child_count, columns);
            Vector<f32> min_row_heights;
            Vector<f32> preferred_row_heights;
            min_row_heights.assign(rows, 1.0f);
            preferred_row_heights.assign(rows, 1.0f);
            f32 min_cell_width = 1.0f;
            f32 preferred_cell_width = 1.0f;
            u32 cell_index = 0;
            for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
            {
                const Node& child_node = m_submitted_desc.nodes[child];
                if(absolute_node(child_node)) continue;
                LayoutMetrics child_metrics = measure_node(child);
                u32 row = cell_index / columns;
                min_cell_width = max(min_cell_width, child_metrics.min_size.x);
                preferred_cell_width = max(preferred_cell_width, child_metrics.preferred_size.x);
                min_row_heights[row] = max(min_row_heights[row], child_metrics.min_size.y);
                preferred_row_heights[row] = max(preferred_row_heights[row], child_metrics.preferred_size.y);
                ++cell_index;
            }

            f32 min_height = padding_y + gap_y * (f32)max((i32)rows - 1, 0);
            f32 preferred_height = min_height;
            for(f32 h : min_row_heights) min_height += h;
            for(f32 h : preferred_row_heights) preferred_height += h;
            f32 min_width = padding_x + min_cell_width * (f32)columns + gap_x * (f32)max((i32)columns - 1, 0);
            f32 preferred_width = padding_x + preferred_cell_width * (f32)columns + gap_x * (f32)max((i32)columns - 1, 0);
            metrics.min_size = Float2U(max(min_width, 1.0f), max(min_height, 1.0f));
            metrics.preferred_size = Float2U(max(preferred_width, 1.0f), max(preferred_height, 1.0f));
            metrics.max_size = Float2U(F32_MAX, F32_MAX);
            return metrics;
        }

        LayoutMetrics Context::measure_node(u32 node_index)
        {
            if(m_layouts[node_index].metrics_valid)
            {
                return m_layouts[node_index].metrics;
            }

            const Node& node = m_submitted_desc.nodes[node_index];
            LayoutMetrics metrics;
            f32 font_size = 16.0f;
            f32 text_width = (f32)node.text.size() * font_size * 0.52f;
            if(node.uses_node_measure())
            {
                ContextNodeMeasureContext node_measure_context;
                node_measure_context.context = this;
                node_measure_context.node_index = node_index;
                metrics = node.measure(node_measure_context);
                metrics = apply_layout_style(node, metrics);
                m_layouts[node_index].metrics = metrics;
                m_layouts[node_index].metrics_valid = true;
                return metrics;
            }

            if(table_layout(node))
            {
                Vector<f32> min_columns;
                Vector<f32> min_rows;
                Vector<f32> preferred_columns;
                Vector<f32> preferred_rows;
                measure_table_tracks(node_index, min_columns, min_rows, false);
                measure_table_tracks(node_index, preferred_columns, preferred_rows, true);
                const TableStyle& style = table_desc(node).style;
                f32 min_width = style.border_size * 2.0f;
                f32 preferred_width = style.border_size * 2.0f;
                for(f32 v : min_columns) min_width += v;
                for(f32 v : preferred_columns) preferred_width += v;
                f32 min_height = style.border_size * 2.0f;
                f32 preferred_height = style.border_size * 2.0f;
                for(f32 v : min_rows) min_height += v;
                for(f32 v : preferred_rows) preferred_height += v;
                f32 separator_size = style.separator_size;
                if(style.column_separators && min_columns.size() > 1)
                {
                    f32 separators = separator_size * (f32)(min_columns.size() - 1);
                    min_width += separators;
                    preferred_width += separators;
                }
                if(style.row_separators && min_rows.size() > 1)
                {
                    f32 separators = separator_size * (f32)(min_rows.size() - 1);
                    min_height += separators;
                    preferred_height += separators;
                }
                metrics.min_size = Float2U(max(min_width, 1.0f), max(min_height, 1.0f));
                metrics.preferred_size = Float2U(max(preferred_width, 1.0f), max(preferred_height, 1.0f));
                metrics.max_size = Float2U(F32_MAX, F32_MAX);
            }
            else if(grid_layout(node))
            {
                metrics = measure_grid_node(node_index, F32_MAX);
            }
            else if(canvas_layout(node))
            {
                f32 min_width = 1.0f;
                f32 min_height = 1.0f;
                f32 preferred_width = node.requested_size.width > 0.0f ? node.requested_size.width : 1.0f;
                f32 preferred_height = node.requested_size.height > 0.0f ? node.requested_size.height : 1.0f;
                for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
                {
                    const Node& child_node = m_submitted_desc.nodes[child];
                    if(absolute_node(child_node) && !child_node.has_canvas_item_layout) continue;
                    LayoutMetrics child_metrics = measure_node(child);
                    min_width = max(min_width, child_metrics.min_size.x);
                    min_height = max(min_height, child_metrics.min_size.y);
                    preferred_width = max(preferred_width, child_metrics.preferred_size.x);
                    preferred_height = max(preferred_height, child_metrics.preferred_size.y);
                }
                metrics.min_size = Float2U(min_width, min_height);
                metrics.preferred_size = Float2U(preferred_width, preferred_height);
                metrics.max_size = Float2U(F32_MAX, F32_MAX);
            }
            else if(tab_bar_layout(node))
            {
                f32 min_header_width = 0.0f;
                f32 preferred_header_width = 0.0f;
                LayoutMetrics content_metrics;
                bool has_content = false;
                for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
                {
                    const Node& child_node = m_submitted_desc.nodes[child];
                    if(!tab_item_layout(child_node)) continue;
                    if(!bool_value_open(child_node)) continue;
                    f32 ideal_width = tab_item_ideal_width(child_node);
                    min_header_width += tab_item_min_width();
                    preferred_header_width += ideal_width;
                    TabBarState* tab_state = get_widget_state<TabBarState>(node.id);
                    if(child_node.tab_item_selected() || (tab_state && tab_state->tab_selected_id == child_node.id))
                    {
                        content_metrics = measure_node(child);
                        has_content = true;
                    }
                }
                f32 header_height = tab_bar_header_height();
                if(!has_content)
                {
                    content_metrics.min_size = Float2U(1.0f, 1.0f);
                    content_metrics.preferred_size = Float2U(1.0f, 1.0f);
                    content_metrics.max_size = Float2U(F32_MAX, F32_MAX);
                }
                metrics.min_size = Float2U(max(min_header_width, 1.0f), header_height + content_metrics.min_size.y);
                metrics.preferred_size = Float2U(max(preferred_header_width, content_metrics.preferred_size.x), header_height + content_metrics.preferred_size.y);
                metrics.max_size = Float2U(F32_MAX, F32_MAX);
            }
            else
            {
                bool horizontal = horizontal_layout(node);
                const EdgeInsets& padding = node.layout_desc.padding;
                f32 gap = node.layout_desc.gap;
                f32 min_main = 0.0f;
                f32 preferred_main = 0.0f;
                f32 min_cross = 0.0f;
                f32 preferred_cross = 0.0f;
                u32 child_count = 0;
                for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
                {
                    const Node& child_node = m_submitted_desc.nodes[child];
                    if(absolute_node(child_node)) continue;
                    LayoutMetrics child_metrics = measure_node(child);
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
                if(window_has_title_bar(node))
                {
                    f32 title_width = max(text_width + 48.0f, 96.0f);
                    min_size.x = max(min_size.x, title_width);
                    preferred_size.x = max(preferred_size.x, title_width);
                    min_size.y += window_title_bar_height();
                    preferred_size.y += window_title_bar_height();
                }
                metrics.min_size = min_size;
                metrics.preferred_size = preferred_size;
                metrics.max_size = Float2U(F32_MAX, F32_MAX);
            }

            metrics = apply_layout_style(node, metrics);
            m_layouts[node_index].metrics = metrics;
            m_layouts[node_index].metrics_valid = true;
            return metrics;
        }

        void Context::arrange_grid_node(u32 node_index, const RectF& rect, const RectF& clip_rect)
        {
            Node& node = m_submitted_desc.nodes[node_index];
            const GridLayoutDesc& desc = grid_desc(node);
            f32 gap_x = max(desc.gap.x, 0.0f);
            f32 gap_y = max(desc.gap.y, 0.0f);
            RectF content_rect(
                rect.offset_x + desc.padding.left,
                rect.offset_y + desc.padding.top,
                max(rect.width - desc.padding.left - desc.padding.right, 1.0f),
                max(rect.height - desc.padding.top - desc.padding.bottom, 1.0f));

            Vector<u32> children;
            Vector<u32> absolute_children;
            children.reserve(16);
            absolute_children.reserve(8);
            for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
            {
                if(absolute_node(m_submitted_desc.nodes[child]))
                {
                    absolute_children.push_back(child);
                    continue;
                }
                children.push_back(child);
            }
            u32 child_count = (u32)children.size();
            u32 columns = grid_columns_for_width(desc, content_rect.width, child_count);
            u32 rows = grid_row_count(child_count, columns);
            if(!columns || !rows) return;

            f32 cell_width = 1.0f;
            Vector<f32> row_heights;
            row_heights.assign(rows, 1.0f);
            if(desc.sizing_mode == GridSizingMode::fixed_cell_size)
            {
                cell_width = max(desc.cell_size.x, 1.0f);
                for(f32& row_height : row_heights)
                {
                    row_height = max(desc.cell_size.y, 1.0f);
                }
            }
            else
            {
                cell_width = max((content_rect.width - gap_x * (f32)max((i32)columns - 1, 0)) / (f32)columns, 1.0f);
                for(usize i = 0; i < children.size(); ++i)
                {
                    u32 row = (u32)i / columns;
                    Node& child_node = m_submitted_desc.nodes[children[i]];
                    LayoutMetrics child_metrics = measure_node(children[i]);
                    f32 child_height = resolve_base_axis_size(child_node, child_metrics, false);
                    child_height = clamp(child_height, child_metrics.min_size.y, child_metrics.max_size.y);
                    row_heights[row] = max(row_heights[row], child_height);
                }
            }

            f32 y = content_rect.offset_y;
            usize child_index = 0;
            for(u32 row = 0; row < rows && child_index < children.size(); ++row)
            {
                f32 x = content_rect.offset_x;
                f32 row_height = row_heights[row];
                for(u32 column = 0; column < columns && child_index < children.size(); ++column, ++child_index)
                {
                    RectF cell_rect(x, y, cell_width, row_height);
                    layout_node(children[child_index], cell_rect, intersect_rect(cell_rect, clip_rect));
                    x += cell_width + gap_x;
                }
                y += row_height + gap_y;
            }

            RectF surface_clip(0.0f, 0.0f, m_frame_desc.surface_size.x, m_frame_desc.surface_size.y);
            for(u32 child : absolute_children)
            {
                Node& child_node = m_submitted_desc.nodes[child];
                LayoutMetrics metrics = measure_node(child);
                f32 width = max(resolve_base_axis_size(child_node, metrics, true), 1.0f);
                f32 height = max(resolve_base_axis_size(child_node, metrics, false), 1.0f);
                layout_node(child, RectF(child_node.position.x, child_node.position.y, width, height), surface_clip);
            }
        }

        void Context::arrange_canvas_node(u32 node_index, const RectF& rect, const RectF& clip_rect)
        {
            Node& node = m_submitted_desc.nodes[node_index];
            const CanvasLayoutDesc& desc = canvas_desc(node);
            RectF content_rect(
                rect.offset_x + desc.padding.left,
                rect.offset_y + desc.padding.top,
                max(rect.width - desc.padding.left - desc.padding.right, 1.0f),
                max(rect.height - desc.padding.top - desc.padding.bottom, 1.0f));
            RectF child_clip = desc.clip_children ? intersect_rect(content_rect, clip_rect) :
                RectF(0.0f, 0.0f, m_frame_desc.surface_size.x, m_frame_desc.surface_size.y);

            for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
            {
                Node& child_node = m_submitted_desc.nodes[child];
                LayoutMetrics metrics = measure_node(child);
                RectF child_rect;
                if(child_node.has_canvas_item_layout)
                {
                    const CanvasItemLayout& item = child_node.canvas_item_layout;
                    f32 anchor_min_x = clamp(item.anchor_min.x, 0.0f, 1.0f);
                    f32 anchor_min_y = clamp(item.anchor_min.y, 0.0f, 1.0f);
                    f32 anchor_max_x = clamp(item.anchor_max.x, 0.0f, 1.0f);
                    f32 anchor_max_y = clamp(item.anchor_max.y, 0.0f, 1.0f);
                    if(anchor_max_x < anchor_min_x) swap(anchor_min_x, anchor_max_x);
                    if(anchor_max_y < anchor_min_y) swap(anchor_min_y, anchor_max_y);
                    f32 left = content_rect.offset_x + content_rect.width * anchor_min_x + item.offset_min.x;
                    f32 top = content_rect.offset_y + content_rect.height * anchor_min_y + item.offset_min.y;
                    f32 right = content_rect.offset_x + content_rect.width * anchor_max_x + item.offset_max.x;
                    f32 bottom = content_rect.offset_y + content_rect.height * anchor_max_y + item.offset_max.y;
                    child_rect = RectF(left, top, max(right - left, 1.0f), max(bottom - top, 1.0f));
                }
                else if(absolute_node(child_node))
                {
                    f32 width = max(resolve_base_axis_size(child_node, metrics, true), 1.0f);
                    f32 height = max(resolve_base_axis_size(child_node, metrics, false), 1.0f);
                    child_rect = RectF(child_node.position.x, child_node.position.y, width, height);
                }
                else
                {
                    f32 width = max(resolve_base_axis_size(child_node, metrics, true), 1.0f);
                    f32 height = max(resolve_base_axis_size(child_node, metrics, false), 1.0f);
                    child_rect = RectF(content_rect.offset_x, content_rect.offset_y, width, height);
                }
                layout_node(child, child_rect, desc.clip_children ? intersect_rect(child_rect, child_clip) : child_clip);
            }
        }

        void Context::arrange_tab_bar_node(u32 node_index, const RectF& rect, const RectF& clip_rect)
        {
            Node& node = m_submitted_desc.nodes[node_index];
            Ref<TabBarState> state = get_or_create_widget_state<TabBarState>(node.id);
            Vector<u32> live_tabs;
            live_tabs.reserve(8);
            id_t selected = state->tab_selected_id;
            id_t first_open = 0;
            bool selected_open = false;

            for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
            {
                Node& child_node = m_submitted_desc.nodes[child];
                if(!tab_item_layout(child_node)) continue;
                bool open = bool_value_open(child_node);
                if(!open)
                {
                    m_layouts[child].tab_content_visible = false;
                    m_layouts[child].rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
                    m_layouts[child].clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
                    continue;
                }
                live_tabs.push_back(child);
                if(!first_open && !test_flags(child_node.get_tab_item_flags(), TabItemFlag::button))
                {
                    first_open = child_node.id;
                }
                // tab_item_selected is build-time visibility; persistent selection is authoritative
                // after input-triggered relayouts.
                if(!selected && child_node.tab_item_selected() && !test_flags(child_node.get_tab_item_flags(), TabItemFlag::button))
                {
                    selected = child_node.id;
                }
                if(selected && selected == child_node.id)
                {
                    selected_open = !test_flags(child_node.get_tab_item_flags(), TabItemFlag::button);
                }
            }
            if(!selected || !selected_open)
            {
                selected = first_open;
            }
            state->tab_selected_id = selected;

            auto live_tab_index = [&](id_t id) -> u32 {
                for(u32 tab : live_tabs)
                {
                    if(m_submitted_desc.nodes[tab].id == id) return tab;
                }
                return U32_MAX;
            };
            for(usize i = 0; i < state->tab_order.size();)
            {
                if(live_tab_index(state->tab_order[i]) == U32_MAX)
                {
                    state->tab_order.erase(state->tab_order.begin() + i);
                }
                else
                {
                    ++i;
                }
            }
            for(u32 tab : live_tabs)
            {
                id_t id = m_submitted_desc.nodes[tab].id;
                if(!tab_order_contains(*state, id))
                {
                    state->tab_order.push_back(id);
                }
            }

            Vector<u32> tabs;
            tabs.reserve(live_tabs.size());
            for(id_t id : state->tab_order)
            {
                u32 tab = live_tab_index(id);
                if(tab != U32_MAX) tabs.push_back(tab);
            }

            RectF header_rect(rect.offset_x, rect.offset_y, rect.width, min(tab_bar_header_height(), max(rect.height, 1.0f)));
            RectF header_area_rect(header_rect.offset_x + 4.0f, header_rect.offset_y, max(header_rect.width - 8.0f, 1.0f), header_rect.height);
            RectF content_rect(
                rect.offset_x,
                rect.offset_y + header_rect.height,
                rect.width,
                max(rect.height - header_rect.height, 1.0f));
            RectF content_clip = intersect_rect(content_rect, clip_rect);

            NodeLayout& bar_layout = m_layouts[node_index];
            bar_layout.tab_scrollable = false;
            bar_layout.tab_scroll_max = 0.0f;
            bar_layout.tab_scroll_left_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            bar_layout.tab_scroll_right_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);

            Vector<f32> widths;
            widths.reserve(tabs.size());
            f32 total_width = 0.0f;
            f32 shrink_capacity = 0.0f;
            for(u32 tab : tabs)
            {
                f32 width = tab_item_ideal_width(m_submitted_desc.nodes[tab]);
                widths.push_back(width);
                total_width += width;
                shrink_capacity += max(width - tab_item_min_width(), 0.0f);
            }
            bool use_scroll = test_flags(node.get_tab_bar_flags(), TabBarFlag::fitting_scroll);
            if(use_scroll && total_width > header_area_rect.width + 0.5f)
            {
                f32 button_size = tab_scroll_button_size();
                bar_layout.tab_scrollable = true;
                bar_layout.tab_scroll_left_rect = RectF(
                    header_rect.offset_x + 4.0f,
                    header_rect.offset_y + 4.0f,
                    button_size,
                    max(header_rect.height - 8.0f, 1.0f));
                bar_layout.tab_scroll_right_rect = RectF(
                    header_rect.offset_x + max(header_rect.width - button_size - 4.0f, 0.0f),
                    header_rect.offset_y + 4.0f,
                    button_size,
                    max(header_rect.height - 8.0f, 1.0f));
                header_area_rect = RectF(
                    bar_layout.tab_scroll_left_rect.offset_x + button_size + 4.0f,
                    header_rect.offset_y,
                    max(bar_layout.tab_scroll_right_rect.offset_x - (bar_layout.tab_scroll_left_rect.offset_x + button_size + 4.0f) - 4.0f, 1.0f),
                    header_rect.height);
            }
            f32 available_width = max(header_area_rect.width, 1.0f);
            if(!bar_layout.tab_scrollable && test_flags(node.get_tab_bar_flags(), TabBarFlag::fitting_shrink) &&
                total_width > available_width && shrink_capacity > 0.0f)
            {
                f32 deficit = total_width - available_width;
                for(usize i = 0; i < widths.size(); ++i)
                {
                    f32 capacity = max(widths[i] - tab_item_min_width(), 0.0f);
                    widths[i] -= min(capacity, deficit * (capacity / shrink_capacity));
                }
                total_width = 0.0f;
                for(f32 width : widths) total_width += width;
            }

            bar_layout.tab_header_area_rect = header_area_rect;
            bar_layout.tab_scroll_max = max(total_width - available_width, 0.0f);
            state->tab_scroll_x = bar_layout.tab_scrollable ? clamp(state->tab_scroll_x, 0.0f, bar_layout.tab_scroll_max) : 0.0f;

            RectF header_clip = intersect_rect(header_area_rect, clip_rect);
            f32 cursor_x = header_area_rect.offset_x - state->tab_scroll_x;
            for(usize i = 0; i < tabs.size(); ++i)
            {
                u32 tab = tabs[i];
                Node& tab_node = m_submitted_desc.nodes[tab];
                NodeLayout& tab_layout = m_layouts[tab];
                bool content_visible = tab_node.id == selected && !test_flags(tab_node.get_tab_item_flags(), TabItemFlag::button);
                RectF tab_header(cursor_x, header_rect.offset_y + 3.0f, max(widths[i], 1.0f), max(header_rect.height - 4.0f, 1.0f));
                tab_layout.tab_header_rect = tab_header;
                tab_layout.tab_header_clip_rect = header_clip;
                tab_layout.tab_content_visible = content_visible;
                tab_layout.tab_close_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
                if(tab_node.bool_value() && !test_flags(tab_node.get_tab_item_flags(), TabItemFlag::no_close_button))
                {
                    f32 close_size = 18.0f;
                    tab_layout.tab_close_rect = RectF(
                        tab_header.offset_x + max(tab_header.width - close_size - 4.0f, 0.0f),
                        tab_header.offset_y + max((tab_header.height - close_size) * 0.5f, 0.0f),
                        close_size,
                        close_size);
                }
                cursor_x += widths[i];
                if(content_visible)
                {
                    layout_node(tab, content_rect, content_clip);
                    tab_layout.tab_header_rect = tab_header;
                    tab_layout.tab_header_clip_rect = header_clip;
                    tab_layout.tab_close_rect = tab_layout.tab_close_rect.width > 0.0f ? tab_layout.tab_close_rect : RectF(0.0f, 0.0f, 0.0f, 0.0f);
                    tab_layout.tab_content_visible = true;
                }
                else
                {
                    tab_layout.rect = content_rect;
                    tab_layout.clip_rect = content_clip;
                    tab_layout.metrics = measure_node(tab);
                    tab_layout.metrics_valid = true;
                    Ref<ItemQueryState> result = get_or_create_query_state(tab_node.id);
                    result->states.insert_or_assign(Name("gui.rect"), Any(tab_header));
                    result->states.insert_or_assign(Name("gui.clip_rect"), Any(header_clip));
                }
                Ref<ItemQueryState> result = get_or_create_query_state(tab_node.id);
                result->states.insert_or_assign(Name("gui.rect"), Any(tab_header));
                result->states.insert_or_assign(Name("gui.clip_rect"), Any(header_clip));
                result->states.insert_or_assign(Name("gui.open"), Any(true));
            }
        }

        void Context::arrange_table_node(u32 node_index, const RectF& rect, const RectF& clip_rect)
        {
            Node& node = m_submitted_desc.nodes[node_index];
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

            const TableStyle& style = table_desc(node).style;
            f32 border_size = style.border_size;
            f32 separator_size = style.separator_size;
            f32 column_separator = style.column_separators ? separator_size : 0.0f;
            f32 row_separator = style.row_separators ? separator_size : 0.0f;
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
                    cell_rect.offset_x + style.padding.left,
                    cell_rect.offset_y + style.padding.top,
                    max(cell_rect.width - style.padding.left - style.padding.right, 1.0f),
                    max(cell_rect.height - style.padding.top - style.padding.bottom, 1.0f));
                layout_node(child, child_rect, intersect_rect(cell_rect, clip_rect));
            }
        }

        static void normalize_dock_panel_heights(Vector<f32>& heights, const Vector<f32>& min_heights, f32 available_height)
        {
            if(heights.empty()) return;
            available_height = max(available_height, 1.0f);
            f32 min_sum = 0.0f;
            for(f32 min_height : min_heights)
            {
                min_sum += max(min_height, 1.0f);
            }
            if(min_sum >= available_height)
            {
                f32 cursor = 0.0f;
                for(usize i = 0; i < heights.size(); ++i)
                {
                    f32 h = (i + 1 == heights.size()) ? max(available_height - cursor, 1.0f) :
                        max(available_height * max(min_heights[i], 1.0f) / min_sum, 1.0f);
                    heights[i] = h;
                    cursor += h;
                }
                return;
            }

            for(usize i = 0; i < heights.size(); ++i)
            {
                heights[i] = max(heights[i], max(min_heights[i], 1.0f));
            }
            for(u32 pass = 0; pass < 4; ++pass)
            {
                f32 sum = 0.0f;
                for(f32 h : heights) sum += h;
                f32 delta = sum - available_height;
                if(delta >= -0.5f && delta <= 0.5f) break;
                if(delta > 0.0f)
                {
                    f32 shrinkable = 0.0f;
                    for(usize i = 0; i < heights.size(); ++i)
                    {
                        shrinkable += max(heights[i] - min_heights[i], 0.0f);
                    }
                    if(shrinkable <= 0.0f) break;
                    for(usize i = 0; i < heights.size(); ++i)
                    {
                        f32 share = max(heights[i] - min_heights[i], 0.0f) / shrinkable;
                        heights[i] = max(heights[i] - delta * share, min_heights[i]);
                    }
                }
                else
                {
                    f32 grow = -delta / (f32)heights.size();
                    for(f32& h : heights)
                    {
                        h += grow;
                    }
                }
            }
        }

        u32 Context::new_dock_leaf(DockSpaceState& dock_state, id_t panel_id, u32 parent)
        {
            DockTreeNode leaf;
            leaf.parent = parent;
            if(panel_id)
            {
                leaf.tabs.push_back(panel_id);
                leaf.selected_tab = panel_id;
            }
            u32 index = (u32)dock_state.dock_nodes.size();
            dock_state.dock_nodes.push_back(move(leaf));
            return index;
        }

        bool Context::dock_tree_contains_panel(const DockSpaceState& dock_state, id_t panel_id) const
        {
            if(dock_state.dock_root_node == U32_MAX || dock_state.dock_root_node >= dock_state.dock_nodes.size()) return false;
            Vector<u32> stack;
            stack.push_back(dock_state.dock_root_node);
            while(!stack.empty())
            {
                u32 node_index = stack.back();
                stack.pop_back();
                if(node_index >= dock_state.dock_nodes.size()) continue;
                const DockTreeNode& node = dock_state.dock_nodes[node_index];
                if(node.split)
                {
                    stack.push_back(node.child1);
                    stack.push_back(node.child0);
                    continue;
                }
                for(id_t tab : node.tabs)
                {
                    if(tab == panel_id) return true;
                }
            }
            return false;
        }

        void Context::dock_tree_add_panel(DockSpaceState& dock_state, id_t panel_id)
        {
            if(!panel_id || dock_tree_contains_panel(dock_state, panel_id)) return;
            if(dock_state.dock_root_node == U32_MAX || dock_state.dock_root_node >= dock_state.dock_nodes.size())
            {
                dock_state.dock_root_node = new_dock_leaf(dock_state, panel_id);
                return;
            }
            Vector<u32> stack;
            stack.push_back(dock_state.dock_root_node);
            while(!stack.empty())
            {
                u32 index = stack.back();
                stack.pop_back();
                if(index >= dock_state.dock_nodes.size()) continue;
                DockTreeNode& node = dock_state.dock_nodes[index];
                if(!node.split)
                {
                    node.tabs.push_back(panel_id);
                    node.selected_tab = panel_id;
                    return;
                }
                stack.push_back(node.child1);
                stack.push_back(node.child0);
            }
            dock_state.dock_root_node = new_dock_leaf(dock_state, panel_id);
        }

        static void dock_tree_replace_node_with_child(DockSpaceState& dock_state, u32 node_index, u32 child_index)
        {
            if(node_index >= dock_state.dock_nodes.size() || child_index >= dock_state.dock_nodes.size()) return;
            u32 parent = dock_state.dock_nodes[node_index].parent;
            dock_state.dock_nodes[node_index] = dock_state.dock_nodes[child_index];
            dock_state.dock_nodes[node_index].parent = parent;
            if(dock_state.dock_nodes[node_index].split)
            {
                if(dock_state.dock_nodes[node_index].child0 < dock_state.dock_nodes.size())
                {
                    dock_state.dock_nodes[dock_state.dock_nodes[node_index].child0].parent = node_index;
                }
                if(dock_state.dock_nodes[node_index].child1 < dock_state.dock_nodes.size())
                {
                    dock_state.dock_nodes[dock_state.dock_nodes[node_index].child1].parent = node_index;
                }
            }
        }

        static void dock_tree_remove_empty_leaf(DockSpaceState& dock_state, u32 leaf_index)
        {
            if(leaf_index >= dock_state.dock_nodes.size()) return;
            u32 parent = dock_state.dock_nodes[leaf_index].parent;
            if(parent == U32_MAX || parent >= dock_state.dock_nodes.size())
            {
                dock_state.dock_root_node = U32_MAX;
                return;
            }
            u32 sibling = dock_state.dock_nodes[parent].child0 == leaf_index ? dock_state.dock_nodes[parent].child1 : dock_state.dock_nodes[parent].child0;
            if(sibling >= dock_state.dock_nodes.size())
            {
                dock_state.dock_root_node = U32_MAX;
                return;
            }
            dock_tree_replace_node_with_child(dock_state, parent, sibling);
            if(dock_state.dock_root_node == leaf_index)
            {
                dock_state.dock_root_node = parent;
            }
        }

        bool Context::dock_tree_remove_panel(DockSpaceState& dock_state, id_t panel_id)
        {
            if(dock_state.dock_root_node == U32_MAX || dock_state.dock_root_node >= dock_state.dock_nodes.size()) return false;
            Vector<u32> stack;
            stack.push_back(dock_state.dock_root_node);
            while(!stack.empty())
            {
                u32 node_index = stack.back();
                stack.pop_back();
                if(node_index >= dock_state.dock_nodes.size()) continue;
                DockTreeNode& node = dock_state.dock_nodes[node_index];
                if(node.split)
                {
                    stack.push_back(node.child1);
                    stack.push_back(node.child0);
                    continue;
                }
                for(usize tab_index = 0; tab_index < node.tabs.size(); ++tab_index)
                {
                    if(node.tabs[tab_index] != panel_id) continue;
                    node.tabs.erase(node.tabs.begin() + tab_index);
                    if(node.selected_tab == panel_id)
                    {
                        node.selected_tab = node.tabs.empty() ? 0 : node.tabs[min(tab_index, node.tabs.size() - 1)];
                    }
                    if(node.tabs.empty())
                    {
                        dock_tree_remove_empty_leaf(dock_state, (u32)node_index);
                    }
                    return true;
                }
            }
            return false;
        }

        void Context::dock_tree_dock_panel(DockSpaceState& dock_state, id_t panel_id, u32 target_leaf, DockDropDirection direction)
        {
            if(!panel_id) return;
            dock_tree_remove_panel(dock_state, panel_id);
            if(direction == DockDropDirection::none) direction = DockDropDirection::center;
            if(dock_state.dock_root_node == U32_MAX || dock_state.dock_root_node >= dock_state.dock_nodes.size() || target_leaf >= dock_state.dock_nodes.size())
            {
                dock_state.dock_root_node = new_dock_leaf(dock_state, panel_id);
                return;
            }
            if(direction == DockDropDirection::center || dock_state.dock_nodes[target_leaf].split)
            {
                DockTreeNode& leaf = dock_state.dock_nodes[target_leaf];
                leaf.tabs.push_back(panel_id);
                leaf.selected_tab = panel_id;
                return;
            }

            DockTreeNode old_leaf = dock_state.dock_nodes[target_leaf];
            u32 old_child = (u32)dock_state.dock_nodes.size();
            old_leaf.parent = target_leaf;
            dock_state.dock_nodes.push_back(move(old_leaf));
            u32 new_child = new_dock_leaf(dock_state, panel_id, target_leaf);

            DockTreeNode split;
            split.split = true;
            split.parent = dock_state.dock_nodes[target_leaf].parent;
            split.split_axis = (direction == DockDropDirection::left || direction == DockDropDirection::right) ? DockSplitAxis::x : DockSplitAxis::y;
            split.split_ratio = 0.5f;
            if(direction == DockDropDirection::left || direction == DockDropDirection::up)
            {
                split.child0 = new_child;
                split.child1 = old_child;
            }
            else
            {
                split.child0 = old_child;
                split.child1 = new_child;
            }
            dock_state.dock_nodes[target_leaf] = move(split);
            dock_state.dock_nodes[old_child].parent = target_leaf;
            dock_state.dock_nodes[new_child].parent = target_leaf;
        }

        static bool dock_tree_prune_node(DockSpaceState& dock_state, u32 node_index, const HashSet<id_t, IdHash>& live_panels)
        {
            if(node_index >= dock_state.dock_nodes.size()) return false;
            DockTreeNode& node = dock_state.dock_nodes[node_index];
            if(!node.split)
            {
                for(usize i = 0; i < node.tabs.size();)
                {
                    if(live_panels.contains(node.tabs[i]))
                    {
                        ++i;
                    }
                    else
                    {
                        node.tabs.erase(node.tabs.begin() + i);
                    }
                }
                if(node.tabs.empty())
                {
                    node.selected_tab = 0;
                    return false;
                }
                bool selected_alive = false;
                for(id_t tab : node.tabs)
                {
                    if(tab == node.selected_tab)
                    {
                        selected_alive = true;
                        break;
                    }
                }
                if(!selected_alive)
                {
                    node.selected_tab = node.tabs[0];
                }
                return true;
            }

            bool child0_alive = dock_tree_prune_node(dock_state, node.child0, live_panels);
            bool child1_alive = dock_tree_prune_node(dock_state, node.child1, live_panels);
            if(child0_alive && child1_alive) return true;
            if(child0_alive || child1_alive)
            {
                dock_tree_replace_node_with_child(dock_state, node_index, child0_alive ? node.child0 : node.child1);
                return true;
            }
            return false;
        }

        void Context::dock_tree_prune_missing(DockSpaceState& dock_state, const HashSet<id_t, IdHash>& live_panels)
        {
            if(dock_state.dock_root_node == U32_MAX || dock_state.dock_root_node >= dock_state.dock_nodes.size()) return;
            if(!dock_tree_prune_node(dock_state, dock_state.dock_root_node, live_panels))
            {
                dock_state.dock_root_node = U32_MAX;
            }
        }

        id_t Context::dock_tree_selected_panel(DockSpaceState& dock_state, u32 leaf_index)
        {
            if(leaf_index >= dock_state.dock_nodes.size()) return 0;
            DockTreeNode& leaf = dock_state.dock_nodes[leaf_index];
            if(leaf.split || leaf.tabs.empty()) return 0;
            for(id_t tab : leaf.tabs)
            {
                if(tab == leaf.selected_tab) return tab;
            }
            leaf.selected_tab = leaf.tabs[0];
            return leaf.selected_tab;
        }

        void Context::arrange_dock_tree_node(id_t dock_space_id, u32 tree_node_index, const RectF& rect, const RectF& clip_rect, const HashMap<id_t, u32, IdHash>& panel_indices)
        {
            Ref<DockSpaceState> dock_state_ref = get_or_create_widget_state<DockSpaceState>(dock_space_id);
            DockSpaceState& dock_state = *dock_state_ref;
            if(tree_node_index >= dock_state.dock_nodes.size()) return;
            DockTreeNode& tree_node = dock_state.dock_nodes[tree_node_index];
            tree_node.rect = rect;
            if(tree_node.split)
            {
                f32 splitter_size = dock_panel_splitter_size();
                if(tree_node.split_axis == DockSplitAxis::x)
                {
                    f32 available_width = max(rect.width - splitter_size, 1.0f);
                    f32 child0_width = clamp(available_width * tree_node.split_ratio, 1.0f, max(available_width - 1.0f, 1.0f));
                    RectF child0_rect(rect.offset_x, rect.offset_y, child0_width, rect.height);
                    tree_node.split_rect = RectF(rect.offset_x + child0_width, rect.offset_y, splitter_size, rect.height);
                    RectF child1_rect(rect.offset_x + child0_width + splitter_size, rect.offset_y, max(available_width - child0_width, 1.0f), rect.height);
                    arrange_dock_tree_node(dock_space_id, tree_node.child0, child0_rect, clip_rect, panel_indices);
                    arrange_dock_tree_node(dock_space_id, tree_node.child1, child1_rect, clip_rect, panel_indices);
                }
                else
                {
                    f32 available_height = max(rect.height - splitter_size, 1.0f);
                    f32 child0_height = clamp(available_height * tree_node.split_ratio, 1.0f, max(available_height - 1.0f, 1.0f));
                    RectF child0_rect(rect.offset_x, rect.offset_y, rect.width, child0_height);
                    tree_node.split_rect = RectF(rect.offset_x, rect.offset_y + child0_height, rect.width, splitter_size);
                    RectF child1_rect(rect.offset_x, rect.offset_y + child0_height + splitter_size, rect.width, max(available_height - child0_height, 1.0f));
                    arrange_dock_tree_node(dock_space_id, tree_node.child0, child0_rect, clip_rect, panel_indices);
                    arrange_dock_tree_node(dock_space_id, tree_node.child1, child1_rect, clip_rect, panel_indices);
                }
                return;
            }

            id_t selected_panel = dock_tree_selected_panel(dock_state, tree_node_index);
            for(id_t panel_id : tree_node.tabs)
            {
                auto iter = panel_indices.find(panel_id);
                if(iter == panel_indices.end()) continue;
                u32 panel_index = iter->second;
                NodeLayout& panel_layout = m_layouts[panel_index];
                Node& panel_node = m_submitted_desc.nodes[panel_index];
                DockPanelStyle style = panel_layout.dock_panel_style;
                bool selected = panel_id == selected_panel;
                panel_layout.dock_panel_visible = selected;
                panel_layout.dock_leaf_index = tree_node_index;
                panel_layout.dock_panel_floating = false;
                panel_layout.dock_panel_rect = rect;
                panel_layout.dock_panel_clip_rect = intersect_rect(rect, clip_rect);
                panel_layout.dock_panel_title_rect = style.title_bar ? dock_panel_title_rect(rect, style) : RectF(0.0f, 0.0f, 0.0f, 0.0f);
                panel_layout.dock_panel_close_rect = style.title_bar && style.close_button ?
                    dock_panel_close_rect(panel_layout.dock_panel_title_rect) :
                    RectF(0.0f, 0.0f, 0.0f, 0.0f);
                panel_layout.dock_panel_resize_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
                if(selected)
                {
                    RectF content_rect = dock_panel_content_rect(rect, style);
                    layout_node(panel_index, content_rect, intersect_rect(rect, clip_rect));
                }
                else
                {
                    panel_layout.rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
                    panel_layout.clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
                }
            }
        }

        void Context::arrange_dock_space_node(u32 node_index, const RectF& rect, const RectF& clip_rect)
        {
            Node& node = m_submitted_desc.nodes[node_index];
            Ref<DockSpaceState> dock_state_ref = get_or_create_widget_state<DockSpaceState>(node.id);
            DockSpaceState& dock_state = *dock_state_ref;
            Vector<u32> floating_children;
            HashMap<id_t, u32, IdHash> docking_panel_indices;
            HashSet<id_t, IdHash> live_docking_panels;

            for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
            {
                Node& child_node = m_submitted_desc.nodes[child];
                DockPanelStyle style = child_node.has_dock_panel_style ? child_node.dock_panel_style : DockPanelStyle();
                DockPanelPersistentState& panel_state = get_or_create_dock_panel_state(dock_state, child_node.id);
                if(!panel_state.initialized)
                {
                    panel_state.initialized = true;
                    panel_state.closed = false;
                    panel_state.mode = style.initial_mode;
                    panel_state.rect = RectF(
                        rect.offset_x + style.floating_position.x,
                        rect.offset_y + style.floating_position.y,
                        max(style.floating_size.x, style.min_floating_size.x),
                        max(style.floating_size.y, style.min_floating_size.y));
                    panel_state.z_order = dock_state.dock_next_z_order++;
                }
                if(child_node.dock_panel_open && *child_node.dock_panel_open)
                {
                    panel_state.closed = false;
                }
                bool visible = child_node.dock_panel_open ? *child_node.dock_panel_open : !panel_state.closed;
                Ref<ItemQueryState> result = get_or_create_query_state(child_node.id);
                result->states.insert_or_assign(Name("gui.open"), Any(visible));

                NodeLayout& child_layout = m_layouts[child];
                child_layout.dock_panel_child = true;
                child_layout.dock_panel_visible = visible;
                child_layout.dock_space_id = node.id;
                child_layout.dock_panel_style = style;
                child_layout.dock_panel_floating = panel_state.mode == DockPanelMode::floating;
                child_layout.dock_panel_z_order = panel_state.z_order;
                child_layout.dock_leaf_index = U32_MAX;

                if(!visible)
                {
                    child_layout.rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
                    child_layout.clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
                    continue;
                }
                if(panel_state.mode == DockPanelMode::floating)
                {
                    floating_children.push_back(child);
                    dock_tree_remove_panel(dock_state, child_node.id);
                }
                else
                {
                    docking_panel_indices.insert_or_assign(child_node.id, child);
                    live_docking_panels.insert(child_node.id);
                    dock_tree_add_panel(dock_state, child_node.id);
                }
            }

            dock_tree_prune_missing(dock_state, live_docking_panels);
            if(dock_state.dock_root_node != U32_MAX)
            {
                arrange_dock_tree_node(node.id, dock_state.dock_root_node, rect, clip_rect, docking_panel_indices);
            }

            for(u32 child : floating_children)
            {
                Node& child_node = m_submitted_desc.nodes[child];
                NodeLayout& child_layout = m_layouts[child];
                DockPanelPersistentState& panel_state = get_or_create_dock_panel_state(dock_state, child_node.id);
                DockPanelStyle style = child_layout.dock_panel_style;
                panel_state.rect.width = max(panel_state.rect.width, style.min_floating_size.x);
                panel_state.rect.height = max(panel_state.rect.height, style.min_floating_size.y);
                panel_state.rect.width = min(panel_state.rect.width, max(rect.width, 1.0f));
                panel_state.rect.height = min(panel_state.rect.height, max(rect.height, 1.0f));
                panel_state.rect.offset_x = clamp(panel_state.rect.offset_x, rect.offset_x, max(rect.offset_x + rect.width - panel_state.rect.width, rect.offset_x));
                panel_state.rect.offset_y = clamp(panel_state.rect.offset_y, rect.offset_y, max(rect.offset_y + rect.height - panel_state.rect.height, rect.offset_y));
                RectF panel_rect = panel_state.rect;
                RectF content_rect = dock_panel_content_rect(panel_rect, style);
                child_layout.dock_panel_rect = panel_rect;
                child_layout.dock_panel_clip_rect = intersect_rect(panel_rect, clip_rect);
                child_layout.dock_panel_title_rect = style.title_bar ? dock_panel_title_rect(panel_rect, style) : RectF(0.0f, 0.0f, 0.0f, 0.0f);
                child_layout.dock_panel_close_rect = style.title_bar && style.close_button ?
                    dock_panel_close_rect(child_layout.dock_panel_title_rect) :
                    RectF(0.0f, 0.0f, 0.0f, 0.0f);
                child_layout.dock_panel_resize_rect = style.resize_border ? dock_panel_resize_rect(panel_rect, style) : RectF(0.0f, 0.0f, 0.0f, 0.0f);
                layout_node(child, content_rect, intersect_rect(panel_rect, clip_rect));
            }
        }

        RectF Context::layout_layer_root_rect(u32 layer_index)
        {
            luassert(layer_index < m_submitted_desc.layers.size());
            Layer& layer = m_submitted_desc.layers[layer_index];
            luassert(layer.root != U32_MAX && layer.root < m_submitted_desc.nodes.size());
            Node& node = m_submitted_desc.nodes[layer.root];
            RectF screen_rect(0.0f, 0.0f, m_frame_desc.surface_size.x, m_frame_desc.surface_size.y);
            if(root_layer(node))
            {
                layer.screen_position = Float2U(0.0f);
                return screen_rect;
            }

            LayoutMetrics metrics = measure_node(layer.root);
            f32 width = resolve_base_axis_size(node, metrics, true);
            f32 height = resolve_base_axis_size(node, metrics, false);
            width = clamp(width, metrics.min_size.x, min(metrics.max_size.x, max(m_frame_desc.surface_size.x, 1.0f)));
            height = clamp(height, metrics.min_size.y, min(metrics.max_size.y, max(m_frame_desc.surface_size.y, 1.0f)));
            Float2U position = layer.screen_position;

            if(tooltip_layer(node))
            {
                const TooltipDesc& desc = tooltip_desc(node);
                f32 max_width = desc.max_width > 0.0f ? desc.max_width : m_frame_desc.surface_size.x;
                width = min(max(width, 1.0f), min(max_width, max(m_frame_desc.surface_size.x, 1.0f)));
                height = min(max(height, 1.0f), max(m_frame_desc.surface_size.y, 1.0f));
                position.x = m_pointer_pos.x + desc.offset.x;
                position.y = m_pointer_pos.y + desc.offset.y;
                if(position.x + width > m_frame_desc.surface_size.x && m_pointer_pos.x - width - desc.offset.x >= 0.0f)
                {
                    position.x = m_pointer_pos.x - width - desc.offset.x;
                }
                if(position.y + height > m_frame_desc.surface_size.y && m_pointer_pos.y - height - desc.offset.y >= 0.0f)
                {
                    position.y = m_pointer_pos.y - height - desc.offset.y;
                }
            }
            else if(popup_layer(node) && node.popup_owner())
            {
                u32 owner_index = find_submitted_node_index(node.popup_owner());
                if(owner_index != U32_MAX)
                {
                    const Node& owner = m_submitted_desc.nodes[owner_index];
                    const RectF& owner_rect = m_layouts[owner_index].rect;
                    bool owner_in_menu_bar = owner.parent != U32_MAX && m_submitted_desc.nodes[owner.parent].accepts_top_level_menus();
                    width = min(max(width, 1.0f), max(m_frame_desc.surface_size.x, 1.0f));
                    height = min(max(height, 1.0f), max(m_frame_desc.surface_size.y, 1.0f));
                    if(owner_in_menu_bar)
                    {
                        position.x = owner_rect.offset_x;
                        position.y = owner_rect.offset_y + owner_rect.height + 2.0f;
                        if(position.y + height > m_frame_desc.surface_size.y && owner_rect.offset_y - height - 2.0f >= 0.0f)
                        {
                            position.y = owner_rect.offset_y - height - 2.0f;
                        }
                    }
                    else if(color_edit_node(owner))
                    {
                        PopupAnchorState* popup_state = get_widget_state<PopupAnchorState>(node.id);
                        if(popup_state && popup_state->popup_anchor_valid)
                        {
                            touch_widget_state<PopupAnchorState>(node.id);
                            Float2U anchor = popup_state->popup_anchor_position;
                            position.x = anchor.x;
                            position.y = anchor.y + 8.0f;
                            if(position.x + width > m_frame_desc.surface_size.x && anchor.x - width >= 0.0f)
                            {
                                position.x = anchor.x - width;
                            }
                            if(position.y + height > m_frame_desc.surface_size.y && anchor.y - height - 8.0f >= 0.0f)
                            {
                                position.y = anchor.y - height - 8.0f;
                            }
                        }
                        else
                        {
                            f32 label_w = owner.text.empty() ? 0.0f :
                                min(max((f32)owner.text.size() * 8.0f + 8.0f, 80.0f), owner_rect.width * 0.45f);
                            position.x = owner_rect.offset_x + label_w;
                            position.y = owner_rect.offset_y + owner_rect.height + 4.0f;
                            if(position.y + height > m_frame_desc.surface_size.y && owner_rect.offset_y - height - 4.0f >= 0.0f)
                            {
                                position.y = owner_rect.offset_y - height - 4.0f;
                            }
                        }
                    }
                    else
                    {
                        position.x = owner_rect.offset_x + owner_rect.width - 2.0f;
                        position.y = owner_rect.offset_y - 5.0f;
                        if(position.x + width > m_frame_desc.surface_size.x && owner_rect.offset_x - width + 2.0f >= 0.0f)
                        {
                            position.x = owner_rect.offset_x - width + 2.0f;
                        }
                    }
                }
            }

            position.x = clamp(position.x, 0.0f, max(m_frame_desc.surface_size.x - width, 0.0f));
            position.y = clamp(position.y, 0.0f, max(m_frame_desc.surface_size.y - height, 0.0f));
            layer.screen_position = position;
            return RectF(position.x, position.y, max(width, 1.0f), max(height, 1.0f));
        }

        void Context::layout_layers()
        {
            RectF screen_rect(0.0f, 0.0f, m_frame_desc.surface_size.x, m_frame_desc.surface_size.y);
            for(u32 i = 0; i < (u32)m_submitted_desc.layers.size(); ++i)
            {
                const Layer& layer = m_submitted_desc.layers[i];
                if(layer.root == U32_MAX || layer.root >= m_submitted_desc.nodes.size()) continue;
                RectF layer_rect = layout_layer_root_rect(i);
                layout_node(layer.root, layer_rect, screen_rect);
            }
        }

        RectF Context::layout_node(u32 node_index, const RectF& rect, const RectF& clip_rect)
        {
            Node& node = m_submitted_desc.nodes[node_index];
            RectF effective_clip = intersect_rect(rect, clip_rect);
            if(node.has_user_clip_rect)
            {
                RectF user_clip = node.user_clip_rect;
                if(node.layer < m_submitted_desc.layers.size())
                {
                    const Float2U& layer_position = m_submitted_desc.layers[node.layer].screen_position;
                    user_clip.offset_x += layer_position.x;
                    user_clip.offset_y += layer_position.y;
                }
                effective_clip = intersect_rect(effective_clip, user_clip);
            }
            m_layouts[node_index].rect = rect;
            m_layouts[node_index].clip_rect = effective_clip;

            if(!root_layer(node))
            {
                Ref<ItemQueryState> result = get_or_create_query_state(node.id);
                result->states.insert_or_assign(Name("gui.rect"), Any(rect));
                result->states.insert_or_assign(Name("gui.clip_rect"), Any(effective_clip));
            }

            if(table_layout(node))
            {
                arrange_table_node(node_index, rect, effective_clip);
                return rect;
            }
            if(grid_layout(node))
            {
                arrange_grid_node(node_index, rect, effective_clip);
                return rect;
            }
            if(canvas_layout(node))
            {
                arrange_canvas_node(node_index, rect, effective_clip);
                return rect;
            }
            if(tab_bar_layout(node))
            {
                arrange_tab_bar_node(node_index, rect, effective_clip);
                return rect;
            }
            if(dock_space_layout(node))
            {
                arrange_dock_space_node(node_index, rect, effective_clip);
                return rect;
            }

            if(node.first_child == U32_MAX) return rect;

            bool horizontal = horizontal_layout(node);
            const EdgeInsets& padding = node.layout_desc.padding;
            RectF content_rect(
                rect.offset_x + padding.left,
                rect.offset_y + padding.top,
                max(rect.width - padding.left - padding.right, 0.0f),
                max(rect.height - padding.top - padding.bottom, 0.0f));
            if(window_has_title_bar(node))
            {
                f32 title_bar_height = window_title_bar_height();
                content_rect.offset_y += title_bar_height;
                content_rect.height = max(content_rect.height - title_bar_height, 0.0f);
            }
            RectF viewport_rect = content_rect;

            Vector<u32> children;
            Vector<u32> absolute_children;
            Vector<LayoutMetrics> child_metrics;
            Vector<f32> main_sizes;
            f32 total_base_main = 0.0f;
            f32 total_fill_weight = 0.0f;
            f32 total_shrink_capacity = 0.0f;
            bool has_grid_child = false;
            for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
            {
                Node& child_node = m_submitted_desc.nodes[child];
                if(absolute_node(child_node))
                {
                    absolute_children.push_back(child);
                    continue;
                }
                if(grid_layout(child_node))
                {
                    has_grid_child = true;
                }
                LayoutMetrics metrics = measure_node(child);
                f32 base_main = resolve_base_axis_size(child_node, metrics, horizontal);
                f32 min_main = axis_value(metrics.min_size, horizontal);
                base_main = max(base_main, min_main);
                children.push_back(child);
                child_metrics.push_back(metrics);
                main_sizes.push_back(base_main);
                total_base_main += base_main;
                total_shrink_capacity += max(base_main - min_main, 0.0f);
                if(axis_policy(child_node.layout_style, horizontal) == SizePolicy::fill)
                {
                    total_fill_weight += max(axis_fill_weight(child_node.layout_style, horizontal), 0.0f);
                }
            }

            auto refresh_axis_metrics = [&](f32 grid_available_width) {
                total_base_main = 0.0f;
                total_fill_weight = 0.0f;
                total_shrink_capacity = 0.0f;
                for(usize i = 0; i < children.size(); ++i)
                {
                    Node& child_node = m_submitted_desc.nodes[children[i]];
                    LayoutMetrics metrics = grid_layout(child_node) ?
                        apply_layout_style(child_node, measure_grid_node(children[i], grid_available_width)) :
                        measure_node(children[i]);
                    child_metrics[i] = metrics;
                    f32 base_main = resolve_base_axis_size(child_node, metrics, horizontal);
                    f32 min_main = axis_value(metrics.min_size, horizontal);
                    base_main = max(base_main, min_main);
                    main_sizes[i] = base_main;
                    total_base_main += base_main;
                    total_shrink_capacity += max(base_main - min_main, 0.0f);
                    if(axis_policy(child_node.layout_style, horizontal) == SizePolicy::fill)
                    {
                        total_fill_weight += max(axis_fill_weight(child_node.layout_style, horizontal), 0.0f);
                    }
                }
            };

            auto layout_absolute_children = [&]() {
                RectF surface_clip(0.0f, 0.0f, m_frame_desc.surface_size.x, m_frame_desc.surface_size.y);
                for(u32 child : absolute_children)
                {
                    Node& child_node = m_submitted_desc.nodes[child];
                    LayoutMetrics metrics = measure_node(child);
                    f32 width = resolve_base_axis_size(child_node, metrics, true);
                    f32 height = resolve_base_axis_size(child_node, metrics, false);
                    Float2U position = child_node.position;
                    if(child_node.layer < m_submitted_desc.layers.size())
                    {
                        const Float2U& layer_position = m_submitted_desc.layers[child_node.layer].screen_position;
                        position.x += layer_position.x;
                        position.y += layer_position.y;
                    }
                    if(tooltip_layer(child_node))
                    {
                        const TooltipDesc& desc = tooltip_desc(child_node);
                        f32 max_width = desc.max_width > 0.0f ? desc.max_width : m_frame_desc.surface_size.x;
                        width = min(max(width, 1.0f), min(max_width, max(m_frame_desc.surface_size.x, 1.0f)));
                        height = min(max(height, 1.0f), max(m_frame_desc.surface_size.y, 1.0f));
                        position.x = m_pointer_pos.x + desc.offset.x;
                        position.y = m_pointer_pos.y + desc.offset.y;
                        if(position.x + width > m_frame_desc.surface_size.x && m_pointer_pos.x - width - desc.offset.x >= 0.0f)
                        {
                            position.x = m_pointer_pos.x - width - desc.offset.x;
                        }
                        if(position.y + height > m_frame_desc.surface_size.y && m_pointer_pos.y - height - desc.offset.y >= 0.0f)
                        {
                            position.y = m_pointer_pos.y - height - desc.offset.y;
                        }
                        position.x = clamp(position.x, 0.0f, max(m_frame_desc.surface_size.x - width, 0.0f));
                        position.y = clamp(position.y, 0.0f, max(m_frame_desc.surface_size.y - height, 0.0f));
                    }
                    else if(popup_layer(child_node) && child_node.popup_owner())
                    {
                        u32 owner_index = find_submitted_node_index(child_node.popup_owner());
                        if(owner_index != U32_MAX)
                        {
                            const Node& owner = m_submitted_desc.nodes[owner_index];
                            const RectF& owner_rect = m_layouts[owner_index].rect;
                            bool owner_in_menu_bar = owner.parent != U32_MAX && m_submitted_desc.nodes[owner.parent].accepts_top_level_menus();
                            width = min(max(width, 1.0f), max(m_frame_desc.surface_size.x, 1.0f));
                            height = min(max(height, 1.0f), max(m_frame_desc.surface_size.y, 1.0f));
                            if(owner_in_menu_bar)
                            {
                                position.x = owner_rect.offset_x;
                                position.y = owner_rect.offset_y + owner_rect.height + 2.0f;
                                if(position.y + height > m_frame_desc.surface_size.y && owner_rect.offset_y - height - 2.0f >= 0.0f)
                                {
                                    position.y = owner_rect.offset_y - height - 2.0f;
                                }
                            }
                            else if(color_edit_node(owner))
                            {
                                PopupAnchorState* popup_state = get_widget_state<PopupAnchorState>(child_node.id);
                                if(popup_state && popup_state->popup_anchor_valid)
                                {
                                    touch_widget_state<PopupAnchorState>(child_node.id);
                                    Float2U anchor = popup_state->popup_anchor_position;
                                    position.x = anchor.x;
                                    position.y = anchor.y + 8.0f;
                                    if(position.x + width > m_frame_desc.surface_size.x && anchor.x - width >= 0.0f)
                                    {
                                        position.x = anchor.x - width;
                                    }
                                    if(position.y + height > m_frame_desc.surface_size.y && anchor.y - height - 8.0f >= 0.0f)
                                    {
                                        position.y = anchor.y - height - 8.0f;
                                    }
                                }
                                else
                                {
                                    f32 label_w = owner.text.empty() ? 0.0f :
                                        min(max((f32)owner.text.size() * 8.0f + 8.0f, 80.0f), owner_rect.width * 0.45f);
                                    position.x = owner_rect.offset_x + label_w;
                                    position.y = owner_rect.offset_y + owner_rect.height + 4.0f;
                                    if(position.y + height > m_frame_desc.surface_size.y && owner_rect.offset_y - height - 4.0f >= 0.0f)
                                    {
                                        position.y = owner_rect.offset_y - height - 4.0f;
                                    }
                                }
                            }
                            else
                            {
                                position.x = owner_rect.offset_x + owner_rect.width - 2.0f;
                                position.y = owner_rect.offset_y - 5.0f;
                                if(position.x + width > m_frame_desc.surface_size.x && owner_rect.offset_x - width + 2.0f >= 0.0f)
                                {
                                    position.x = owner_rect.offset_x - width + 2.0f;
                                }
                            }
                            position.x = clamp(position.x, 0.0f, max(m_frame_desc.surface_size.x - width, 0.0f));
                            position.y = clamp(position.y, 0.0f, max(m_frame_desc.surface_size.y - height, 0.0f));
                        }
                    }
                    RectF child_rect(position.x, position.y, max(width, 1.0f), max(height, 1.0f));
                    layout_node(child, child_rect, surface_clip);
                }
            };

            if(children.empty())
            {
                if(scroll_layout(node))
                {
                    NodeLayout& layout = m_layouts[node_index];
                    layout.scroll_viewport_size = Float2U(max(viewport_rect.width, 1.0f), max(viewport_rect.height, 1.0f));
                    layout.scroll_content_size = layout.scroll_viewport_size;
                    layout.scroll_has_vertical = false;
                    layout.scroll_has_horizontal = false;
                    Ref<ScrollState> persistent = get_or_create_widget_state<ScrollState>(node.id);
                    persistent->scroll_x = 0.0f;
                    persistent->scroll_y = 0.0f;
                }
                layout_absolute_children();
                return rect;
            }

            f32 gap = node.layout_desc.gap;
            f32 total_gap = gap * (f32)(children.size() - 1);
            if(scroll_layout(node))
            {
                if(has_grid_child)
                {
                    refresh_axis_metrics(viewport_rect.width);
                }

                auto compute_scroll_content_size = [&]() {
                    f32 content_main = total_base_main + total_gap;
                    f32 content_cross = 0.0f;
                    for(usize i = 0; i < children.size(); ++i)
                    {
                        Node& child_node = m_submitted_desc.nodes[children[i]];
                        bool cross_x_axis = !horizontal;
                        f32 child_cross = node.layout_desc.cross_axis_alignment == LayoutCrossAxisAlignment::stretch &&
                            axis_policy(child_node.layout_style, cross_x_axis) != SizePolicy::fixed ?
                            axis_value(child_metrics[i].min_size, cross_x_axis) :
                            resolve_base_axis_size(child_node, child_metrics[i], cross_x_axis);
                        child_cross = clamp(child_cross,
                            axis_value(child_metrics[i].min_size, cross_x_axis),
                            axis_value(child_metrics[i].max_size, cross_x_axis));
                        content_cross = max(content_cross, child_cross);
                    }
                    return horizontal ? Float2U(content_main, content_cross) : Float2U(content_cross, content_main);
                };

                Float2U raw_content_size = compute_scroll_content_size();
                f32 raw_content_width = raw_content_size.x;
                f32 raw_content_height = raw_content_size.y;
                auto compute_scrollbars = [&]() {
                    bool vertical = raw_content_height > viewport_rect.height + 0.5f;
                    bool horizontal_bar = raw_content_width > viewport_rect.width + 0.5f;
                    f32 bar_padding = scroll_bar_padding();
                    f32 padded_width = max(viewport_rect.width - (vertical ? bar_padding : 0.0f), 1.0f);
                    f32 padded_height = max(viewport_rect.height - (horizontal_bar ? bar_padding : 0.0f), 1.0f);
                    if(vertical && raw_content_width > padded_width + 0.5f)
                    {
                        horizontal_bar = true;
                        padded_height = max(viewport_rect.height - bar_padding, 1.0f);
                    }
                    if(horizontal_bar && raw_content_height > padded_height + 0.5f)
                    {
                        vertical = true;
                        padded_width = max(viewport_rect.width - bar_padding, 1.0f);
                    }
                    return Float4U(vertical ? 1.0f : 0.0f, horizontal_bar ? 1.0f : 0.0f, padded_width, padded_height);
                };

                Float4U scrollbar_info = compute_scrollbars();
                bool has_vertical_bar = scrollbar_info.x > 0.5f;
                bool has_horizontal_bar = scrollbar_info.y > 0.5f;
                f32 padded_viewport_width = scrollbar_info.z;
                f32 padded_viewport_height = scrollbar_info.w;

                if(has_grid_child && has_vertical_bar &&
                    (padded_viewport_width + 0.5f < viewport_rect.width || padded_viewport_width > viewport_rect.width + 0.5f))
                {
                    refresh_axis_metrics(padded_viewport_width);
                    raw_content_size = compute_scroll_content_size();
                    raw_content_width = raw_content_size.x;
                    raw_content_height = raw_content_size.y;
                    scrollbar_info = compute_scrollbars();
                    has_vertical_bar = scrollbar_info.x > 0.5f;
                    has_horizontal_bar = scrollbar_info.y > 0.5f;
                    padded_viewport_width = scrollbar_info.z;
                    padded_viewport_height = scrollbar_info.w;
                }

                NodeLayout& layout = m_layouts[node_index];
                layout.scroll_has_vertical = has_vertical_bar;
                layout.scroll_has_horizontal = has_horizontal_bar;
                layout.scroll_viewport_size = Float2U(padded_viewport_width, padded_viewport_height);
                layout.scroll_content_size = Float2U(
                    max(raw_content_width, padded_viewport_width),
                    max(raw_content_height, padded_viewport_height));

                Ref<ScrollState> persistent = get_or_create_widget_state<ScrollState>(node.id);
                persistent->scroll_x = clamp(persistent->scroll_x, 0.0f, scroll_max_x(layout));
                persistent->scroll_y = clamp(persistent->scroll_y, 0.0f, scroll_max_y(layout));

                content_rect = viewport_rect;
                content_rect.width = padded_viewport_width;
                content_rect.height = padded_viewport_height;
                content_rect.offset_x -= persistent->scroll_x;
                content_rect.offset_y -= persistent->scroll_y;
            }
            f32 available_main = horizontal ? content_rect.width : content_rect.height;
            if(scroll_layout(node))
            {
                available_main = max(available_main, total_base_main + total_gap);
            }
            f32 remaining = available_main - total_base_main - total_gap;
            if(remaining > 0.0f && total_fill_weight > 0.0f)
            {
                for(usize i = 0; i < children.size(); ++i)
                {
                    Node& child_node = m_submitted_desc.nodes[children[i]];
                    if(axis_policy(child_node.layout_style, horizontal) != SizePolicy::fill) continue;
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
            if(node.layout_desc.main_axis_alignment == LayoutMainAxisAlignment::center)
            {
                main_offset = free_main * 0.5f;
            }
            else if(node.layout_desc.main_axis_alignment == LayoutMainAxisAlignment::end)
            {
                main_offset = free_main;
            }
            else if(node.layout_desc.main_axis_alignment == LayoutMainAxisAlignment::space_between && children.size() > 1)
            {
                gap += free_main / (f32)(children.size() - 1);
                main_offset = 0.0f;
            }

            f32 main_cursor = (horizontal ? content_rect.offset_x : content_rect.offset_y) + main_offset;
            f32 cross_start = horizontal ? content_rect.offset_y : content_rect.offset_x;
            f32 available_cross = horizontal ? content_rect.height : content_rect.width;
            RectF child_clip = effective_clip;
            if(scroll_layout(node))
            {
                const NodeLayout& layout = m_layouts[node_index];
                child_clip = intersect_rect(
                    RectF(viewport_rect.offset_x, viewport_rect.offset_y, layout.scroll_viewport_size.x, layout.scroll_viewport_size.y),
                    effective_clip);
            }
            for(usize i = 0; i < children.size(); ++i)
            {
                Node& child_node = m_submitted_desc.nodes[children[i]];
                bool cross_x_axis = !horizontal;
                f32 cross_size;
                if(node.layout_desc.cross_axis_alignment == LayoutCrossAxisAlignment::stretch &&
                    axis_policy(child_node.layout_style, cross_x_axis) != SizePolicy::fixed)
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
                if(node.layout_desc.cross_axis_alignment == LayoutCrossAxisAlignment::center)
                {
                    cross_offset = max(available_cross - cross_size, 0.0f) * 0.5f;
                }
                else if(node.layout_desc.cross_axis_alignment == LayoutCrossAxisAlignment::end)
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
                layout_node(children[i], child_rect, child_clip);
                main_cursor += main_sizes[i] + gap;
            }
            layout_absolute_children();
            return rect;
        }
    }
}
