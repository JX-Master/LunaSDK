/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUIInput.cpp
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
        namespace
        {
            bool delete_input_text_selection(String& value, PersistentItemState& state)
            {
                if(!input_text_has_selection(value, state)) return false;
                usize begin = 0;
                usize end = 0;
                input_text_selection_range(value, state, begin, end);
                value.erase(begin, end - begin);
                state.text_cursor = begin;
                input_text_clear_selection(state);
                return true;
            }

            String filter_input_text(const String& text)
            {
                String filtered;
                usize offset = 0;
                const c8* src = text.c_str();
                while(offset < text.size() && src[offset])
                {
                    usize len = min(utf8_charlen(src + offset), text.size() - offset);
                    c32 ch = utf8_decode_char(src + offset);
                    if(ch >= 0x20 && ch != 0x7F)
                    {
                        filtered.append(src + offset, len);
                    }
                    offset += len;
                }
                return filtered;
            }
        }

        bool GUIContext::hit_test_table_separator(const Float2U& pos, GUIID& out_id, bool& out_column, u32& out_index) const
        {
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                const GUINode& node = m_submitted_desc.nodes[i];
                if(node.kind != GUINodeKind::table_layout) continue;
                const NodeLayout& layout = m_layouts[i];
                const RectF& clip = layout.clip_rect;
                if(!point_in_rect(pos, layout.rect) || !point_in_rect(pos, clip)) continue;
                const GUITableStyle& style = node.table_desc.style;
                f32 hit_size = max(style.resize_hit_size, style.separator_size);
                if(style.column_separators && style.resize_fixed_columns && layout.table_rows && layout.table_columns > 1)
                {
                    f32 top = layout.table_row_offsets[0];
                    f32 bottom = layout.table_row_offsets.back() + layout.table_row_heights.back();
                    if(pos.y >= top && pos.y < bottom)
                    {
                        for(u32 col = 0; col + 1 < layout.table_columns; ++col)
                        {
                            if(!table_track_is_fixed(node, true, col)) continue;
                            f32 x = layout.table_column_offsets[col] + layout.table_column_widths[col];
                            if(pos.x >= x - hit_size * 0.5f && pos.x <= x + hit_size * 0.5f)
                            {
                                out_id = node.id;
                                out_column = true;
                                out_index = col;
                                return true;
                            }
                        }
                    }
                }
                if(style.row_separators && style.resize_fixed_rows && layout.table_columns && layout.table_rows > 1)
                {
                    f32 left = layout.table_column_offsets[0];
                    f32 right = layout.table_column_offsets.back() + layout.table_column_widths.back();
                    if(pos.x >= left && pos.x < right)
                    {
                        for(u32 row = 0; row + 1 < layout.table_rows; ++row)
                        {
                            if(!table_track_is_fixed(node, false, row)) continue;
                            f32 y = layout.table_row_offsets[row] + layout.table_row_heights[row];
                            if(pos.y >= y - hit_size * 0.5f && pos.y <= y + hit_size * 0.5f)
                            {
                                out_id = node.id;
                                out_column = false;
                                out_index = row;
                                return true;
                            }
                        }
                    }
                }
            }
            return false;
        }

        void GUIContext::update_table_resize_from_pointer(const Float2U& pos)
        {
            if(!m_active_table_resize_id || m_active_table_resize_index == U32_MAX) return;
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                GUINode& node = m_submitted_desc.nodes[i];
                if(node.id != m_active_table_resize_id || node.kind != GUINodeKind::table_layout) continue;
                NodeLayout& layout = m_layouts[i];
                PersistentItemState& persistent = get_or_create_persistent_state(node.id);
                if(m_active_table_resize_column)
                {
                    u32 col = m_active_table_resize_index;
                    if(col >= layout.table_column_offsets.size()) return;
                    if(persistent.table_column_sizes.size() <= col)
                    {
                        persistent.table_column_sizes.resize(col + 1, 0.0f);
                    }
                    persistent.table_column_sizes[col] = max(pos.x - layout.table_column_offsets[col], 24.0f);
                }
                else
                {
                    u32 row = m_active_table_resize_index;
                    if(row >= layout.table_row_offsets.size()) return;
                    if(persistent.table_row_sizes.size() <= row)
                    {
                        persistent.table_row_sizes.resize(row + 1, 0.0f);
                    }
                    persistent.table_row_sizes[row] = max(pos.y - layout.table_row_offsets[row], 20.0f);
                }
                ItemResult& result = get_or_create_current_result(node.id);
                result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
                m_layout_dirty = true;
                return;
            }
        }

        void GUIContext::clamp_scroll_state(GUIID id)
        {
            if(!id || m_layouts.size() != m_submitted_desc.nodes.size()) return;
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                const GUINode& node = m_submitted_desc.nodes[i];
                if(node.id != id || node.kind != GUINodeKind::scroll_view) continue;
                const NodeLayout& layout = m_layouts[i];
                PersistentItemState& state = get_or_create_persistent_state(id);
                state.scroll_x = clamp(state.scroll_x, 0.0f, scroll_max_x(layout));
                state.scroll_y = clamp(state.scroll_y, 0.0f, scroll_max_y(layout));
                return;
            }
        }

        bool GUIContext::hit_test_scrollbar(const Float2U& pos, GUIID& out_id, bool& out_vertical, RectF& out_thumb_rect) const
        {
            if(m_layouts.size() != m_submitted_desc.nodes.size()) return false;
            for(usize i = m_submitted_desc.nodes.size(); i > 0; --i)
            {
                u32 node_index = (u32)(i - 1);
                const GUINode& node = m_submitted_desc.nodes[node_index];
                if(node.kind != GUINodeKind::scroll_view) continue;
                const NodeLayout& layout = m_layouts[node_index];
                if(!point_in_rect(pos, layout.rect) || !point_in_rect(pos, layout.clip_rect)) continue;

                PersistentItemState empty_state;
                auto iter = m_persistent_states.find(node.id);
                const PersistentItemState& state = iter == m_persistent_states.end() ? empty_state : iter->second;
                if(scroll_has_vertical_bar(layout))
                {
                    RectF track = scroll_vertical_track_rect(layout);
                    if(point_in_rect(pos, track))
                    {
                        out_id = node.id;
                        out_vertical = true;
                        out_thumb_rect = scroll_vertical_thumb_rect(layout, state);
                        return true;
                    }
                }
                if(scroll_has_horizontal_bar(layout))
                {
                    RectF track = scroll_horizontal_track_rect(layout);
                    if(point_in_rect(pos, track))
                    {
                        out_id = node.id;
                        out_vertical = false;
                        out_thumb_rect = scroll_horizontal_thumb_rect(layout, state);
                        return true;
                    }
                }
            }
            return false;
        }

        void GUIContext::update_scrollbar_from_pointer(const Float2U& pos)
        {
            if(!m_active_scrollbar_id || m_layouts.size() != m_submitted_desc.nodes.size()) return;
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                const GUINode& node = m_submitted_desc.nodes[i];
                if(node.id != m_active_scrollbar_id || node.kind != GUINodeKind::scroll_view) continue;
                const NodeLayout& layout = m_layouts[i];
                PersistentItemState& state = get_or_create_persistent_state(node.id);
                f32 old_scroll_x = state.scroll_x;
                f32 old_scroll_y = state.scroll_y;
                if(m_active_scrollbar_vertical)
                {
                    RectF thumb = scroll_vertical_thumb_rect(layout, state);
                    RectF track = scroll_vertical_track_rect(layout);
                    f32 travel = max(track.height - thumb.height, 0.0f);
                    f32 t = travel > 0.0f ? (pos.y - track.offset_y - m_active_scrollbar_grab_offset) / travel : 0.0f;
                    state.scroll_y = clamp(t, 0.0f, 1.0f) * scroll_max_y(layout);
                }
                else
                {
                    RectF thumb = scroll_horizontal_thumb_rect(layout, state);
                    RectF track = scroll_horizontal_track_rect(layout);
                    f32 travel = max(track.width - thumb.width, 0.0f);
                    f32 t = travel > 0.0f ? (pos.x - track.offset_x - m_active_scrollbar_grab_offset) / travel : 0.0f;
                    state.scroll_x = clamp(t, 0.0f, 1.0f) * scroll_max_x(layout);
                }
                clamp_scroll_state(node.id);
                if(state.scroll_x != old_scroll_x || state.scroll_y != old_scroll_y)
                {
                    ItemResult& result = get_or_create_current_result(node.id);
                    result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
                    m_layout_dirty = true;
                }
                return;
            }
        }

        bool GUIContext::hit_test_combo_dropdown(const Float2U& pos, GUIID& out_id, i32& out_item) const
        {
            if(!m_open_combo_id || m_layouts.size() != m_submitted_desc.nodes.size()) return false;
            for(usize i = m_submitted_desc.nodes.size(); i > 0; --i)
            {
                u32 node_index = (u32)(i - 1);
                const GUINode& node = m_submitted_desc.nodes[node_index];
                if(node.kind != GUINodeKind::combo || node.id != m_open_combo_id) continue;
                auto iter = m_persistent_states.find(node.id);
                if(iter == m_persistent_states.end() || !iter->second.open) continue;
                RectF dropdown = combo_dropdown_rect(node, m_layouts[node_index].rect, m_frame_desc.surface_size);
                if(!point_in_rect(pos, dropdown)) continue;
                out_id = node.id;
                out_item = combo_dropdown_item_at(node, dropdown, pos);
                return true;
            }
            return false;
        }

        void GUIContext::close_combo_dropdowns_except(GUIID keep_id)
        {
            if(m_open_combo_id && m_open_combo_id != keep_id)
            {
                get_or_create_persistent_state(m_open_combo_id).open = false;
            }
            m_open_combo_id = keep_id;
            for(const GUINode& node : m_submitted_desc.nodes)
            {
                if(node.kind != GUINodeKind::combo) continue;
                PersistentItemState& state = get_or_create_persistent_state(node.id);
                state.open = node.id == keep_id;
            }
        }

        GUIID GUIContext::hit_test_node(u32 node_index, const Float2U& pos, bool filter_kind, GUINodeKind kind) const
        {
            GUIID ret = 0;
            const GUINode& node = m_submitted_desc.nodes[node_index];
            const RectF& rect = m_layouts[node_index].rect;
            const RectF& clip = m_layouts[node_index].clip_rect;
            if((filter_kind ? node.kind == kind : node.interactive) && point_in_rect(pos, rect) && point_in_rect(pos, clip))
            {
                ret = node.id;
            }
            for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
            {
                if(is_absolute_node(m_submitted_desc.nodes[child])) continue;
                GUIID child_hit = hit_test_node(child, pos, filter_kind, kind);
                if(child_hit)
                {
                    ret = child_hit;
                }
            }
            for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
            {
                if(!is_absolute_node(m_submitted_desc.nodes[child])) continue;
                GUIID child_hit = hit_test_node(child, pos, filter_kind, kind);
                if(child_hit)
                {
                    ret = child_hit;
                }
            }
            return ret;
        }

        GUIID GUIContext::hit_test(const Float2U& pos) const
        {
            return m_submitted_desc.nodes.empty() ? 0 : hit_test_node(0, pos, false, GUINodeKind::root);
        }

        GUIID GUIContext::hit_test_node_kind(const Float2U& pos, GUINodeKind kind) const
        {
            return m_submitted_desc.nodes.empty() ? 0 : hit_test_node(0, pos, true, kind);
        }

        GUINode* GUIContext::find_node(GUIID id)
        {
            for(GUINode& node : m_submitted_desc.nodes)
            {
                if(node.id == id) return &node;
            }
            return nullptr;
        }

        u32 GUIContext::hit_test_float_component(const GUINode& node, const RectF& rect, const Float2U& pos) const
        {
            u32 value_count = node.kind == GUINodeKind::slider_float ? 1 : f32_value_count(node);
            if(value_count <= 1) return 0;
            f32 label_w = min(max((f32)node.text.size() * 8.0f + 8.0f, 80.0f), rect.width * 0.45f);
            f32 gap = 4.0f;
            f32 value_area_x = rect.offset_x + label_w;
            f32 value_area_w = max(rect.width - label_w - 8.0f, 1.0f);
            f32 component_w = max((value_area_w - gap * (f32)(value_count - 1)) / (f32)value_count, 1.0f);
            f32 rel = max(pos.x - value_area_x, 0.0f);
            return min((u32)(rel / (component_w + gap)), value_count - 1);
        }

        void GUIContext::update_float_node_from_pointer(GUIID id, const Float2U& pos, const Float2U* old_pos)
        {
            GUINode* node = find_node(id);
            if(!node || !node->f32_value) return;
            if(node->kind != GUINodeKind::slider_float && node->kind != GUINodeKind::drag_float) return;

            RectF rect;
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                if(m_submitted_desc.nodes[i].id == id)
                {
                    rect = m_layouts[i].rect;
                    break;
                }
            }
            f32 label_w = min(max((f32)node->text.size() * 8.0f + 8.0f, 80.0f), rect.width * 0.45f);
            u32 value_count = node->kind == GUINodeKind::slider_float ? 1 : f32_value_count(*node);
            f32 gap = 4.0f;
            f32 value_area_x = rect.offset_x + label_w;
            f32 value_area_w = max(rect.width - label_w - 8.0f, 1.0f);
            f32 component_w = max((value_area_w - gap * (f32)(value_count - 1)) / (f32)value_count, 1.0f);
            u32 component = hit_test_float_component(*node, rect, pos);
            if(m_active_id == id && m_active_float_component != U32_MAX)
            {
                component = min(m_active_float_component, value_count - 1);
            }
            f32 component_x = value_area_x + (component_w + gap) * (f32)component;
            f32 new_value = node->f32_value[component];
            if(node->kind == GUINodeKind::drag_float && node->max_value <= node->min_value)
            {
                if(!old_pos) return;
                f32 speed = node->step_value == 0.0f ? 1.0f : node->step_value;
                new_value += (pos.x - old_pos->x) * speed;
            }
            else
            {
                f32 t = clamp((pos.x - component_x) / component_w, 0.0f, 1.0f);
                new_value = node->min_value + (node->max_value - node->min_value) * t;
            }
            if(node->f32_value[component] != new_value)
            {
                node->f32_value[component] = new_value;
                ItemResult& result = get_or_create_current_result(id);
                result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
            }
        }

        bool GUIContext::input_text_cursor_from_pointer(GUIID id, const Float2U& pos, usize& out_cursor)
        {
            GUINode* node = find_node(id);
            if(!node || node->kind != GUINodeKind::input_text || !node->string_value) return false;
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                if(m_submitted_desc.nodes[i].id == id)
                {
                    RectF text_rect(m_layouts[i].rect.offset_x + 8.0f, m_layouts[i].rect.offset_y,
                        max(m_layouts[i].rect.width - 16.0f, 1.0f), m_layouts[i].rect.height);
                    out_cursor = input_text_cursor_from_x(*node->string_value, pos.x - text_rect.offset_x, 16.0f);
                    return true;
                }
            }
            return false;
        }

        bool GUIContext::update_input_text_selection_from_pointer(GUIID id, const Float2U& pos)
        {
            GUINode* node = find_node(id);
            if(!node || node->kind != GUINodeKind::input_text || !node->string_value) return false;
            usize cursor = 0;
            if(!input_text_cursor_from_pointer(id, pos, cursor)) return false;
            PersistentItemState& state = get_or_create_persistent_state(id);
            if(state.text_select_anchor == USIZE_MAX)
            {
                state.text_select_anchor = clamp_utf8_cursor(*node->string_value, state.text_cursor);
            }
            state.text_cursor = cursor;
            state.text_cursor_blink_start = m_time;
            return true;
        }

        void GUIContext::process_input_events()
        {
            for(const GUIInputEvent& e : m_input_events)
            {
                if(e.type == GUIInputEventType::pointer_enter)
                {
                    m_pointer_inside = true;
                    m_pointer_pos = e.position;
                }
                else if(e.type == GUIInputEventType::pointer_leave)
                {
                    m_pointer_inside = false;
                    m_hovered_id = 0;
                }
                else if(e.type == GUIInputEventType::pointer_move)
                {
                    Float2U old_pos = m_pointer_pos;
                    m_pointer_inside = true;
                    m_pointer_pos = e.position;
                    if(m_active_scrollbar_id)
                    {
                        update_scrollbar_from_pointer(e.position);
                    }
                    else if(m_active_table_resize_id)
                    {
                        update_table_resize_from_pointer(e.position);
                    }
                    else if(m_active_id)
                    {
                        update_input_text_selection_from_pointer(m_active_id, e.position);
                        update_float_node_from_pointer(m_active_id, e.position, &old_pos);
                    }
                }
                else if(e.type == GUIInputEventType::pointer_down)
                {
                    m_pointer_inside = true;
                    m_pointer_pos = e.position;
                    m_active_float_component = U32_MAX;
                    GUIID old_focused_id = m_focused_id;
                    if(e.button != GUIPointerButton::left)
                    {
                        GUIID target = hit_test(e.position);
                        if(target)
                        {
                            m_focused_id = target;
                            if(old_focused_id && old_focused_id != target)
                            {
                                input_text_clear_selection(get_or_create_persistent_state(old_focused_id));
                            }
                        }
                        continue;
                    }
                    GUIID dropdown_combo = 0;
                    i32 dropdown_item = -1;
                    if(hit_test_combo_dropdown(e.position, dropdown_combo, dropdown_item))
                    {
                        m_active_id = dropdown_combo;
                        m_focused_id = dropdown_combo;
                        if(old_focused_id && old_focused_id != dropdown_combo)
                        {
                            input_text_clear_selection(get_or_create_persistent_state(old_focused_id));
                        }
                        PersistentItemState& state = get_or_create_persistent_state(dropdown_combo);
                        state.pointer_down = true;
                        state.active = true;
                        state.focused = true;
                        continue;
                    }
                    if(m_open_combo_id)
                    {
                        GUIID target = hit_test(e.position);
                        if(target != m_open_combo_id)
                        {
                            close_combo_dropdowns_except(0);
                        }
                    }
                    GUIID scrollbar_id = 0;
                    bool scrollbar_vertical = false;
                    RectF scrollbar_thumb;
                    if(hit_test_scrollbar(e.position, scrollbar_id, scrollbar_vertical, scrollbar_thumb))
                    {
                        m_active_id = scrollbar_id;
                        m_focused_id = scrollbar_id;
                        if(old_focused_id && old_focused_id != scrollbar_id)
                        {
                            input_text_clear_selection(get_or_create_persistent_state(old_focused_id));
                        }
                        m_active_scrollbar_id = scrollbar_id;
                        m_active_scrollbar_vertical = scrollbar_vertical;
                        if(point_in_rect(e.position, scrollbar_thumb))
                        {
                            m_active_scrollbar_grab_offset = scrollbar_vertical ?
                                e.position.y - scrollbar_thumb.offset_y :
                                e.position.x - scrollbar_thumb.offset_x;
                        }
                        else
                        {
                            m_active_scrollbar_grab_offset = scrollbar_vertical ?
                                scrollbar_thumb.height * 0.5f :
                                scrollbar_thumb.width * 0.5f;
                        }
                        PersistentItemState& state = get_or_create_persistent_state(scrollbar_id);
                        state.pointer_down = true;
                        state.active = true;
                        state.focused = true;
                        update_scrollbar_from_pointer(e.position);
                        continue;
                    }
                    GUIID resize_table = 0;
                    bool resize_column = false;
                    u32 resize_index = U32_MAX;
                    if(hit_test_table_separator(e.position, resize_table, resize_column, resize_index))
                    {
                        m_active_id = resize_table;
                        m_focused_id = resize_table;
                        if(old_focused_id && old_focused_id != resize_table)
                        {
                            input_text_clear_selection(get_or_create_persistent_state(old_focused_id));
                        }
                        m_active_table_resize_id = resize_table;
                        m_active_table_resize_column = resize_column;
                        m_active_table_resize_index = resize_index;
                        PersistentItemState& state = get_or_create_persistent_state(resize_table);
                        state.pointer_down = true;
                        state.active = true;
                        state.focused = true;
                        continue;
                    }
                    GUIID target = hit_test(e.position);
                    m_active_id = target;
                    m_focused_id = target;
                    if(old_focused_id && old_focused_id != target)
                    {
                        input_text_clear_selection(get_or_create_persistent_state(old_focused_id));
                    }
                    if(target)
                    {
                        PersistentItemState& state = get_or_create_persistent_state(target);
                        state.pointer_down = true;
                        state.active = true;
                        state.focused = true;
                        GUINode* node = find_node(target);
                        if(node && node->kind == GUINodeKind::input_text && node->string_value)
                        {
                            usize cursor = 0;
                            input_text_cursor_from_pointer(target, e.position, cursor);
                            state.text_cursor = cursor;
                            state.text_select_anchor = cursor;
                            state.text_selecting = true;
                            state.text_cursor_blink_start = m_time;
                        }
                        if(node && (node->kind == GUINodeKind::slider_float || node->kind == GUINodeKind::drag_float))
                        {
                            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
                            {
                                if(m_submitted_desc.nodes[i].id == target)
                                {
                                    m_active_float_component = hit_test_float_component(*node, m_layouts[i].rect, e.position);
                                    break;
                                }
                            }
                        }
                        update_float_node_from_pointer(target, e.position);
                    }
                }
                else if(e.type == GUIInputEventType::pointer_up)
                {
                    m_pointer_inside = true;
                    m_pointer_pos = e.position;
                    if(e.button == GUIPointerButton::right)
                    {
                        GUIID target = hit_test(e.position);
                        if(target)
                        {
                            ItemResult& result = get_or_create_current_result(target);
                            result.states.insert_or_assign(Name("gui.right_clicked"), Any(true));
                            PersistentItemState& state = get_or_create_persistent_state(target);
                            state.last_right_click_time = m_time;
                        }
                        continue;
                    }
                    if(e.button != GUIPointerButton::left)
                    {
                        continue;
                    }
                    GUIID dropdown_combo = 0;
                    i32 dropdown_item = -1;
                    if(hit_test_combo_dropdown(e.position, dropdown_combo, dropdown_item))
                    {
                        if(dropdown_combo && dropdown_combo == m_active_id)
                        {
                            ItemResult& result = get_or_create_current_result(dropdown_combo);
                            result.states.insert_or_assign(Name("gui.clicked"), Any(true));
                            PersistentItemState& state = get_or_create_persistent_state(dropdown_combo);
                            bool dbl = (m_time - state.last_click_time) <= 0.4;
                            result.states.insert_or_assign(Name("gui.double_clicked"), Any(dbl));
                            state.last_click_time = m_time;
                            for(GUINode& node : m_submitted_desc.nodes)
                            {
                                if(node.id != dropdown_combo || node.kind != GUINodeKind::combo || !node.i32_value) continue;
                                if(dropdown_item >= 0 && (usize)dropdown_item < node.items.size())
                                {
                                    if(*node.i32_value != dropdown_item)
                                    {
                                        *node.i32_value = dropdown_item;
                                        result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
                                    }
                                }
                                break;
                            }
                            close_combo_dropdowns_except(0);
                        }
                        if(m_active_id)
                        {
                            PersistentItemState& state = get_or_create_persistent_state(m_active_id);
                            state.pointer_down = false;
                            state.active = false;
                        }
                        m_active_id = 0;
                        m_active_float_component = U32_MAX;
                        continue;
                    }
                    if(m_active_scrollbar_id)
                    {
                        PersistentItemState& state = get_or_create_persistent_state(m_active_scrollbar_id);
                        state.pointer_down = false;
                        state.active = false;
                        m_active_scrollbar_id = 0;
                        m_active_scrollbar_vertical = false;
                        m_active_scrollbar_grab_offset = 0.0f;
                        m_active_id = 0;
                        m_active_float_component = U32_MAX;
                        continue;
                    }
                    if(m_active_table_resize_id)
                    {
                        PersistentItemState& state = get_or_create_persistent_state(m_active_table_resize_id);
                        state.pointer_down = false;
                        state.active = false;
                        m_active_table_resize_id = 0;
                        m_active_table_resize_column = false;
                        m_active_table_resize_index = U32_MAX;
                        m_active_id = 0;
                        m_active_float_component = U32_MAX;
                        continue;
                    }
                    GUIID target = hit_test(e.position);
                    if(target && target == m_active_id)
                    {
                        ItemResult& result = get_or_create_current_result(target);
                        result.states.insert_or_assign(Name("gui.clicked"), Any(true));
                        PersistentItemState& state = get_or_create_persistent_state(target);
                        bool dbl = (m_time - state.last_click_time) <= 0.4;
                        result.states.insert_or_assign(Name("gui.double_clicked"), Any(dbl));
                        state.last_click_time = m_time;
                        for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
                        {
                            GUINode& node = m_submitted_desc.nodes[i];
                            if(node.id != target) continue;
                            if((node.kind == GUINodeKind::checkbox || node.kind == GUINodeKind::toggle_switch) && node.bool_value)
                            {
                                *node.bool_value = !*node.bool_value;
                                result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
                            }
                            else if(node.kind == GUINodeKind::collapsing_header)
                            {
                                state.open = !state.open;
                                result.states.insert_or_assign(Name("gui.open"), Any(state.open));
                                result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
                            }
                            else if(node.kind == GUINodeKind::combo && node.i32_value && !node.items.empty())
                            {
                                state.open = !state.open;
                                if(state.open)
                                {
                                    close_combo_dropdowns_except(node.id);
                                    state.open = true;
                                }
                                else if(m_open_combo_id == node.id)
                                {
                                    m_open_combo_id = 0;
                                }
                                result.states.insert_or_assign(Name("gui.open"), Any(state.open));
                            }
                            else if(window_has_title_bar(node) && node.bool_value)
                            {
                                RectF rect = m_layouts[i].rect;
                                if(point_in_rect(e.position, window_close_rect(rect)))
                                {
                                    *node.bool_value = false;
                                    result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
                                }
                            }
                            break;
                        }
                    }
                    if(m_active_id)
                    {
                        PersistentItemState& state = get_or_create_persistent_state(m_active_id);
                        state.pointer_down = false;
                        state.active = false;
                        state.text_selecting = false;
                        if(state.text_select_anchor == state.text_cursor)
                        {
                            state.text_select_anchor = USIZE_MAX;
                        }
                    }
                    m_active_id = 0;
                    m_active_float_component = U32_MAX;
                }
                else if(e.type == GUIInputEventType::pointer_wheel)
                {
                    m_pointer_inside = true;
                    m_pointer_pos = e.position;
                    GUIID dropdown_combo = 0;
                    i32 dropdown_item = -1;
                    if(hit_test_combo_dropdown(e.position, dropdown_combo, dropdown_item))
                    {
                        continue;
                    }
                    GUIID scroll_target = 0;
                    bool scrollbar_vertical = false;
                    RectF scrollbar_thumb;
                    if(!hit_test_scrollbar(e.position, scroll_target, scrollbar_vertical, scrollbar_thumb))
                    {
                        scroll_target = hit_test_node_kind(e.position, GUINodeKind::scroll_view);
                    }
                    if(scroll_target)
                    {
                        PersistentItemState& state = get_or_create_persistent_state(scroll_target);
                        f32 old_scroll_x = state.scroll_x;
                        f32 old_scroll_y = state.scroll_y;
                        state.scroll_x -= e.wheel_delta.x * 24.0f;
                        state.scroll_y -= e.wheel_delta.y * 24.0f;
                        clamp_scroll_state(scroll_target);
                        if(state.scroll_x != old_scroll_x || state.scroll_y != old_scroll_y)
                        {
                            ItemResult& result = get_or_create_current_result(scroll_target);
                            result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
                            m_layout_dirty = true;
                        }
                    }
                }
                else if(e.type == GUIInputEventType::text_utf8)
                {
                    if(!m_focused_id) continue;
                    for(GUINode& node : m_submitted_desc.nodes)
                    {
                        if(node.id == m_focused_id && node.kind == GUINodeKind::input_text && node.string_value)
                        {
                            String filtered = filter_input_text(e.text);
                            if(!filtered.empty())
                            {
                                PersistentItemState& state = get_or_create_persistent_state(node.id);
                                state.text_cursor = clamp_utf8_cursor(*node.string_value, state.text_cursor);
                                delete_input_text_selection(*node.string_value, state);
                                node.string_value->insert(state.text_cursor, filtered);
                                state.text_cursor += filtered.size();
                                input_text_clear_selection(state);
                                state.text_cursor_blink_start = m_time;
                                ItemResult& result = get_or_create_current_result(node.id);
                                result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
                            }
                            break;
                        }
                    }
                }
                else if(e.type == GUIInputEventType::key_down)
                {
                    if(!m_focused_id) continue;
                    for(GUINode& node : m_submitted_desc.nodes)
                    {
                        if(node.id != m_focused_id || node.kind != GUINodeKind::input_text || !node.string_value)
                        {
                            continue;
                        }
                        PersistentItemState& state = get_or_create_persistent_state(node.id);
                        state.text_cursor = clamp_utf8_cursor(*node.string_value, state.text_cursor);
                        bool changed = false;
                        bool shortcut = has_modifier(e.modifiers, GUIKeyModifierFlag::ctrl) || has_modifier(e.modifiers, GUIKeyModifierFlag::system);
                        bool shift = has_modifier(e.modifiers, GUIKeyModifierFlag::shift);
                        if(shortcut && e.key == GUIKey::c)
                        {
                            if(input_text_has_selection(*node.string_value, state) && m_clipboard_io.set_text)
                            {
                                usize begin = 0;
                                usize end = 0;
                                input_text_selection_range(*node.string_value, state, begin, end);
                                String selected = node.string_value->substr(begin, end - begin);
                                RV clipboard_result = m_clipboard_io.set_text(selected.c_str(), selected.size(), m_clipboard_io.userdata);
                                (void)clipboard_result;
                            }
                        }
                        else if(shortcut && e.key == GUIKey::v)
                        {
                            if(m_clipboard_io.get_text)
                            {
                                String clipboard_text;
                                RV r = m_clipboard_io.get_text(clipboard_text, m_clipboard_io.userdata);
                                if(succeeded(r))
                                {
                                    String filtered = filter_input_text(clipboard_text);
                                    if(!filtered.empty() || input_text_has_selection(*node.string_value, state))
                                    {
                                        delete_input_text_selection(*node.string_value, state);
                                        node.string_value->insert(state.text_cursor, filtered);
                                        state.text_cursor += filtered.size();
                                        input_text_clear_selection(state);
                                        state.text_cursor_blink_start = m_time;
                                        changed = true;
                                    }
                                }
                            }
                        }
                        else if(e.key == GUIKey::backspace)
                        {
                            if(input_text_has_selection(*node.string_value, state))
                            {
                                changed = delete_input_text_selection(*node.string_value, state);
                            }
                            else
                            {
                                usize old_size = node.string_value->size();
                                erase_previous_utf8_codepoint(*node.string_value, state.text_cursor);
                                changed = node.string_value->size() != old_size;
                            }
                            state.text_cursor_blink_start = m_time;
                        }
                        else if(e.key == GUIKey::del)
                        {
                            if(input_text_has_selection(*node.string_value, state))
                            {
                                changed = delete_input_text_selection(*node.string_value, state);
                            }
                            else
                            {
                                usize old_size = node.string_value->size();
                                erase_utf8_codepoint_at(*node.string_value, state.text_cursor);
                                changed = node.string_value->size() != old_size;
                            }
                            state.text_cursor_blink_start = m_time;
                        }
                        else if(e.key == GUIKey::left)
                        {
                            if(shift && state.text_select_anchor == USIZE_MAX)
                            {
                                state.text_select_anchor = state.text_cursor;
                            }
                            if(!shift && input_text_has_selection(*node.string_value, state))
                            {
                                usize begin = 0;
                                usize end = 0;
                                input_text_selection_range(*node.string_value, state, begin, end);
                                state.text_cursor = begin;
                                input_text_clear_selection(state);
                            }
                            else
                            {
                                state.text_cursor = previous_utf8_cursor(*node.string_value, state.text_cursor);
                                if(!shift) input_text_clear_selection(state);
                            }
                            state.text_cursor_blink_start = m_time;
                        }
                        else if(e.key == GUIKey::right)
                        {
                            if(shift && state.text_select_anchor == USIZE_MAX)
                            {
                                state.text_select_anchor = state.text_cursor;
                            }
                            if(!shift && input_text_has_selection(*node.string_value, state))
                            {
                                usize begin = 0;
                                usize end = 0;
                                input_text_selection_range(*node.string_value, state, begin, end);
                                state.text_cursor = end;
                                input_text_clear_selection(state);
                            }
                            else
                            {
                                state.text_cursor = next_utf8_cursor(*node.string_value, state.text_cursor);
                                if(!shift) input_text_clear_selection(state);
                            }
                            state.text_cursor_blink_start = m_time;
                        }
                        else if(e.key == GUIKey::enter || e.key == GUIKey::esc)
                        {
                            m_focused_id = 0;
                            state.focused = false;
                            input_text_clear_selection(state);
                        }
                        if(changed)
                        {
                            ItemResult& result = get_or_create_current_result(node.id);
                            result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
                        }
                        break;
                    }
                }
                else if(e.type == GUIInputEventType::blur)
                {
                    if(m_focused_id)
                    {
                        input_text_clear_selection(get_or_create_persistent_state(m_focused_id));
                    }
                    m_focused_id = 0;
                    m_active_id = 0;
                    m_active_float_component = U32_MAX;
                    m_active_table_resize_id = 0;
                    m_active_table_resize_column = false;
                    m_active_table_resize_index = U32_MAX;
                    m_active_scrollbar_id = 0;
                    m_active_scrollbar_vertical = false;
                    m_active_scrollbar_grab_offset = 0.0f;
                    close_combo_dropdowns_except(0);
                }
            }
            m_input_events.clear();

            if(m_pointer_inside)
            {
                GUIID combo_id = 0;
                i32 combo_item = -1;
                GUIID scrollbar_id = 0;
                bool scrollbar_vertical = false;
                RectF scrollbar_thumb;
                m_hovered_id = hit_test_combo_dropdown(m_pointer_pos, combo_id, combo_item) ?
                    combo_id :
                    (hit_test_scrollbar(m_pointer_pos, scrollbar_id, scrollbar_vertical, scrollbar_thumb) ?
                        scrollbar_id :
                        hit_test(m_pointer_pos));
            }
            else
            {
                m_hovered_id = 0;
            }
        }

        RV GUIContext::submit(const GUIDescription& desc)
        {
            lutsassert();
            lutry
            {
                m_submitted_desc = desc;
                m_layouts.clear();
                m_layouts.resize(m_submitted_desc.nodes.size());
                HashSet<GUIID> ids;
                bool open_combo_submitted = false;
                for(const GUINode& node : m_submitted_desc.nodes)
                {
                    if(!node.interactive) continue;
                    auto r = ids.insert(node.id);
                    luassert_msg(r.second, "Duplicate GUI item ID detected.");
                    ItemResult& result = get_or_create_current_result(node.id);
                    result.generation = m_generation;
                    result.states.insert_or_assign(Name("gui.clicked"), Any(false));
                    result.states.insert_or_assign(Name("gui.right_clicked"), Any(false));
                    result.states.insert_or_assign(Name("gui.double_clicked"), Any(false));
                    result.states.insert_or_assign(Name("gui.hovered"), Any(false));
                    result.states.insert_or_assign(Name("gui.active"), Any(false));
                    result.states.insert_or_assign(Name("gui.focused"), Any(false));
                    result.states.insert_or_assign(Name("gui.value_changed"), Any(false));
                    PersistentItemState& persistent = get_or_create_persistent_state(node.id);
                    if(node.kind == GUINodeKind::collapsing_header)
                    {
                        result.states.insert_or_assign(Name("gui.open"), Any(persistent.open));
                    }
                    else if(node.kind == GUINodeKind::combo)
                    {
                        if(node.id == m_open_combo_id)
                        {
                            open_combo_submitted = true;
                            persistent.open = true;
                        }
                        else
                        {
                            persistent.open = false;
                        }
                        result.states.insert_or_assign(Name("gui.open"), Any(persistent.open));
                    }
                    else if(node.kind == GUINodeKind::input_text && node.string_value)
                    {
                        persistent.text_cursor = clamp_utf8_cursor(*node.string_value, persistent.text_cursor);
                    }
                }
                if(m_open_combo_id && !open_combo_submitted)
                {
                    m_open_combo_id = 0;
                }
                RectF root_rect(0.0f, 0.0f, m_frame_desc.surface_size.x, m_frame_desc.surface_size.y);
                m_layout_dirty = false;
                layout_node(0, root_rect, root_rect);
                process_input_events();
                if(m_layout_dirty)
                {
                    for(NodeLayout& layout : m_layouts)
                    {
                        layout.metrics_valid = false;
                    }
                    layout_node(0, root_rect, root_rect);
                    if(m_pointer_inside)
                    {
                        GUIID combo_id = 0;
                        i32 combo_item = -1;
                        GUIID scrollbar_id = 0;
                        bool scrollbar_vertical = false;
                        RectF scrollbar_thumb;
                        m_hovered_id = hit_test_combo_dropdown(m_pointer_pos, combo_id, combo_item) ?
                            combo_id :
                            (hit_test_scrollbar(m_pointer_pos, scrollbar_id, scrollbar_vertical, scrollbar_thumb) ?
                                scrollbar_id :
                                hit_test(m_pointer_pos));
                    }
                }
                for(const GUINode& node : m_submitted_desc.nodes)
                {
                    if(!node.interactive) continue;
                    ItemResult& result = get_or_create_current_result(node.id);
                    PersistentItemState& persistent = get_or_create_persistent_state(node.id);
                    result.states.insert_or_assign(Name("gui.hovered"), Any(node.id == m_hovered_id));
                    result.states.insert_or_assign(Name("gui.active"), Any(node.id == m_active_id || persistent.active));
                    result.states.insert_or_assign(Name("gui.focused"), Any(node.id == m_focused_id));
                    if(node.kind == GUINodeKind::collapsing_header)
                    {
                        result.states.insert_or_assign(Name("gui.open"), Any(persistent.open));
                    }
                    else if(node.kind == GUINodeKind::combo)
                    {
                        result.states.insert_or_assign(Name("gui.open"), Any(persistent.open));
                    }
                }
                m_submitted = true;
            }
            lucatchret;
            return ok;
        }
    }
}
