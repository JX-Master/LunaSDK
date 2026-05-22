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

        GUIID GUIContext::hit_test(const Float2U& pos) const
        {
            GUIID ret = 0;
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                const GUINode& node = m_submitted_desc.nodes[i];
                if(!node.interactive) continue;
                const RectF& rect = m_layouts[i].rect;
                const RectF& clip = m_layouts[i].clip_rect;
                if(point_in_rect(pos, rect) && point_in_rect(pos, clip))
                {
                    ret = node.id;
                }
            }
            return ret;
        }

        GUIID GUIContext::hit_test_node_kind(const Float2U& pos, GUINodeKind kind) const
        {
            GUIID ret = 0;
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                const GUINode& node = m_submitted_desc.nodes[i];
                if(node.kind != kind) continue;
                const RectF& rect = m_layouts[i].rect;
                const RectF& clip = m_layouts[i].clip_rect;
                if(point_in_rect(pos, rect) && point_in_rect(pos, clip))
                {
                    ret = node.id;
                }
            }
            return ret;
        }

        GUINode* GUIContext::find_node(GUIID id)
        {
            for(GUINode& node : m_submitted_desc.nodes)
            {
                if(node.id == id) return &node;
            }
            return nullptr;
        }

        void GUIContext::update_float_node_from_pointer(GUIID id, const Float2U& pos)
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
            f32 track_x = rect.offset_x + label_w;
            f32 track_w = max(rect.width - label_w - 8.0f, 1.0f);
            f32 t = clamp((pos.x - track_x) / track_w, 0.0f, 1.0f);
            f32 new_value = node->min_value + (node->max_value - node->min_value) * t;
            if(*node->f32_value != new_value)
            {
                *node->f32_value = new_value;
                ItemResult& result = get_or_create_current_result(id);
                result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
            }
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
                    m_pointer_inside = true;
                    m_pointer_pos = e.position;
                    if(m_active_table_resize_id)
                    {
                        update_table_resize_from_pointer(e.position);
                    }
                    else if(m_active_id)
                    {
                        update_float_node_from_pointer(m_active_id, e.position);
                    }
                }
                else if(e.type == GUIInputEventType::pointer_down)
                {
                    m_pointer_inside = true;
                    m_pointer_pos = e.position;
                    GUIID resize_table = 0;
                    bool resize_column = false;
                    u32 resize_index = U32_MAX;
                    if(hit_test_table_separator(e.position, resize_table, resize_column, resize_index))
                    {
                        m_active_id = resize_table;
                        m_focused_id = resize_table;
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
                    if(target)
                    {
                        PersistentItemState& state = get_or_create_persistent_state(target);
                        state.pointer_down = true;
                        state.active = true;
                        state.focused = true;
                        update_float_node_from_pointer(target, e.position);
                    }
                }
                else if(e.type == GUIInputEventType::pointer_up)
                {
                    m_pointer_inside = true;
                    m_pointer_pos = e.position;
                    if(m_active_table_resize_id)
                    {
                        PersistentItemState& state = get_or_create_persistent_state(m_active_table_resize_id);
                        state.pointer_down = false;
                        state.active = false;
                        m_active_table_resize_id = 0;
                        m_active_table_resize_column = false;
                        m_active_table_resize_index = U32_MAX;
                        m_active_id = 0;
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
                            if(node.kind == GUINodeKind::checkbox && node.bool_value)
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
                                *node.i32_value = (*node.i32_value + 1) % (i32)node.items.size();
                                result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
                            }
                            break;
                        }
                    }
                    if(m_active_id)
                    {
                        PersistentItemState& state = get_or_create_persistent_state(m_active_id);
                        state.pointer_down = false;
                        state.active = false;
                    }
                    m_active_id = 0;
                }
                else if(e.type == GUIInputEventType::pointer_wheel)
                {
                    m_pointer_inside = true;
                    m_pointer_pos = e.position;
                    GUIID scroll_target = hit_test_node_kind(e.position, GUINodeKind::scroll_view);
                    if(scroll_target)
                    {
                        PersistentItemState& state = get_or_create_persistent_state(scroll_target);
                        state.scroll_y = max(0.0f, state.scroll_y - e.wheel_delta.y * 24.0f);
                        ItemResult& result = get_or_create_current_result(scroll_target);
                        result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
                    }
                }
                else if(e.type == GUIInputEventType::text_utf8)
                {
                    if(!m_focused_id) continue;
                    for(GUINode& node : m_submitted_desc.nodes)
                    {
                        if(node.id == m_focused_id && node.kind == GUINodeKind::input_text && node.string_value)
                        {
                            node.string_value->append(e.text);
                            ItemResult& result = get_or_create_current_result(node.id);
                            result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
                            break;
                        }
                    }
                }
                else if(e.type == GUIInputEventType::key_down)
                {
                    if(e.key == GUIKey::backspace && m_focused_id)
                    {
                        for(GUINode& node : m_submitted_desc.nodes)
                        {
                            if(node.id == m_focused_id && node.kind == GUINodeKind::input_text && node.string_value && !node.string_value->empty())
                            {
                                pop_utf8_codepoint(*node.string_value);
                                ItemResult& result = get_or_create_current_result(node.id);
                                result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
                                break;
                            }
                        }
                    }
                }
                else if(e.type == GUIInputEventType::blur)
                {
                    m_focused_id = 0;
                    m_active_id = 0;
                    m_active_table_resize_id = 0;
                    m_active_table_resize_column = false;
                    m_active_table_resize_index = U32_MAX;
                }
            }
            m_input_events.clear();

            if(m_pointer_inside)
            {
                m_hovered_id = hit_test(m_pointer_pos);
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
                for(const GUINode& node : m_submitted_desc.nodes)
                {
                    if(!node.interactive) continue;
                    auto r = ids.insert(node.id);
                    luassert_msg(r.second, "Duplicate GUI item ID detected.");
                    ItemResult& result = get_or_create_current_result(node.id);
                    result.generation = m_generation;
                    result.states.insert_or_assign(Name("gui.clicked"), Any(false));
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
                        m_hovered_id = hit_test(m_pointer_pos);
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
                }
                m_submitted = true;
            }
            lucatchret;
            return ok;
        }
    }
}
