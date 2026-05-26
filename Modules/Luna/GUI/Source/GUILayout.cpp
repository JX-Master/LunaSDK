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
                f32 h = font_size + 4.0f;
                f32 max_width = F32_MAX;
                if(node.parent != U32_MAX)
                {
                    const GUINode& parent = m_submitted_desc.nodes[node.parent];
                    if(parent.kind == GUINodeKind::tooltip)
                    {
                        f32 tooltip_width = parent.requested_size.width > 0.0f ? parent.requested_size.width : parent.tooltip_desc.max_width;
                        if(tooltip_width <= 0.0f)
                        {
                            tooltip_width = m_frame_desc.surface_size.x;
                        }
                        tooltip_width = min(tooltip_width, max(m_frame_desc.surface_size.x, 1.0f));
                        max_width = max(tooltip_width - parent.layout_desc.padding.left - parent.layout_desc.padding.right, 1.0f);
                        VG::TextArrangeSection section;
                        section.font_file = Font::get_default_font();
                        section.font_index = 0;
                        section.font_size = font_size;
                        section.num_chars = node.text.size();
                        auto arranged = VG::arrange_text(node.text.c_str(), node.text.size(), {&section, 1},
                            RectF(0.0f, 0.0f, max_width, 100000.0f),
                            VG::TextAlignment::begin, VG::TextAlignment::begin);
                        w = max(arranged.bounding_rect.width, 1.0f);
                        h = max(arranged.bounding_rect.height, h);
                    }
                }
                metrics.min_size = Float2U(min(w, 32.0f), h);
                metrics.preferred_size = Float2U(min(w, max_width), h);
                metrics.max_size = Float2U(max_width, h);
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
            case GUINodeKind::selectable:
            {
                f32 w = max(text_width + 24.0f, 72.0f);
                metrics.min_size = Float2U(72.0f, 26.0f);
                metrics.preferred_size = Float2U(w, 26.0f);
                metrics.max_size = Float2U(F32_MAX, 26.0f);
                break;
            }
            case GUINodeKind::menu:
            {
                bool top_level = node.parent != U32_MAX && m_submitted_desc.nodes[node.parent].kind == GUINodeKind::menu_bar;
                f32 w = top_level ? max(text_width + 22.0f, 42.0f) : max(text_width + 62.0f, 132.0f);
                f32 h = top_level ? menu_bar_height() - 6.0f : menu_item_height();
                metrics.min_size = Float2U(top_level ? min(w, 42.0f) : 96.0f, h);
                metrics.preferred_size = Float2U(w, h);
                metrics.max_size = Float2U(F32_MAX, h);
                break;
            }
            case GUINodeKind::menu_item:
            {
                f32 shortcut_width = node.shortcut.empty() ? 0.0f : menu_text_width(node.shortcut) + 28.0f;
                f32 w = max(text_width + shortcut_width + 42.0f, 132.0f);
                metrics.min_size = Float2U(96.0f, menu_item_height());
                metrics.preferred_size = Float2U(w, menu_item_height());
                metrics.max_size = Float2U(F32_MAX, menu_item_height());
                break;
            }
            case GUINodeKind::menu_separator:
                metrics.min_size = Float2U(1.0f, menu_separator_height());
                metrics.preferred_size = Float2U(96.0f, menu_separator_height());
                metrics.max_size = Float2U(F32_MAX, menu_separator_height());
                break;
            case GUINodeKind::checkbox:
            {
                f32 w = max(text_width + 30.0f, 80.0f);
                metrics.min_size = Float2U(26.0f, 26.0f);
                metrics.preferred_size = Float2U(w, 26.0f);
                metrics.max_size = Float2U(F32_MAX, 26.0f);
                break;
            }
            case GUINodeKind::toggle_switch:
            {
                f32 w = max(text_width + 58.0f, 72.0f);
                metrics.min_size = Float2U(46.0f, 28.0f);
                metrics.preferred_size = Float2U(w, 28.0f);
                metrics.max_size = Float2U(F32_MAX, 28.0f);
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
            case GUINodeKind::tree_node:
            {
                f32 indent = tree_node_indent_width() * (f32)node.tree_depth;
                f32 w = max(text_width + indent + 34.0f, 80.0f);
                metrics.min_size = Float2U(min(w, 80.0f), 26.0f);
                metrics.preferred_size = Float2U(w, 26.0f);
                metrics.max_size = Float2U(F32_MAX, 26.0f);
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
            case GUINodeKind::hit_box:
                metrics.min_size = Float2U(1.0f, 1.0f);
                metrics.preferred_size = Float2U(1.0f, 1.0f);
                metrics.max_size = Float2U(F32_MAX, F32_MAX);
                break;
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
            case GUINodeKind::tab_bar:
            {
                f32 min_header_width = 0.0f;
                f32 preferred_header_width = 0.0f;
                GUILayoutMetrics content_metrics;
                bool has_content = false;
                for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
                {
                    const GUINode& child_node = m_submitted_desc.nodes[child];
                    if(child_node.kind != GUINodeKind::tab_item) continue;
                    if(child_node.bool_value && !*child_node.bool_value) continue;
                    f32 ideal_width = tab_item_ideal_width(child_node);
                    min_header_width += tab_item_min_width();
                    preferred_header_width += ideal_width;
                    if(child_node.selected || (get_or_create_persistent_state(node.id).tab_selected_id == child_node.id))
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
                break;
            }
            default:
            {
                bool horizontal = node.kind == GUINodeKind::h_layout || node.kind == GUINodeKind::menu_bar;
                const GUIEdgeInsets& padding = node.layout_desc.padding;
                f32 gap = node.layout_desc.gap;
                f32 min_main = 0.0f;
                f32 preferred_main = 0.0f;
                f32 min_cross = 0.0f;
                f32 preferred_cross = 0.0f;
                u32 child_count = 0;
                for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
                {
                    const GUINode& child_node = m_submitted_desc.nodes[child];
                    if(is_absolute_node(child_node)) continue;
                    GUILayoutMetrics child_metrics = measure_node(child);
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
                break;
            }
            }

            metrics = apply_layout_style(node, metrics);
            m_layouts[node_index].metrics = metrics;
            m_layouts[node_index].metrics_valid = true;
            return metrics;
        }

        void GUIContext::arrange_tab_bar_node(u32 node_index, const RectF& rect, const RectF& clip_rect)
        {
            GUINode& node = m_submitted_desc.nodes[node_index];
            PersistentItemState& state = get_or_create_persistent_state(node.id);
            Vector<u32> live_tabs;
            live_tabs.reserve(8);
            GUIID selected = state.tab_selected_id;
            GUIID first_open = 0;
            bool selected_open = false;

            for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
            {
                GUINode& child_node = m_submitted_desc.nodes[child];
                if(child_node.kind != GUINodeKind::tab_item) continue;
                bool open = !child_node.bool_value || *child_node.bool_value;
                if(!open)
                {
                    m_layouts[child].tab_content_visible = false;
                    m_layouts[child].rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
                    m_layouts[child].clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
                    continue;
                }
                live_tabs.push_back(child);
                if(!first_open && !test_flags(child_node.tab_item_flags, GUITabItemFlag::button))
                {
                    first_open = child_node.id;
                }
                if(child_node.selected && !test_flags(child_node.tab_item_flags, GUITabItemFlag::button))
                {
                    selected = child_node.id;
                }
                if(selected && selected == child_node.id)
                {
                    selected_open = !test_flags(child_node.tab_item_flags, GUITabItemFlag::button);
                }
            }
            if(!selected || !selected_open)
            {
                selected = first_open;
            }
            state.tab_selected_id = selected;

            auto live_tab_index = [&](GUIID id) -> u32 {
                for(u32 tab : live_tabs)
                {
                    if(m_submitted_desc.nodes[tab].id == id) return tab;
                }
                return U32_MAX;
            };
            for(usize i = 0; i < state.tab_order.size();)
            {
                if(live_tab_index(state.tab_order[i]) == U32_MAX)
                {
                    state.tab_order.erase(state.tab_order.begin() + i);
                }
                else
                {
                    ++i;
                }
            }
            for(u32 tab : live_tabs)
            {
                GUIID id = m_submitted_desc.nodes[tab].id;
                if(!tab_order_contains(state, id))
                {
                    state.tab_order.push_back(id);
                }
            }

            Vector<u32> tabs;
            tabs.reserve(live_tabs.size());
            for(GUIID id : state.tab_order)
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
            bool use_scroll = test_flags(node.tab_bar_flags, GUITabBarFlag::fitting_scroll);
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
            if(!bar_layout.tab_scrollable && test_flags(node.tab_bar_flags, GUITabBarFlag::fitting_shrink) &&
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
            state.tab_scroll_x = bar_layout.tab_scrollable ? clamp(state.tab_scroll_x, 0.0f, bar_layout.tab_scroll_max) : 0.0f;

            RectF header_clip = intersect_rect(header_area_rect, clip_rect);
            f32 cursor_x = header_area_rect.offset_x - state.tab_scroll_x;
            for(usize i = 0; i < tabs.size(); ++i)
            {
                u32 tab = tabs[i];
                GUINode& tab_node = m_submitted_desc.nodes[tab];
                NodeLayout& tab_layout = m_layouts[tab];
                bool content_visible = tab_node.id == selected && !test_flags(tab_node.tab_item_flags, GUITabItemFlag::button);
                RectF tab_header(cursor_x, header_rect.offset_y + 3.0f, max(widths[i], 1.0f), max(header_rect.height - 4.0f, 1.0f));
                tab_layout.tab_header_rect = tab_header;
                tab_layout.tab_header_clip_rect = header_clip;
                tab_layout.tab_content_visible = content_visible;
                tab_layout.tab_close_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
                if(tab_node.bool_value && !test_flags(tab_node.tab_item_flags, GUITabItemFlag::no_close_button))
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
                    ItemResult& result = get_or_create_current_result(tab_node.id);
                    result.states.insert_or_assign(Name("gui.rect"), Any(tab_header));
                    result.states.insert_or_assign(Name("gui.clip_rect"), Any(header_clip));
                }
                ItemResult& result = get_or_create_current_result(tab_node.id);
                result.states.insert_or_assign(Name("gui.rect"), Any(tab_header));
                result.states.insert_or_assign(Name("gui.clip_rect"), Any(header_clip));
                result.states.insert_or_assign(Name("gui.open"), Any(true));
            }
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

        u32 GUIContext::new_dock_leaf(PersistentItemState& dock_state, GUIID panel_id, u32 parent)
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

        bool GUIContext::dock_tree_contains_panel(const PersistentItemState& dock_state, GUIID panel_id) const
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
                for(GUIID tab : node.tabs)
                {
                    if(tab == panel_id) return true;
                }
            }
            return false;
        }

        void GUIContext::dock_tree_add_panel(PersistentItemState& dock_state, GUIID panel_id)
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

        static void dock_tree_replace_node_with_child(PersistentItemState& dock_state, u32 node_index, u32 child_index)
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

        static void dock_tree_remove_empty_leaf(PersistentItemState& dock_state, u32 leaf_index)
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

        bool GUIContext::dock_tree_remove_panel(PersistentItemState& dock_state, GUIID panel_id)
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

        void GUIContext::dock_tree_dock_panel(PersistentItemState& dock_state, GUIID panel_id, u32 target_leaf, GUIDockDropDirection direction)
        {
            if(!panel_id) return;
            dock_tree_remove_panel(dock_state, panel_id);
            if(direction == GUIDockDropDirection::none) direction = GUIDockDropDirection::center;
            if(dock_state.dock_root_node == U32_MAX || dock_state.dock_root_node >= dock_state.dock_nodes.size() || target_leaf >= dock_state.dock_nodes.size())
            {
                dock_state.dock_root_node = new_dock_leaf(dock_state, panel_id);
                return;
            }
            if(direction == GUIDockDropDirection::center || dock_state.dock_nodes[target_leaf].split)
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
            split.split_axis = (direction == GUIDockDropDirection::left || direction == GUIDockDropDirection::right) ? GUIDockSplitAxis::x : GUIDockSplitAxis::y;
            split.split_ratio = 0.5f;
            if(direction == GUIDockDropDirection::left || direction == GUIDockDropDirection::up)
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

        static bool dock_tree_prune_node(PersistentItemState& dock_state, u32 node_index, const HashSet<GUIID, GUIIDHash>& live_panels)
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
                for(GUIID tab : node.tabs)
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

        void GUIContext::dock_tree_prune_missing(PersistentItemState& dock_state, const HashSet<GUIID, GUIIDHash>& live_panels)
        {
            if(dock_state.dock_root_node == U32_MAX || dock_state.dock_root_node >= dock_state.dock_nodes.size()) return;
            if(!dock_tree_prune_node(dock_state, dock_state.dock_root_node, live_panels))
            {
                dock_state.dock_root_node = U32_MAX;
            }
        }

        GUIID GUIContext::dock_tree_selected_panel(PersistentItemState& dock_state, u32 leaf_index)
        {
            if(leaf_index >= dock_state.dock_nodes.size()) return 0;
            DockTreeNode& leaf = dock_state.dock_nodes[leaf_index];
            if(leaf.split || leaf.tabs.empty()) return 0;
            for(GUIID tab : leaf.tabs)
            {
                if(tab == leaf.selected_tab) return tab;
            }
            leaf.selected_tab = leaf.tabs[0];
            return leaf.selected_tab;
        }

        void GUIContext::arrange_dock_tree_node(GUIID dock_space_id, u32 tree_node_index, const RectF& rect, const RectF& clip_rect, const HashMap<GUIID, u32, GUIIDHash>& panel_indices)
        {
            PersistentItemState& dock_state = get_or_create_persistent_state(dock_space_id);
            if(tree_node_index >= dock_state.dock_nodes.size()) return;
            DockTreeNode& tree_node = dock_state.dock_nodes[tree_node_index];
            tree_node.rect = rect;
            if(tree_node.split)
            {
                f32 splitter_size = dock_panel_splitter_size();
                if(tree_node.split_axis == GUIDockSplitAxis::x)
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

            GUIID selected_panel = dock_tree_selected_panel(dock_state, tree_node_index);
            for(GUIID panel_id : tree_node.tabs)
            {
                auto iter = panel_indices.find(panel_id);
                if(iter == panel_indices.end()) continue;
                u32 panel_index = iter->second;
                NodeLayout& panel_layout = m_layouts[panel_index];
                GUINode& panel_node = m_submitted_desc.nodes[panel_index];
                GUIDockPanelStyle style = panel_layout.dock_panel_style;
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
                panel_node.render_layer = GUIRenderLayer::main;
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

        void GUIContext::arrange_dock_space_node(u32 node_index, const RectF& rect, const RectF& clip_rect)
        {
            GUINode& node = m_submitted_desc.nodes[node_index];
            PersistentItemState& dock_state = get_or_create_persistent_state(node.id);
            Vector<u32> floating_children;
            HashMap<GUIID, u32, GUIIDHash> docking_panel_indices;
            HashSet<GUIID, GUIIDHash> live_docking_panels;

            for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
            {
                GUINode& child_node = m_submitted_desc.nodes[child];
                GUIDockPanelStyle style = child_node.has_dock_panel_style ? child_node.dock_panel_style : GUIDockPanelStyle();
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
                ItemResult& result = get_or_create_current_result(child_node.id);
                result.states.insert_or_assign(Name("gui.open"), Any(visible));

                NodeLayout& child_layout = m_layouts[child];
                child_layout.dock_panel_child = true;
                child_layout.dock_panel_visible = visible;
                child_layout.dock_space_id = node.id;
                child_layout.dock_panel_style = style;
                child_layout.dock_panel_floating = panel_state.mode == GUIDockPanelMode::floating;
                child_layout.dock_panel_z_order = panel_state.z_order;
                child_layout.dock_leaf_index = U32_MAX;

                if(!visible)
                {
                    child_layout.rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
                    child_layout.clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
                    continue;
                }
                if(panel_state.mode == GUIDockPanelMode::floating)
                {
                    child_node.render_layer = GUIRenderLayer::overlay;
                    floating_children.push_back(child);
                    dock_tree_remove_panel(dock_state, child_node.id);
                }
                else
                {
                    child_node.render_layer = node.render_layer;
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
                GUINode& child_node = m_submitted_desc.nodes[child];
                NodeLayout& child_layout = m_layouts[child];
                PersistentItemState& dock_state_ref = get_or_create_persistent_state(node.id);
                DockPanelPersistentState& panel_state = get_or_create_dock_panel_state(dock_state_ref, child_node.id);
                GUIDockPanelStyle style = child_layout.dock_panel_style;
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

        RectF GUIContext::layout_node(u32 node_index, const RectF& rect, const RectF& clip_rect)
        {
            GUINode& node = m_submitted_desc.nodes[node_index];
            RectF effective_clip = intersect_rect(rect, clip_rect);
            if(node.has_user_clip_rect)
            {
                effective_clip = intersect_rect(effective_clip, node.user_clip_rect);
            }
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
            if(node.kind == GUINodeKind::tab_bar)
            {
                arrange_tab_bar_node(node_index, rect, effective_clip);
                return rect;
            }
            if(node.kind == GUINodeKind::dock_space)
            {
                arrange_dock_space_node(node_index, rect, effective_clip);
                return rect;
            }

            if(node.first_child == U32_MAX) return rect;

            bool horizontal = node.kind == GUINodeKind::h_layout || node.kind == GUINodeKind::menu_bar;
            const GUIEdgeInsets& padding = node.layout_desc.padding;
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
            Vector<GUILayoutMetrics> child_metrics;
            Vector<f32> main_sizes;
            f32 total_base_main = 0.0f;
            f32 total_fill_weight = 0.0f;
            f32 total_shrink_capacity = 0.0f;
            for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
            {
                GUINode& child_node = m_submitted_desc.nodes[child];
                if(is_absolute_node(child_node))
                {
                    absolute_children.push_back(child);
                    continue;
                }
                GUILayoutMetrics metrics = measure_node(child);
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

            auto layout_absolute_children = [&]() {
                RectF surface_clip(0.0f, 0.0f, m_frame_desc.surface_size.x, m_frame_desc.surface_size.y);
                for(u32 child : absolute_children)
                {
                    GUINode& child_node = m_submitted_desc.nodes[child];
                    GUILayoutMetrics metrics = measure_node(child);
                    f32 width = resolve_base_axis_size(child_node, metrics, true);
                    f32 height = resolve_base_axis_size(child_node, metrics, false);
                    Float2U position = child_node.position;
                    if(child_node.kind == GUINodeKind::tooltip)
                    {
                        f32 max_width = child_node.tooltip_desc.max_width > 0.0f ? child_node.tooltip_desc.max_width : m_frame_desc.surface_size.x;
                        width = min(max(width, 1.0f), min(max_width, max(m_frame_desc.surface_size.x, 1.0f)));
                        height = min(max(height, 1.0f), max(m_frame_desc.surface_size.y, 1.0f));
                        position.x = m_pointer_pos.x + child_node.tooltip_desc.offset.x;
                        position.y = m_pointer_pos.y + child_node.tooltip_desc.offset.y;
                        if(position.x + width > m_frame_desc.surface_size.x && m_pointer_pos.x - width - child_node.tooltip_desc.offset.x >= 0.0f)
                        {
                            position.x = m_pointer_pos.x - width - child_node.tooltip_desc.offset.x;
                        }
                        if(position.y + height > m_frame_desc.surface_size.y && m_pointer_pos.y - height - child_node.tooltip_desc.offset.y >= 0.0f)
                        {
                            position.y = m_pointer_pos.y - height - child_node.tooltip_desc.offset.y;
                        }
                        position.x = clamp(position.x, 0.0f, max(m_frame_desc.surface_size.x - width, 0.0f));
                        position.y = clamp(position.y, 0.0f, max(m_frame_desc.surface_size.y - height, 0.0f));
                    }
                    else if(child_node.kind == GUINodeKind::popup && child_node.popup_owner_id)
                    {
                        u32 owner_index = find_submitted_node_index(child_node.popup_owner_id);
                        if(owner_index != U32_MAX)
                        {
                            const GUINode& owner = m_submitted_desc.nodes[owner_index];
                            const RectF& owner_rect = m_layouts[owner_index].rect;
                            bool owner_in_menu_bar = owner.parent != U32_MAX && m_submitted_desc.nodes[owner.parent].kind == GUINodeKind::menu_bar;
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
                if(node.kind == GUINodeKind::scroll_view)
                {
                    NodeLayout& layout = m_layouts[node_index];
                    layout.scroll_viewport_size = Float2U(max(viewport_rect.width, 1.0f), max(viewport_rect.height, 1.0f));
                    layout.scroll_content_size = layout.scroll_viewport_size;
                    layout.scroll_has_vertical = false;
                    layout.scroll_has_horizontal = false;
                    PersistentItemState& persistent = get_or_create_persistent_state(node.id);
                    persistent.scroll_x = 0.0f;
                    persistent.scroll_y = 0.0f;
                }
                layout_absolute_children();
                return rect;
            }

            f32 gap = node.layout_desc.gap;
            f32 total_gap = gap * (f32)(children.size() - 1);
            if(node.kind == GUINodeKind::scroll_view)
            {
                f32 content_main = total_base_main + total_gap;
                f32 content_cross = 0.0f;
                for(usize i = 0; i < children.size(); ++i)
                {
                    GUINode& child_node = m_submitted_desc.nodes[children[i]];
                    bool cross_x_axis = !horizontal;
                    f32 child_cross = node.layout_desc.cross_axis_alignment == GUILayoutCrossAxisAlignment::stretch &&
                        axis_policy(child_node.layout_style, cross_x_axis) != GUISizePolicy::fixed ?
                        axis_value(child_metrics[i].min_size, cross_x_axis) :
                        resolve_base_axis_size(child_node, child_metrics[i], cross_x_axis);
                    child_cross = clamp(child_cross,
                        axis_value(child_metrics[i].min_size, cross_x_axis),
                        axis_value(child_metrics[i].max_size, cross_x_axis));
                    content_cross = max(content_cross, child_cross);
                }

                f32 raw_content_width = horizontal ? content_main : content_cross;
                f32 raw_content_height = horizontal ? content_cross : content_main;
                bool has_vertical_bar = raw_content_height > viewport_rect.height + 0.5f;
                bool has_horizontal_bar = raw_content_width > viewport_rect.width + 0.5f;
                f32 bar_padding = scroll_bar_padding();
                f32 padded_viewport_width = max(viewport_rect.width - (has_vertical_bar ? bar_padding : 0.0f), 1.0f);
                f32 padded_viewport_height = max(viewport_rect.height - (has_horizontal_bar ? bar_padding : 0.0f), 1.0f);
                if(has_vertical_bar && raw_content_width > padded_viewport_width + 0.5f)
                {
                    has_horizontal_bar = true;
                    padded_viewport_height = max(viewport_rect.height - bar_padding, 1.0f);
                }
                if(has_horizontal_bar && raw_content_height > padded_viewport_height + 0.5f)
                {
                    has_vertical_bar = true;
                    padded_viewport_width = max(viewport_rect.width - bar_padding, 1.0f);
                }

                NodeLayout& layout = m_layouts[node_index];
                layout.scroll_has_vertical = has_vertical_bar;
                layout.scroll_has_horizontal = has_horizontal_bar;
                layout.scroll_viewport_size = Float2U(padded_viewport_width, padded_viewport_height);
                layout.scroll_content_size = Float2U(
                    max(raw_content_width, padded_viewport_width),
                    max(raw_content_height, padded_viewport_height));

                PersistentItemState& persistent = get_or_create_persistent_state(node.id);
                persistent.scroll_x = clamp(persistent.scroll_x, 0.0f, scroll_max_x(layout));
                persistent.scroll_y = clamp(persistent.scroll_y, 0.0f, scroll_max_y(layout));

                content_rect = viewport_rect;
                content_rect.width = padded_viewport_width;
                content_rect.height = padded_viewport_height;
                content_rect.offset_x -= persistent.scroll_x;
                content_rect.offset_y -= persistent.scroll_y;
            }
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
            RectF child_clip = effective_clip;
            if(node.kind == GUINodeKind::scroll_view)
            {
                const NodeLayout& layout = m_layouts[node_index];
                child_clip = intersect_rect(
                    RectF(viewport_rect.offset_x, viewport_rect.offset_y, layout.scroll_viewport_size.x, layout.scroll_viewport_size.y),
                    effective_clip);
            }
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
                layout_node(children[i], child_rect, child_clip);
                main_cursor += main_sizes[i] + gap;
            }
            layout_absolute_children();
            return rect;
        }
    }
}
