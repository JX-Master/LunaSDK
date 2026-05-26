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

        DockPanelPersistentState* GUIContext::find_dock_panel_state(GUIID dock_space_id, GUIID panel_id)
        {
            auto dock_iter = m_persistent_states.find(dock_space_id);
            if(dock_iter == m_persistent_states.end()) return nullptr;
            auto panel_iter = dock_iter->second.dock_panels.find(panel_id);
            return panel_iter == dock_iter->second.dock_panels.end() ? nullptr : &panel_iter->second;
        }

        void GUIContext::raise_dock_panel(GUIID dock_space_id, GUIID panel_id)
        {
            if(!dock_space_id || !panel_id) return;
            PersistentItemState& dock_state = get_or_create_persistent_state(dock_space_id);
            DockPanelPersistentState& panel_state = get_or_create_dock_panel_state(dock_state, panel_id);
            if(panel_state.mode != GUIDockPanelMode::floating) return;
            panel_state.z_order = dock_state.dock_next_z_order++;
        }

        bool GUIContext::hit_test_dock_panel(const Float2U& pos, GUIID& out_space_id, GUIID& out_panel_id) const
        {
            bool found = false;
            u32 best_z = 0;
            if(m_layouts.size() != m_submitted_desc.nodes.size()) return false;
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                const GUINode& dock_node = m_submitted_desc.nodes[i];
                if(dock_node.kind != GUINodeKind::dock_space) continue;
                if(!point_in_rect(pos, m_layouts[i].rect) || !point_in_rect(pos, m_layouts[i].clip_rect)) continue;
                for(u32 child = dock_node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
                {
                    const NodeLayout& layout = m_layouts[child];
                    if(!layout.dock_panel_child || !layout.dock_panel_visible) continue;
                    if(!point_in_rect(pos, layout.dock_panel_rect) || !point_in_rect(pos, layout.dock_panel_clip_rect)) continue;
                    u32 z = layout.dock_panel_floating ? layout.dock_panel_z_order : 0;
                    if(!found || z >= best_z)
                    {
                        found = true;
                        best_z = z;
                        out_space_id = dock_node.id;
                        out_panel_id = m_submitted_desc.nodes[child].id;
                    }
                }
            }
            return found;
        }

        bool GUIContext::hit_test_dock_panel_chrome(const Float2U& pos, GUIID& out_space_id, GUIID& out_panel_id, bool& out_resize, bool& out_close) const
        {
            bool found = false;
            u32 best_z = 0;
            out_resize = false;
            out_close = false;
            if(m_layouts.size() != m_submitted_desc.nodes.size()) return false;
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                const GUINode& dock_node = m_submitted_desc.nodes[i];
                if(dock_node.kind != GUINodeKind::dock_space) continue;
                if(!point_in_rect(pos, m_layouts[i].rect) || !point_in_rect(pos, m_layouts[i].clip_rect)) continue;
                for(u32 child = dock_node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
                {
                    const NodeLayout& layout = m_layouts[child];
                    if(!layout.dock_panel_child || !layout.dock_panel_visible) continue;
                    if(!point_in_rect(pos, layout.dock_panel_rect) || !point_in_rect(pos, layout.dock_panel_clip_rect)) continue;
                    bool close_hit = layout.dock_panel_style.close_button && point_in_rect(pos, layout.dock_panel_close_rect);
                    bool resize_hit = layout.dock_panel_style.resize_border && point_in_rect(pos, layout.dock_panel_resize_rect);
                    bool title_hit = layout.dock_panel_style.title_bar && point_in_rect(pos, layout.dock_panel_title_rect);
                    if(!close_hit && !resize_hit && !title_hit) continue;
                    u32 z = layout.dock_panel_floating ? layout.dock_panel_z_order : 0;
                    if(!found || z >= best_z)
                    {
                        found = true;
                        best_z = z;
                        out_space_id = dock_node.id;
                        out_panel_id = m_submitted_desc.nodes[child].id;
                        out_close = close_hit;
                        out_resize = !close_hit && resize_hit;
                    }
                }
            }
            return found;
        }

        bool GUIContext::hit_test_dock_panel_tab(const Float2U& pos, GUIID& out_space_id, GUIID& out_panel_id, u32& out_leaf_index) const
        {
            bool found = false;
            u32 best_z = 0;
            if(m_layouts.size() != m_submitted_desc.nodes.size()) return false;
            GUIID top_space = 0;
            GUIID top_panel = 0;
            if(hit_test_dock_panel(pos, top_space, top_panel))
            {
                for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
                {
                    if(m_submitted_desc.nodes[i].id == top_panel && m_layouts[i].dock_panel_floating)
                    {
                        return false;
                    }
                }
            }
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                const GUINode& dock_node = m_submitted_desc.nodes[i];
                if(dock_node.kind != GUINodeKind::dock_space) continue;
                auto dock_state_iter = m_persistent_states.find(dock_node.id);
                if(dock_state_iter == m_persistent_states.end()) continue;
                const PersistentItemState& dock_state = dock_state_iter->second;
                if(!point_in_rect(pos, m_layouts[i].rect) || !point_in_rect(pos, m_layouts[i].clip_rect)) continue;
                for(u32 child = dock_node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
                {
                    const NodeLayout& layout = m_layouts[child];
                    if(!layout.dock_panel_child || !layout.dock_panel_visible || layout.dock_panel_floating) continue;
                    if(layout.dock_leaf_index >= dock_state.dock_nodes.size()) continue;
                    const DockTreeNode& leaf = dock_state.dock_nodes[layout.dock_leaf_index];
                    if(leaf.split || leaf.tabs.empty()) continue;
                    for(usize tab_index = 0; tab_index < leaf.tabs.size(); ++tab_index)
                    {
                        RectF tab_rect = dock_panel_tab_rect(layout.dock_panel_title_rect, tab_index, leaf.tabs.size(), layout.dock_panel_style.close_button);
                        if(!point_in_rect(pos, tab_rect)) continue;
                        u32 z = layout.dock_panel_z_order;
                        if(!found || z >= best_z)
                        {
                            found = true;
                            best_z = z;
                            out_space_id = dock_node.id;
                            out_panel_id = leaf.tabs[tab_index];
                            out_leaf_index = layout.dock_leaf_index;
                        }
                    }
                }
            }
            return found;
        }

        bool GUIContext::hit_test_dock_splitter(const Float2U& pos, GUIID& out_space_id, u32& out_node_index, GUIDockSplitAxis& out_axis) const
        {
            if(m_layouts.size() != m_submitted_desc.nodes.size()) return false;
            GUIID top_space = 0;
            GUIID top_panel = 0;
            if(hit_test_dock_panel(pos, top_space, top_panel))
            {
                for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
                {
                    if(m_submitted_desc.nodes[i].id == top_panel && m_layouts[i].dock_panel_floating)
                    {
                        return false;
                    }
                }
            }
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                const GUINode& dock_node = m_submitted_desc.nodes[i];
                if(dock_node.kind != GUINodeKind::dock_space) continue;
                auto dock_state_iter = m_persistent_states.find(dock_node.id);
                if(dock_state_iter == m_persistent_states.end()) continue;
                const PersistentItemState& dock_state = dock_state_iter->second;
                if(!point_in_rect(pos, m_layouts[i].rect) || !point_in_rect(pos, m_layouts[i].clip_rect)) continue;
                if(dock_state.dock_root_node == U32_MAX || dock_state.dock_root_node >= dock_state.dock_nodes.size()) continue;
                Vector<u32> stack;
                stack.push_back(dock_state.dock_root_node);
                while(!stack.empty())
                {
                    u32 node_index = stack.back();
                    stack.pop_back();
                    if(node_index >= dock_state.dock_nodes.size()) continue;
                    const DockTreeNode& tree_node = dock_state.dock_nodes[node_index];
                    if(!tree_node.split) continue;
                    stack.push_back(tree_node.child1);
                    stack.push_back(tree_node.child0);
                    if(!point_in_rect(pos, tree_node.split_rect)) continue;
                    out_space_id = dock_node.id;
                    out_node_index = (u32)node_index;
                    out_axis = tree_node.split_axis;
                    return true;
                }
            }
            return false;
        }

        void GUIContext::update_dock_splitter_from_pointer(const Float2U& pos)
        {
            if(!m_active_dock_split_space_id || m_active_dock_split_node == U32_MAX) return;
            PersistentItemState& dock_state = get_or_create_persistent_state(m_active_dock_split_space_id);
            if(m_active_dock_split_node >= dock_state.dock_nodes.size()) return;
            DockTreeNode& tree_node = dock_state.dock_nodes[m_active_dock_split_node];
            if(!tree_node.split) return;
            f32 splitter_size = dock_panel_splitter_size();
            f32 axis_size = tree_node.split_axis == GUIDockSplitAxis::x ? tree_node.rect.width : tree_node.rect.height;
            f32 available = max(axis_size - splitter_size, 1.0f);
            f32 delta = tree_node.split_axis == GUIDockSplitAxis::x ? pos.x - m_active_dock_split_start_pos.x : pos.y - m_active_dock_split_start_pos.y;
            f32 ratio = m_active_dock_split_start_ratio + delta / available;
            tree_node.split_ratio = clamp(ratio, 0.08f, 0.92f);
            m_layout_dirty = true;
        }

        bool GUIContext::find_dock_drop_target(GUIID payload_panel, const Float2U& pos, GUIID& out_space_id, u32& out_leaf_index, GUIDockDropDirection& out_direction) const
        {
            if(m_layouts.size() != m_submitted_desc.nodes.size()) return false;
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                const GUINode& dock_node = m_submitted_desc.nodes[i];
                if(dock_node.kind != GUINodeKind::dock_space) continue;
                auto dock_state_iter = m_persistent_states.find(dock_node.id);
                if(dock_state_iter == m_persistent_states.end()) continue;
                const PersistentItemState& dock_state = dock_state_iter->second;
                if(!point_in_rect(pos, m_layouts[i].rect) || !point_in_rect(pos, m_layouts[i].clip_rect)) continue;
                if(dock_state.dock_root_node == U32_MAX || dock_state.dock_root_node >= dock_state.dock_nodes.size())
                {
                    out_space_id = dock_node.id;
                    out_leaf_index = U32_MAX;
                    out_direction = point_in_rect(pos, dock_drop_icon_rect(m_layouts[i].rect, GUIDockDropDirection::center)) ?
                        GUIDockDropDirection::center :
                        GUIDockDropDirection::none;
                    return true;
                }
                Vector<u32> stack;
                stack.push_back(dock_state.dock_root_node);
                while(!stack.empty())
                {
                    u32 node_index = stack.back();
                    stack.pop_back();
                    if(node_index >= dock_state.dock_nodes.size()) continue;
                    const DockTreeNode& leaf = dock_state.dock_nodes[node_index];
                    if(leaf.split)
                    {
                        stack.push_back(leaf.child1);
                        stack.push_back(leaf.child0);
                        continue;
                    }
                    if(leaf.tabs.empty()) continue;
                    if(!point_in_rect(pos, leaf.rect)) continue;
                    bool payload_is_only_tab = leaf.tabs.size() == 1 && leaf.tabs[0] == payload_panel;
                    if(payload_is_only_tab) continue;
                    static const GUIDockDropDirection directions[] = {
                        GUIDockDropDirection::center,
                        GUIDockDropDirection::left,
                        GUIDockDropDirection::right,
                        GUIDockDropDirection::up,
                        GUIDockDropDirection::down
                    };
                    for(GUIDockDropDirection direction : directions)
                    {
                        if(point_in_rect(pos, dock_drop_icon_rect(leaf.rect, direction)))
                        {
                            out_space_id = dock_node.id;
                            out_leaf_index = (u32)node_index;
                            out_direction = direction;
                            return true;
                        }
                    }
                    out_space_id = dock_node.id;
                    out_leaf_index = (u32)node_index;
                    out_direction = GUIDockDropDirection::none;
                    return true;
                }
            }
            return false;
        }

        void GUIContext::update_dock_panel_from_pointer(const Float2U& pos)
        {
            if(!m_active_dock_space_id || !m_active_dock_panel_id) return;
            DockPanelPersistentState* panel_state = find_dock_panel_state(m_active_dock_space_id, m_active_dock_panel_id);
            if(!panel_state) return;
            if(m_active_dock_panel_close) return;
            if(m_active_dock_panel_title_drag && !m_active_dock_panel_was_floating && !m_active_dock_panel_undocked)
            {
                RectF release_rect = m_active_dock_panel_start_title_rect;
                release_rect.offset_x -= 8.0f;
                release_rect.offset_y -= 8.0f;
                release_rect.width += 16.0f;
                release_rect.height += 16.0f;
                if(point_in_rect(pos, release_rect))
                {
                    return;
                }
                PersistentItemState& dock_state = get_or_create_persistent_state(m_active_dock_space_id);
                dock_tree_remove_panel(dock_state, m_active_dock_panel_id);
                panel_state->mode = GUIDockPanelMode::floating;
                panel_state->rect = m_active_dock_panel_restore_rect;
                panel_state->rect.width = max(panel_state->rect.width, 1.0f);
                panel_state->rect.height = max(panel_state->rect.height, 1.0f);
                panel_state->z_order = dock_state.dock_next_z_order++;
                m_active_dock_panel_start_rect = panel_state->rect;
                m_active_dock_panel_grab_offset.x = clamp(m_active_dock_panel_grab_offset.x, 8.0f, max(panel_state->rect.width - 8.0f, 8.0f));
                m_active_dock_panel_grab_offset.y = clamp(m_active_dock_panel_grab_offset.y, 4.0f, max(panel_state->rect.height - 4.0f, 4.0f));
                m_active_dock_panel_undocked = true;
            }
            if(m_active_dock_panel_resize && !m_active_dock_panel_was_floating)
            {
                DockPanelPersistentState* neighbor_state = find_dock_panel_state(m_active_dock_space_id, m_active_dock_panel_resize_neighbor_id);
                if(!neighbor_state) return;
                f32 active_min_height = 32.0f;
                f32 neighbor_min_height = 32.0f;
                for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
                {
                    const GUINode& node = m_submitted_desc.nodes[i];
                    if(node.id == m_active_dock_panel_id)
                    {
                        active_min_height = dock_panel_min_height(m_layouts[i].dock_panel_style);
                    }
                    else if(node.id == m_active_dock_panel_resize_neighbor_id)
                    {
                        neighbor_min_height = dock_panel_min_height(m_layouts[i].dock_panel_style);
                    }
                }
                f32 total_height = max(m_active_dock_panel_start_rect.height + m_active_dock_panel_start_neighbor_height, 1.0f);
                f32 delta = pos.y - (m_active_dock_panel_start_rect.offset_y + m_active_dock_panel_start_rect.height);
                f32 active_height = m_active_dock_panel_start_rect.height + delta;
                if(total_height <= active_min_height + neighbor_min_height)
                {
                    active_height = total_height * active_min_height / max(active_min_height + neighbor_min_height, 1.0f);
                }
                else
                {
                    active_height = clamp(active_height, active_min_height, total_height - neighbor_min_height);
                }
                panel_state->docking_height = active_height;
                neighbor_state->docking_height = max(total_height - active_height, 1.0f);
                m_layout_dirty = true;
                return;
            }

            panel_state->mode = GUIDockPanelMode::floating;
            if(m_active_dock_panel_resize)
            {
                f32 min_width = 1.0f;
                f32 min_height = 1.0f;
                for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
                {
                    if(m_submitted_desc.nodes[i].id == m_active_dock_panel_id)
                    {
                        min_width = max(m_layouts[i].dock_panel_style.min_floating_size.x, 1.0f);
                        min_height = dock_panel_min_height(m_layouts[i].dock_panel_style);
                        break;
                    }
                }
                panel_state->rect = m_active_dock_panel_start_rect;
                panel_state->rect.width = max(pos.x - m_active_dock_panel_start_rect.offset_x, min_width);
                panel_state->rect.height = max(pos.y - m_active_dock_panel_start_rect.offset_y, min_height);
            }
            else if(!m_active_dock_panel_close)
            {
                panel_state->rect.offset_x = pos.x - m_active_dock_panel_grab_offset.x;
                panel_state->rect.offset_y = pos.y - m_active_dock_panel_grab_offset.y;
                panel_state->rect.width = m_active_dock_panel_start_rect.width;
                panel_state->rect.height = m_active_dock_panel_start_rect.height;
            }
            m_layout_dirty = true;
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

        bool GUIContext::hit_test_tab_header(const Float2U& pos, GUIID& out_tab_bar_id, GUIID& out_tab_item_id, bool& out_close) const
        {
            if(m_layouts.size() != m_submitted_desc.nodes.size()) return false;
            for(usize i = m_submitted_desc.nodes.size(); i > 0; --i)
            {
                u32 node_index = (u32)(i - 1);
                const GUINode& node = m_submitted_desc.nodes[node_index];
                if(node.kind != GUINodeKind::tab_item) continue;
                const NodeLayout& layout = m_layouts[node_index];
                if(layout.tab_header_rect.width <= 0.0f || layout.tab_header_rect.height <= 0.0f) continue;
                if(!point_in_rect(pos, layout.tab_header_rect) || !point_in_rect(pos, layout.tab_header_clip_rect)) continue;
                if(layout.dock_panel_child && !layout.dock_panel_visible) continue;
                out_tab_item_id = node.id;
                out_tab_bar_id = 0;
                if(node.parent != U32_MAX && node.parent < m_submitted_desc.nodes.size())
                {
                    out_tab_bar_id = m_submitted_desc.nodes[node.parent].id;
                }
                out_close = layout.tab_close_rect.width > 0.0f && point_in_rect(pos, layout.tab_close_rect);
                return out_tab_bar_id != 0;
            }
            return false;
        }

        bool GUIContext::hit_test_tab_scroll_button(const Float2U& pos, GUIID& out_tab_bar_id, bool& out_left) const
        {
            if(m_layouts.size() != m_submitted_desc.nodes.size()) return false;
            for(usize i = m_submitted_desc.nodes.size(); i > 0; --i)
            {
                u32 node_index = (u32)(i - 1);
                const GUINode& node = m_submitted_desc.nodes[node_index];
                if(node.kind != GUINodeKind::tab_bar) continue;
                const NodeLayout& layout = m_layouts[node_index];
                if(!layout.tab_scrollable) continue;
                if(point_in_rect(pos, layout.tab_scroll_left_rect) && point_in_rect(pos, layout.clip_rect))
                {
                    out_tab_bar_id = node.id;
                    out_left = true;
                    return true;
                }
                if(point_in_rect(pos, layout.tab_scroll_right_rect) && point_in_rect(pos, layout.clip_rect))
                {
                    out_tab_bar_id = node.id;
                    out_left = false;
                    return true;
                }
            }
            return false;
        }

        GUIID GUIContext::hit_test_tab_scroll_area(const Float2U& pos) const
        {
            if(m_layouts.size() != m_submitted_desc.nodes.size()) return 0;
            for(usize i = m_submitted_desc.nodes.size(); i > 0; --i)
            {
                u32 node_index = (u32)(i - 1);
                const GUINode& node = m_submitted_desc.nodes[node_index];
                if(node.kind != GUINodeKind::tab_bar) continue;
                const NodeLayout& layout = m_layouts[node_index];
                if(!layout.tab_scrollable) continue;
                if((point_in_rect(pos, layout.tab_header_area_rect) ||
                    point_in_rect(pos, layout.tab_scroll_left_rect) ||
                    point_in_rect(pos, layout.tab_scroll_right_rect)) &&
                    point_in_rect(pos, layout.clip_rect))
                {
                    return node.id;
                }
            }
            return 0;
        }

        GUIID GUIContext::fallback_tab_item(GUIID tab_bar_id, GUIID excluded_tab_item_id) const
        {
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                const GUINode& tab_bar = m_submitted_desc.nodes[i];
                if(tab_bar.id != tab_bar_id || tab_bar.kind != GUINodeKind::tab_bar) continue;
                GUIID fallback = 0;
                for(u32 child = tab_bar.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
                {
                    const GUINode& tab = m_submitted_desc.nodes[child];
                    if(tab.kind != GUINodeKind::tab_item || tab.id == excluded_tab_item_id) continue;
                    if(test_flags(tab.tab_item_flags, GUITabItemFlag::button)) continue;
                    if(tab.bool_value && !*tab.bool_value) continue;
                    fallback = tab.id;
                    break;
                }
                return fallback;
            }
            return 0;
        }

        void GUIContext::select_tab_item(GUIID tab_bar_id, GUIID tab_item_id)
        {
            if(!tab_bar_id || !tab_item_id) return;
            PersistentItemState& state = get_or_create_persistent_state(tab_bar_id);
            if(state.tab_selected_id == tab_item_id) return;
            state.tab_selected_id = tab_item_id;
            ItemResult& result = get_or_create_current_result(tab_bar_id);
            result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
        }

        bool GUIContext::reorder_tab_item_from_pointer(GUIID tab_bar_id, GUIID tab_item_id, const Float2U& pos)
        {
            if(!tab_bar_id || !tab_item_id) return false;
            PersistentItemState& state = get_or_create_persistent_state(tab_bar_id);
            usize old_index = USIZE_MAX;
            for(usize i = 0; i < state.tab_order.size(); ++i)
            {
                if(state.tab_order[i] == tab_item_id)
                {
                    old_index = i;
                    break;
                }
            }
            if(old_index == USIZE_MAX) return false;

            usize new_index = state.tab_order.size();
            for(usize i = 0; i < state.tab_order.size(); ++i)
            {
                GUIID id = state.tab_order[i];
                if(id == tab_item_id) continue;
                for(usize node_index = 0; node_index < m_submitted_desc.nodes.size(); ++node_index)
                {
                    const GUINode& node = m_submitted_desc.nodes[node_index];
                    if(node.id != id || node.kind != GUINodeKind::tab_item) continue;
                    if(node.parent == U32_MAX || node.parent >= m_submitted_desc.nodes.size() ||
                        m_submitted_desc.nodes[node.parent].id != tab_bar_id) break;
                    const RectF& rect = m_layouts[node_index].tab_header_rect;
                    if(pos.x < rect.offset_x + rect.width * 0.5f)
                    {
                        new_index = i;
                    }
                    break;
                }
                if(new_index != state.tab_order.size()) break;
            }
            if(new_index > old_index) --new_index;
            new_index = min(new_index, state.tab_order.size() - 1);
            if(new_index == old_index) return false;
            state.tab_order.erase(state.tab_order.begin() + old_index);
            state.tab_order.insert(state.tab_order.begin() + new_index, tab_item_id);
            ItemResult& result = get_or_create_current_result(tab_bar_id);
            result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
            return true;
        }

        void GUIContext::scroll_tab_bar(GUIID tab_bar_id, f32 delta)
        {
            if(!tab_bar_id || delta == 0.0f) return;
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                const GUINode& node = m_submitted_desc.nodes[i];
                if(node.id != tab_bar_id || node.kind != GUINodeKind::tab_bar) continue;
                const NodeLayout& layout = m_layouts[i];
                if(!layout.tab_scrollable) return;
                PersistentItemState& state = get_or_create_persistent_state(tab_bar_id);
                f32 old_scroll = state.tab_scroll_x;
                state.tab_scroll_x = clamp(state.tab_scroll_x + delta, 0.0f, layout.tab_scroll_max);
                if(state.tab_scroll_x != old_scroll)
                {
                    ItemResult& result = get_or_create_current_result(tab_bar_id);
                    result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
                    m_layout_dirty = true;
                }
                return;
            }
        }

        GUIID GUIContext::hit_test_node(u32 node_index, const Float2U& pos, bool filter_kind, GUINodeKind kind) const
        {
            GUIID ret = 0;
            const GUINode& node = m_submitted_desc.nodes[node_index];
            if(node.kind == GUINodeKind::popup && !popup_node_visible(node))
            {
                return 0;
            }
            const RectF& rect = m_layouts[node_index].rect;
            const RectF& clip = m_layouts[node_index].clip_rect;
            if(node.kind == GUINodeKind::tab_item)
            {
                const NodeLayout& layout = m_layouts[node_index];
                if((filter_kind ? node.kind == kind : node.interactive) &&
                    point_in_rect(pos, layout.tab_header_rect) &&
                    point_in_rect(pos, layout.tab_header_clip_rect))
                {
                    ret = node.id;
                }
                if(!layout.tab_content_visible)
                {
                    return ret;
                }
            }
            else if((filter_kind ? node.kind == kind : node.interactive) && point_in_rect(pos, rect) && point_in_rect(pos, clip))
            {
                ret = node.id;
            }
            if(node.kind == GUINodeKind::dock_space)
            {
                Vector<u32> floating_children;
                for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
                {
                    if(!m_layouts[child].dock_panel_visible) continue;
                    if(m_layouts[child].dock_panel_floating)
                    {
                        floating_children.push_back(child);
                    }
                    else
                    {
                        GUIID child_hit = hit_test_node(child, pos, filter_kind, kind);
                        if(child_hit) ret = child_hit;
                    }
                }
                for(usize i = 0; i < floating_children.size(); ++i)
                {
                    for(usize j = i + 1; j < floating_children.size(); ++j)
                    {
                        if(m_layouts[floating_children[j]].dock_panel_z_order < m_layouts[floating_children[i]].dock_panel_z_order)
                        {
                            u32 tmp = floating_children[i];
                            floating_children[i] = floating_children[j];
                            floating_children[j] = tmp;
                        }
                    }
                }
                for(u32 child : floating_children)
                {
                    GUIID child_hit = hit_test_node(child, pos, filter_kind, kind);
                    if(child_hit) ret = child_hit;
                }
                return ret;
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

        GUIID GUIContext::hit_test_drag_drop_source(const Float2U& pos, Name& out_type) const
        {
            if(m_layouts.size() != m_submitted_desc.nodes.size()) return 0;
            for(usize i = m_submitted_desc.nodes.size(); i > 0; --i)
            {
                u32 node_index = (u32)(i - 1);
                const GUINode& node = m_submitted_desc.nodes[node_index];
                if(node.drag_drop_source_types.empty()) continue;
                const NodeLayout& layout = m_layouts[node_index];
                if(layout.dock_panel_child && !layout.dock_panel_visible) continue;
                if(!point_in_rect(pos, layout.rect) || !point_in_rect(pos, layout.clip_rect)) continue;
                out_type = node.drag_drop_source_types[0];
                return node.id;
            }
            return 0;
        }

        GUIID GUIContext::hit_test_drag_drop_target(const Name& type, const Float2U& pos) const
        {
            if(!type || m_layouts.size() != m_submitted_desc.nodes.size()) return 0;
            GUIID best = 0;
            f32 best_area = F32_MAX;
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                const GUINode& node = m_submitted_desc.nodes[i];
                if(!contains_name(node.drag_drop_target_types, type)) continue;
                if(node.id == m_drag_drop_source_id) continue;
                const NodeLayout& layout = m_layouts[i];
                if(layout.dock_panel_child && !layout.dock_panel_visible) continue;
                if(!point_in_rect(pos, layout.rect) || !point_in_rect(pos, layout.clip_rect)) continue;
                f32 area = max(layout.rect.width, 1.0f) * max(layout.rect.height, 1.0f);
                if(area < best_area)
                {
                    best = node.id;
                    best_area = area;
                }
            }
            return best;
        }

        void GUIContext::start_drag_drop(GUIID source_id, const Name& type)
        {
            if(!source_id || !type) return;
            m_drag_drop_active = true;
            m_drag_drop_source_id = source_id;
            m_drag_drop_type = type;
            m_drag_drop_payload_set = false;
            m_drag_drop_payload_data.clear();
            ItemResult& result = get_or_create_current_result(source_id);
            result.states.insert_or_assign(Name("gui.active"), Any(true));
        }

        void GUIContext::clear_drag_drop()
        {
            m_drag_drop_candidate_source_id = 0;
            m_drag_drop_candidate_type.reset();
            m_drag_drop_active = false;
            m_drag_drop_payload_set = false;
            m_drag_drop_source_id = 0;
            m_drag_drop_type.reset();
            m_drag_drop_payload_data.clear();
        }

        void GUIContext::deliver_drag_drop_payload(GUIID target_id)
        {
            if(!m_drag_drop_active || !target_id || !m_drag_drop_payload_set) return;
            DragDropPayloadStorage storage;
            storage.type = m_drag_drop_type;
            storage.data = m_drag_drop_payload_data;
            storage.source = GUIItemHandle{get_object(), m_drag_drop_source_id, m_generation};
            storage.target = GUIItemHandle{get_object(), target_id, m_generation};
            storage.preview = true;
            storage.delivery = true;
            m_current_drag_drop_deliveries.insert_or_assign(target_id, move(storage));
            ItemResult& result = get_or_create_current_result(target_id);
            result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
            result.states.insert_or_assign(Name("gui.drag_drop_delivered"), Any(true));
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
                    if(m_drag_drop_candidate_source_id && !m_drag_drop_active)
                    {
                        f32 dx = e.position.x - m_drag_drop_start_pos.x;
                        f32 dy = e.position.y - m_drag_drop_start_pos.y;
                        if(dx * dx + dy * dy >= 16.0f)
                        {
                            start_drag_drop(m_drag_drop_candidate_source_id, m_drag_drop_candidate_type);
                        }
                    }
                    if(m_active_dock_split_space_id)
                    {
                        update_dock_splitter_from_pointer(e.position);
                    }
                    else if(m_active_dock_panel_id)
                    {
                        update_dock_panel_from_pointer(e.position);
                    }
                    else if(m_active_scrollbar_id)
                    {
                        update_scrollbar_from_pointer(e.position);
                    }
                    else if(m_active_table_resize_id)
                    {
                        update_table_resize_from_pointer(e.position);
                    }
                    else if(m_active_tab_scroll_id)
                    {
                    }
                    else if(m_active_tab_item_id)
                    {
                        if(m_active_tab_reorder_allowed)
                        {
                            f32 dx = e.position.x - m_active_tab_start_pos.x;
                            f32 dy = e.position.y - m_active_tab_start_pos.y;
                            if(!m_active_tab_reordering && dx * dx + dy * dy >= 16.0f)
                            {
                                m_active_tab_reordering = true;
                                select_tab_item(m_active_tab_bar_id, m_active_tab_item_id);
                            }
                            if(m_active_tab_reordering && reorder_tab_item_from_pointer(m_active_tab_bar_id, m_active_tab_item_id, e.position))
                            {
                                m_layout_dirty = true;
                            }
                        }
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
                    if(close_popups_for_pointer_down(e.position))
                    {
                        if(old_focused_id)
                        {
                            input_text_clear_selection(get_or_create_persistent_state(old_focused_id));
                        }
                        m_active_id = 0;
                        m_active_float_component = U32_MAX;
                        continue;
                    }
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
                    GUIID split_space_id = 0;
                    u32 split_node_index = U32_MAX;
                    GUIDockSplitAxis split_axis = GUIDockSplitAxis::x;
                    if(hit_test_dock_splitter(e.position, split_space_id, split_node_index, split_axis))
                    {
                        m_active_id = split_space_id;
                        m_focused_id = split_space_id;
                        m_active_dock_split_space_id = split_space_id;
                        m_active_dock_split_node = split_node_index;
                        m_active_dock_split_axis = split_axis;
                        m_active_dock_split_start_pos = e.position;
                        PersistentItemState& dock_state = get_or_create_persistent_state(split_space_id);
                        if(split_node_index < dock_state.dock_nodes.size())
                        {
                            m_active_dock_split_start_ratio = dock_state.dock_nodes[split_node_index].split_ratio;
                        }
                        PersistentItemState& state = get_or_create_persistent_state(split_space_id);
                        state.pointer_down = true;
                        state.active = true;
                        state.focused = true;
                        continue;
                    }
                    GUIID tab_space_id = 0;
                    GUIID tab_panel_id = 0;
                    u32 tab_leaf_index = U32_MAX;
                    if(hit_test_dock_panel_tab(e.position, tab_space_id, tab_panel_id, tab_leaf_index))
                    {
                        PersistentItemState& dock_state = get_or_create_persistent_state(tab_space_id);
                        if(tab_leaf_index < dock_state.dock_nodes.size())
                        {
                            dock_state.dock_nodes[tab_leaf_index].selected_tab = tab_panel_id;
                        }
                        m_active_id = tab_panel_id;
                        m_focused_id = tab_panel_id;
                        m_active_dock_space_id = tab_space_id;
                        m_active_dock_panel_id = tab_panel_id;
                        m_active_dock_panel_resize = false;
                        m_active_dock_panel_close = false;
                        m_active_dock_panel_title_drag = true;
                        m_active_dock_panel_was_floating = false;
                        m_active_dock_panel_undocked = false;
                        m_active_dock_panel_resize_neighbor_id = 0;
                        m_active_dock_panel_start_neighbor_height = 0.0f;
                        DockPanelPersistentState* panel_state = find_dock_panel_state(tab_space_id, tab_panel_id);
                        m_active_dock_panel_restore_rect = panel_state ? panel_state->rect : RectF(0.0f, 0.0f, 320.0f, 220.0f);
                        if(tab_leaf_index < dock_state.dock_nodes.size())
                        {
                            m_active_dock_panel_start_rect = dock_state.dock_nodes[tab_leaf_index].rect;
                            GUINode* tab_node = find_node(tab_panel_id);
                            GUIDockPanelStyle style;
                            if(tab_node)
                            {
                                for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
                                {
                                    if(m_submitted_desc.nodes[i].id == tab_panel_id)
                                    {
                                        style = m_layouts[i].dock_panel_style;
                                        break;
                                    }
                                }
                            }
                            m_active_dock_panel_start_title_rect = dock_panel_title_rect(m_active_dock_panel_start_rect, style);
                            m_active_dock_panel_grab_offset = Float2U(
                                e.position.x - m_active_dock_panel_start_rect.offset_x,
                                e.position.y - m_active_dock_panel_start_rect.offset_y);
                        }
                        PersistentItemState& state = get_or_create_persistent_state(tab_panel_id);
                        state.pointer_down = true;
                        state.active = true;
                        state.focused = true;
                        m_layout_dirty = true;
                        continue;
                    }
                    GUIID dock_space_id = 0;
                    GUIID dock_panel_id = 0;
                    bool dock_resize = false;
                    bool dock_close = false;
                    if(hit_test_dock_panel_chrome(e.position, dock_space_id, dock_panel_id, dock_resize, dock_close))
                    {
                        m_active_id = dock_panel_id;
                        m_focused_id = dock_panel_id;
                        m_active_dock_space_id = dock_space_id;
                        m_active_dock_panel_id = dock_panel_id;
                        m_active_dock_panel_resize = dock_resize;
                        m_active_dock_panel_close = dock_close;
                        m_active_dock_panel_title_drag = !dock_resize && !dock_close;
                        m_active_dock_panel_was_floating = false;
                        m_active_dock_panel_undocked = false;
                        m_active_dock_panel_resize_neighbor_id = 0;
                        m_active_dock_panel_start_neighbor_height = 0.0f;
                        raise_dock_panel(dock_space_id, dock_panel_id);
                        DockPanelPersistentState* panel_state = find_dock_panel_state(dock_space_id, dock_panel_id);
                        if(panel_state && !dock_close)
                        {
                            m_active_dock_panel_start_rect = panel_state->rect;
                            m_active_dock_panel_restore_rect = panel_state->rect;
                        }
                        else
                        {
                            m_active_dock_panel_restore_rect = RectF(0.0f, 0.0f, 320.0f, 220.0f);
                        }
                        for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
                        {
                            if(m_submitted_desc.nodes[i].id == dock_panel_id)
                            {
                                m_active_dock_panel_start_rect = m_layouts[i].dock_panel_rect;
                                m_active_dock_panel_start_title_rect = m_layouts[i].dock_panel_title_rect;
                                m_active_dock_panel_was_floating = m_layouts[i].dock_panel_floating;
                                m_active_dock_panel_grab_offset = Float2U(
                                    e.position.x - m_layouts[i].dock_panel_rect.offset_x,
                                    e.position.y - m_layouts[i].dock_panel_rect.offset_y);
                                if(dock_resize && !m_layouts[i].dock_panel_floating)
                                {
                                    for(u32 sibling = m_submitted_desc.nodes[i].next_sibling; sibling != U32_MAX; sibling = m_submitted_desc.nodes[sibling].next_sibling)
                                    {
                                        if(!m_layouts[sibling].dock_panel_child || !m_layouts[sibling].dock_panel_visible || m_layouts[sibling].dock_panel_floating)
                                        {
                                            continue;
                                        }
                                        m_active_dock_panel_resize_neighbor_id = m_submitted_desc.nodes[sibling].id;
                                        m_active_dock_panel_start_neighbor_height = m_layouts[sibling].dock_panel_rect.height;
                                        break;
                                    }
                                }
                                break;
                            }
                        }
                        PersistentItemState& state = get_or_create_persistent_state(dock_panel_id);
                        state.pointer_down = true;
                        state.active = true;
                        state.focused = true;
                        m_layout_dirty = true;
                        continue;
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
                    GUIID tab_scroll_bar_id = 0;
                    bool tab_scroll_left = false;
                    if(hit_test_tab_scroll_button(e.position, tab_scroll_bar_id, tab_scroll_left))
                    {
                        m_active_id = tab_scroll_bar_id;
                        m_focused_id = tab_scroll_bar_id;
                        if(old_focused_id && old_focused_id != tab_scroll_bar_id)
                        {
                            input_text_clear_selection(get_or_create_persistent_state(old_focused_id));
                        }
                        m_active_tab_scroll_id = tab_scroll_bar_id;
                        m_active_tab_scroll_left = tab_scroll_left;
                        PersistentItemState& state = get_or_create_persistent_state(tab_scroll_bar_id);
                        state.pointer_down = true;
                        state.active = true;
                        state.focused = true;
                        scroll_tab_bar(tab_scroll_bar_id, tab_scroll_left ? -96.0f : 96.0f);
                        continue;
                    }
                    GUIID tab_bar_id = 0;
                    GUIID tab_item_id = 0;
                    bool tab_close = false;
                    if(hit_test_tab_header(e.position, tab_bar_id, tab_item_id, tab_close))
                    {
                        m_active_id = tab_item_id;
                        m_focused_id = tab_item_id;
                        if(old_focused_id && old_focused_id != tab_item_id)
                        {
                            input_text_clear_selection(get_or_create_persistent_state(old_focused_id));
                        }
                        m_active_tab_bar_id = tab_bar_id;
                        m_active_tab_item_id = tab_item_id;
                        m_active_tab_close = tab_close;
                        m_active_tab_start_pos = e.position;
                        m_active_tab_reordering = false;
                        m_active_tab_reorder_allowed = false;
                        GUINode* tab_bar = find_node(tab_bar_id);
                        GUINode* tab_item = find_node(tab_item_id);
                        if(tab_bar && tab_item && tab_bar->kind == GUINodeKind::tab_bar && tab_item->kind == GUINodeKind::tab_item)
                        {
                            m_active_tab_reorder_allowed = test_flags(tab_bar->tab_bar_flags, GUITabBarFlag::reorderable) &&
                                !tab_close &&
                                !test_flags(tab_item->tab_item_flags, GUITabItemFlag::button) &&
                                !test_flags(tab_item->tab_item_flags, GUITabItemFlag::no_reorder);
                        }
                        PersistentItemState& state = get_or_create_persistent_state(tab_item_id);
                        state.pointer_down = true;
                        state.active = true;
                        state.focused = true;
                        continue;
                    }
                    GUIID target = hit_test(e.position);
                    Name drag_drop_type;
                    GUIID drag_drop_source = hit_test_drag_drop_source(e.position, drag_drop_type);
                    m_drag_drop_candidate_source_id = drag_drop_source;
                    m_drag_drop_candidate_type = drag_drop_type;
                    m_drag_drop_start_pos = e.position;
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
                    if(m_drag_drop_active)
                    {
                        GUIID drop_target = hit_test_drag_drop_target(m_drag_drop_type, e.position);
                        deliver_drag_drop_payload(drop_target);
                        if(m_active_id)
                        {
                            PersistentItemState& state = get_or_create_persistent_state(m_active_id);
                            state.pointer_down = false;
                            state.active = false;
                            state.text_selecting = false;
                        }
                        clear_drag_drop();
                        m_active_id = 0;
                        m_active_float_component = U32_MAX;
                        continue;
                    }
                    m_drag_drop_candidate_source_id = 0;
                    m_drag_drop_candidate_type.reset();
                    if(m_active_dock_split_space_id)
                    {
                        PersistentItemState& state = get_or_create_persistent_state(m_active_dock_split_space_id);
                        state.pointer_down = false;
                        state.active = false;
                        m_active_dock_split_space_id = 0;
                        m_active_dock_split_node = U32_MAX;
                        m_active_id = 0;
                        continue;
                    }
                    if(m_active_dock_panel_id)
                    {
                        if(m_active_dock_panel_close)
                        {
                            DockPanelPersistentState* panel_state = find_dock_panel_state(m_active_dock_space_id, m_active_dock_panel_id);
                            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
                            {
                                GUINode& node = m_submitted_desc.nodes[i];
                                if(node.id != m_active_dock_panel_id) continue;
                                if(point_in_rect(e.position, m_layouts[i].dock_panel_close_rect))
                                {
                                    if(node.dock_panel_open)
                                    {
                                        *node.dock_panel_open = false;
                                    }
                                    else if(panel_state)
                                    {
                                        panel_state->closed = true;
                                    }
                                    ItemResult& result = get_or_create_current_result(node.id);
                                    result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
                                    result.states.insert_or_assign(Name("gui.open"), Any(false));
                                    m_layout_dirty = true;
                                }
                                break;
                            }
                        }
                        else if(m_active_dock_panel_title_drag && !m_active_dock_panel_resize)
                        {
                            GUIID target_space_id = 0;
                            u32 target_leaf = U32_MAX;
                            GUIDockDropDirection drop_direction = GUIDockDropDirection::none;
                            DockPanelPersistentState* panel_state = find_dock_panel_state(m_active_dock_space_id, m_active_dock_panel_id);
                            if(panel_state && panel_state->mode == GUIDockPanelMode::floating &&
                                find_dock_drop_target(m_active_dock_panel_id, e.position, target_space_id, target_leaf, drop_direction) &&
                                target_space_id == m_active_dock_space_id && drop_direction != GUIDockDropDirection::none)
                            {
                                PersistentItemState& dock_state = get_or_create_persistent_state(target_space_id);
                                dock_tree_dock_panel(dock_state, m_active_dock_panel_id, target_leaf, drop_direction);
                                panel_state->mode = GUIDockPanelMode::docking;
                                panel_state->closed = false;
                                ItemResult& result = get_or_create_current_result(m_active_dock_panel_id);
                                result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
                                result.states.insert_or_assign(Name("gui.open"), Any(true));
                                m_layout_dirty = true;
                            }
                        }
                        PersistentItemState& state = get_or_create_persistent_state(m_active_dock_panel_id);
                        state.pointer_down = false;
                        state.active = false;
                        m_active_dock_space_id = 0;
                        m_active_dock_panel_id = 0;
                        m_active_dock_panel_resize = false;
                        m_active_dock_panel_close = false;
                        m_active_dock_panel_title_drag = false;
                        m_active_dock_panel_was_floating = false;
                        m_active_dock_panel_undocked = false;
                        m_active_dock_panel_resize_neighbor_id = 0;
                        m_active_dock_panel_start_neighbor_height = 0.0f;
                        m_active_id = 0;
                        m_active_float_component = U32_MAX;
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
                    if(m_active_tab_scroll_id)
                    {
                        PersistentItemState& state = get_or_create_persistent_state(m_active_tab_scroll_id);
                        state.pointer_down = false;
                        state.active = false;
                        m_active_tab_scroll_id = 0;
                        m_active_tab_scroll_left = false;
                        m_active_id = 0;
                        m_active_float_component = U32_MAX;
                        continue;
                    }
                    if(m_active_tab_item_id)
                    {
                        GUIID tab_bar_id = 0;
                        GUIID tab_item_id = 0;
                        bool tab_close = false;
                        bool hit_tab = hit_test_tab_header(e.position, tab_bar_id, tab_item_id, tab_close);
                        if(hit_tab && tab_item_id == m_active_tab_item_id && !m_active_tab_reordering)
                        {
                            ItemResult& item_result = get_or_create_current_result(tab_item_id);
                            item_result.states.insert_or_assign(Name("gui.clicked"), Any(true));
                            PersistentItemState& item_state = get_or_create_persistent_state(tab_item_id);
                            bool dbl = (m_time - item_state.last_click_time) <= 0.4;
                            item_result.states.insert_or_assign(Name("gui.double_clicked"), Any(dbl));
                            item_state.last_click_time = m_time;
                            for(GUINode& node : m_submitted_desc.nodes)
                            {
                                if(node.id != tab_item_id || node.kind != GUINodeKind::tab_item) continue;
                                if((m_active_tab_close || tab_close) && node.bool_value && !test_flags(node.tab_item_flags, GUITabItemFlag::no_close_button))
                                {
                                    *node.bool_value = false;
                                    item_result.states.insert_or_assign(Name("gui.open"), Any(false));
                                    item_result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
                                    PersistentItemState& bar_state = get_or_create_persistent_state(tab_bar_id);
                                    if(bar_state.tab_selected_id == tab_item_id)
                                    {
                                        bar_state.tab_selected_id = fallback_tab_item(tab_bar_id, tab_item_id);
                                        ItemResult& bar_result = get_or_create_current_result(tab_bar_id);
                                        bar_result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
                                    }
                                }
                                else if(!test_flags(node.tab_item_flags, GUITabItemFlag::button))
                                {
                                    select_tab_item(tab_bar_id, tab_item_id);
                                }
                                break;
                            }
                        }
                        PersistentItemState& state = get_or_create_persistent_state(m_active_tab_item_id);
                        state.pointer_down = false;
                        state.active = false;
                        m_active_tab_bar_id = 0;
                        m_active_tab_item_id = 0;
                        m_active_tab_close = false;
                        m_active_tab_reorder_allowed = false;
                        m_active_tab_reordering = false;
                        m_active_id = 0;
                        m_active_float_component = U32_MAX;
                        continue;
                    }
                    GUIID target = hit_test(e.position);
                    GUIID target_dock_space = 0;
                    GUIID target_dock_panel = 0;
                    if(hit_test_dock_panel(e.position, target_dock_space, target_dock_panel))
                    {
                        raise_dock_panel(target_dock_space, target_dock_panel);
                    }
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
                            else if(node.kind == GUINodeKind::tree_node && !tree_node_is_leaf(node))
                            {
                                bool toggle = true;
                                if(test_flags(node.tree_flags, GUITreeNodeFlag::open_on_arrow))
                                {
                                    toggle = point_in_rect(e.position, tree_node_arrow_rect(node, m_layouts[i].rect));
                                }
                                if(toggle)
                                {
                                    state.open = !state.open;
                                    result.states.insert_or_assign(Name("gui.open"), Any(state.open));
                                    result.states.insert_or_assign(Name("gui.value_changed"), Any(true));
                                }
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
                    GUIID tab_scroll_area = hit_test_tab_scroll_area(e.position);
                    if(tab_scroll_area)
                    {
                        f32 delta = e.wheel_delta.x != 0.0f ? -e.wheel_delta.x * 48.0f : -e.wheel_delta.y * 48.0f;
                        scroll_tab_bar(tab_scroll_area, delta);
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
                    if(e.key == GUIKey::esc && !m_open_popup_stack.empty())
                    {
                        if(test_flags(m_open_popup_stack.back().flags, GUIPopupFlag::close_on_escape))
                        {
                            close_current_popup();
                        }
                        continue;
                    }
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
                    m_active_tab_bar_id = 0;
                    m_active_tab_item_id = 0;
                    m_active_tab_close = false;
                    m_active_tab_reorder_allowed = false;
                    m_active_tab_reordering = false;
                    m_active_tab_scroll_id = 0;
                    m_active_tab_scroll_left = false;
                    m_active_dock_space_id = 0;
                    m_active_dock_panel_id = 0;
                    m_active_dock_panel_resize = false;
                    m_active_dock_panel_close = false;
                    m_active_dock_panel_title_drag = false;
                    m_active_dock_panel_was_floating = false;
                    m_active_dock_panel_undocked = false;
                    m_active_dock_panel_resize_neighbor_id = 0;
                    m_active_dock_panel_start_neighbor_height = 0.0f;
                    m_active_dock_split_space_id = 0;
                    m_active_dock_split_node = U32_MAX;
                    close_combo_dropdowns_except(0);
                    for(usize i = 0; i < m_open_popup_stack.size(); ++i)
                    {
                        if(test_flags(m_open_popup_stack[i].flags, GUIPopupFlag::close_on_blur))
                        {
                            close_popup_stack_from(i);
                            break;
                        }
                    }
                    clear_drag_drop();
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
                GUIID dock_space_id = 0;
                GUIID dock_panel_id = 0;
                bool dock_resize = false;
                bool dock_close = false;
                u32 dock_split_node = U32_MAX;
                GUIDockSplitAxis dock_split_axis = GUIDockSplitAxis::x;
                u32 dock_leaf_index = U32_MAX;
                GUIID tab_bar_id = 0;
                GUIID tab_item_id = 0;
                bool tab_close = false;
                GUIID tab_scroll_bar_id = 0;
                bool tab_scroll_left = false;
                if(hit_test_combo_dropdown(m_pointer_pos, combo_id, combo_item))
                {
                    m_hovered_id = combo_id;
                }
                else if(hit_test_tab_scroll_button(m_pointer_pos, tab_scroll_bar_id, tab_scroll_left))
                {
                    m_hovered_id = tab_scroll_bar_id;
                }
                else if(hit_test_scrollbar(m_pointer_pos, scrollbar_id, scrollbar_vertical, scrollbar_thumb))
                {
                    m_hovered_id = scrollbar_id;
                }
                else if(hit_test_dock_splitter(m_pointer_pos, dock_space_id, dock_split_node, dock_split_axis))
                {
                    m_hovered_id = dock_space_id;
                }
                else if(hit_test_dock_panel_tab(m_pointer_pos, dock_space_id, dock_panel_id, dock_leaf_index))
                {
                    m_hovered_id = dock_panel_id;
                }
                else if(hit_test_dock_panel_chrome(m_pointer_pos, dock_space_id, dock_panel_id, dock_resize, dock_close))
                {
                    m_hovered_id = dock_panel_id;
                }
                else if(hit_test_tab_header(m_pointer_pos, tab_bar_id, tab_item_id, tab_close))
                {
                    m_hovered_id = tab_item_id;
                }
                else
                {
                    m_hovered_id = hit_test(m_pointer_pos);
                }
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
                rebuild_popup_node_indices();
                prune_popup_stack();
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
                        if(!persistent.open_initialized)
                        {
                            persistent.open = true;
                            persistent.open_initialized = true;
                        }
                        result.states.insert_or_assign(Name("gui.open"), Any(persistent.open));
                    }
                    else if(node.kind == GUINodeKind::tree_node)
                    {
                        if(!persistent.open_initialized)
                        {
                            persistent.open = !tree_node_is_leaf(node) && test_flags(node.tree_flags, GUITreeNodeFlag::default_open);
                            persistent.open_initialized = true;
                        }
                        if(tree_node_is_leaf(node))
                        {
                            persistent.open = false;
                        }
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
                    else if(node.kind == GUINodeKind::popup)
                    {
                        persistent.open = popup_node_visible(node);
                        result.states.insert_or_assign(Name("gui.open"), Any(persistent.open));
                    }
                    else if(node.kind == GUINodeKind::tab_item)
                    {
                        bool open = !node.bool_value || *node.bool_value;
                        result.states.insert_or_assign(Name("gui.open"), Any(open));
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
                if(m_drag_drop_active)
                {
                    bool source_live = false;
                    for(const GUINode& node : m_submitted_desc.nodes)
                    {
                        if(node.id == m_drag_drop_source_id && contains_name(node.drag_drop_source_types, m_drag_drop_type))
                        {
                            source_live = true;
                            break;
                        }
                    }
                    if(!source_live)
                    {
                        clear_drag_drop();
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
                        GUIID combo_id = 0;
                        i32 combo_item = -1;
                        GUIID scrollbar_id = 0;
                        bool scrollbar_vertical = false;
                        RectF scrollbar_thumb;
                        GUIID dock_space_id = 0;
                        GUIID dock_panel_id = 0;
                        bool dock_resize = false;
                        bool dock_close = false;
                        u32 dock_split_node = U32_MAX;
                        GUIDockSplitAxis dock_split_axis = GUIDockSplitAxis::x;
                        u32 dock_leaf_index = U32_MAX;
                        GUIID tab_bar_id = 0;
                        GUIID tab_item_id = 0;
                        bool tab_close = false;
                        GUIID tab_scroll_bar_id = 0;
                        bool tab_scroll_left = false;
                        if(hit_test_combo_dropdown(m_pointer_pos, combo_id, combo_item))
                        {
                            m_hovered_id = combo_id;
                        }
                        else if(hit_test_tab_scroll_button(m_pointer_pos, tab_scroll_bar_id, tab_scroll_left))
                        {
                            m_hovered_id = tab_scroll_bar_id;
                        }
                        else if(hit_test_scrollbar(m_pointer_pos, scrollbar_id, scrollbar_vertical, scrollbar_thumb))
                        {
                            m_hovered_id = scrollbar_id;
                        }
                        else if(hit_test_dock_splitter(m_pointer_pos, dock_space_id, dock_split_node, dock_split_axis))
                        {
                            m_hovered_id = dock_space_id;
                        }
                        else if(hit_test_dock_panel_tab(m_pointer_pos, dock_space_id, dock_panel_id, dock_leaf_index))
                        {
                            m_hovered_id = dock_panel_id;
                        }
                        else if(hit_test_dock_panel_chrome(m_pointer_pos, dock_space_id, dock_panel_id, dock_resize, dock_close))
                        {
                            m_hovered_id = dock_panel_id;
                        }
                        else if(hit_test_tab_header(m_pointer_pos, tab_bar_id, tab_item_id, tab_close))
                        {
                            m_hovered_id = tab_item_id;
                        }
                        else
                        {
                            m_hovered_id = hit_test(m_pointer_pos);
                        }
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
                    else if(node.kind == GUINodeKind::tree_node)
                    {
                        result.states.insert_or_assign(Name("gui.open"), Any(persistent.open));
                    }
                    else if(node.kind == GUINodeKind::combo)
                    {
                        result.states.insert_or_assign(Name("gui.open"), Any(persistent.open));
                    }
                    else if(node.kind == GUINodeKind::popup)
                    {
                        persistent.open = popup_node_visible(node);
                        result.states.insert_or_assign(Name("gui.open"), Any(persistent.open));
                    }
                    else if(node.kind == GUINodeKind::tab_item)
                    {
                        bool open = !node.bool_value || *node.bool_value;
                        result.states.insert_or_assign(Name("gui.open"), Any(open));
                    }
                }
                m_submitted = true;
            }
            lucatchret;
            return ok;
        }
    }
}
