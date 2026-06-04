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
#include <Luna/Runtime/StringUtils.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "GUI.hpp"

namespace Luna
{
    namespace GUI
    {
        struct ContextNodeInputContext : NodeInputContext
        {
            Context* context = nullptr;
            ItemQueryState* result = nullptr;
            id_t node_id = 0;
            Float2U current_pointer_position = Float2U(0.0f);
            RectF current_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);

            virtual Float2U pointer_position() const override
            {
                return current_pointer_position;
            }

            virtual RectF rect() const override
            {
                return current_rect;
            }

            virtual object_t get_state(id_t id) const override
            {
                return context ? context->get_state_object(id) : nullptr;
            }

            virtual RV set_state(id_t id, object_t data, StateLifetime lifetime = StateLifetime::next_frame) override
            {
                return context ? context->set_state(id, data, lifetime) : BasicError::bad_arguments();
            }

            virtual void clear_state(id_t id) override
            {
                if(context)
                {
                    context->clear_state(id);
                }
            }

            virtual void set_state(const Name& key, const Any& value) override
            {
                if(result)
                {
                    result->states.insert_or_assign(key, value);
                }
            }

            virtual bool is_popup_open(id_t popup_id) const override
            {
                return context && popup_id ? context->is_popup_open(popup_id) : false;
            }

            virtual void open_menu_popup(id_t menu_id) override
            {
                if(context)
                {
                    context->open_menu_popup(menu_id);
                }
            }

            virtual void close_popup(id_t popup_id) override
            {
                if(context && popup_id)
                {
                    context->close_popup(ItemHandle{context->get_object(), popup_id, context->m_generation});
                }
            }

            virtual void close_all_popups() override
            {
                if(context)
                {
                    context->close_all_popups();
                }
            }
        };

        namespace
        {
            bool delete_input_text_selection(String& value, InputEditState& state)
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

            void clear_text_edit_state(InputEditState& state)
            {
                input_text_clear_selection(state);
                state.numeric_editing = false;
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

            String filter_numeric_text(const String& text, bool floating_point)
            {
                String filtered;
                for(c8 ch : text)
                {
                    if(ch >= '0' && ch <= '9')
                    {
                        filtered.push_back(ch);
                    }
                    else if(ch == '-' || ch == '+')
                    {
                        filtered.push_back(ch);
                    }
                    else if(floating_point && (ch == '.' || ch == 'e' || ch == 'E'))
                    {
                        filtered.push_back(ch);
                    }
                }
                return filtered;
            }

            bool parse_i32_text(const String& text, i32& value)
            {
                if(text.empty()) return false;
                c8* end = nullptr;
                i64 parsed = strtoi64(text.c_str(), &end, 10);
                if(end == text.c_str() || (end && *end)) return false;
                value = (i32)clamp(parsed, (i64)I32_MIN, (i64)I32_MAX);
                return true;
            }

            bool parse_f32_text(const String& text, f32& value)
            {
                if(text.empty()) return false;
                c8* end = nullptr;
                f32 parsed = strtof32(text.c_str(), &end);
                if(end == text.c_str() || (end && *end)) return false;
                value = parsed;
                return true;
            }

            i32 round_to_i32(f32 value)
            {
                f32 rounded = value >= 0.0f ? value + 0.5f : value - 0.5f;
                return (i32)clamp((i64)rounded, (i64)I32_MIN, (i64)I32_MAX);
            }

            bool apply_numeric_edit_text(Context& ctx, Node& node, InputEditState& state)
            {
                if(!numeric_text_editable(node) || !state.numeric_editing) return false;
                NumericBinding* binding = numeric_binding(node);
                if(!binding) return false;
                u32 component = min(state.numeric_edit_component, numeric_value_count(node) - 1);
                bool changed = false;
                if(numeric_value_f32(node))
                {
                    f32* values = binding->f32_value;
                    if(!values) return false;
                    f32 value = 0.0f;
                    if(!parse_f32_text(state.numeric_edit_text, value)) return false;
                    if(binding->max_value > binding->min_value)
                    {
                        value = clamp(value, binding->min_value, binding->max_value);
                    }
                    if(values[component] != value)
                    {
                        values[component] = value;
                        changed = true;
                    }
                }
                else
                {
                    i32* values = binding->i32_value;
                    if(!values) return false;
                    i32 value = 0;
                    if(!parse_i32_text(state.numeric_edit_text, value)) return false;
                    if(binding->max_value > binding->min_value)
                    {
                        value = clamp(value, (i32)binding->min_value, (i32)binding->max_value);
                    }
                    if(values[component] != value)
                    {
                        values[component] = value;
                        changed = true;
                    }
                }
                if(changed)
                {
                    ctx.mark_value_changed(node.id);
                }
                return changed;
            }
        }

        bool Context::hit_test_table_separator(const Float2U& pos, id_t& out_id, bool& out_column, u32& out_index) const
        {
            u32 hit_layer = hit_test_layer_index(pos);
            if(hit_layer == U32_MAX) return false;
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                const Node& node = m_submitted_desc.nodes[i];
                if(node.layer != hit_layer) continue;
                if(!table_layout(node)) continue;
                if(!node.enabled_state()) continue;
                const NodeLayout& layout = m_layouts[i];
                const RectF& clip = layout.clip_rect;
                if(!point_in_rect(pos, layout.rect) || !point_in_rect(pos, clip)) continue;
                const TableStyle& style = table_desc(node).style;
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

        void Context::update_table_resize_from_pointer(const Float2U& pos)
        {
            if(!table_resize_interaction_state().active_table_resize_id || table_resize_interaction_state().active_table_resize_index == U32_MAX) return;
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                Node& node = m_submitted_desc.nodes[i];
                if(node.id != table_resize_interaction_state().active_table_resize_id || !table_layout(node)) continue;
                NodeLayout& layout = m_layouts[i];
                Ref<TableLayoutState> persistent = get_or_create_widget_state<TableLayoutState>(node.id);
                if(table_resize_interaction_state().active_table_resize_column)
                {
                    u32 col = table_resize_interaction_state().active_table_resize_index;
                    if(col >= layout.table_column_offsets.size()) return;
                    if(persistent->table_column_sizes.size() <= col)
                    {
                        persistent->table_column_sizes.resize(col + 1, 0.0f);
                    }
                    persistent->table_column_sizes[col] = max(pos.x - layout.table_column_offsets[col], 24.0f);
                }
                else
                {
                    u32 row = table_resize_interaction_state().active_table_resize_index;
                    if(row >= layout.table_row_offsets.size()) return;
                    if(persistent->table_row_sizes.size() <= row)
                    {
                        persistent->table_row_sizes.resize(row + 1, 0.0f);
                    }
                    persistent->table_row_sizes[row] = max(pos.y - layout.table_row_offsets[row], 20.0f);
                }
                Ref<ItemQueryState> result = get_or_create_query_state(node.id);
                result->states.insert_or_assign(Name("gui.value_changed"), Any(true));
                m_layout_dirty = true;
                return;
            }
        }

        DockPanelPersistentState* Context::find_dock_panel_state(id_t dock_space_id, id_t panel_id)
        {
            DockSpaceState* dock_state = get_widget_state<DockSpaceState>(dock_space_id);
            if(!dock_state) return nullptr;
            auto panel_iter = dock_state->dock_panels.find(panel_id);
            return panel_iter == dock_state->dock_panels.end() ? nullptr : &panel_iter->second;
        }

        void Context::raise_dock_panel(id_t dock_space_id, id_t panel_id)
        {
            if(!dock_space_id || !panel_id) return;
            Ref<DockSpaceState> dock_state = get_or_create_widget_state<DockSpaceState>(dock_space_id);
            DockPanelPersistentState& panel_state = get_or_create_dock_panel_state(*dock_state, panel_id);
            if(panel_state.mode != DockPanelMode::floating) return;
            panel_state.z_order = dock_state->dock_next_z_order++;
        }

        bool Context::hit_test_dock_panel_layer(u32 layer_index, const Float2U& pos) const
        {
            if(m_layouts.size() != m_submitted_desc.nodes.size()) return false;
            if(layer_index >= m_submitted_desc.layers.size()) return false;
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                const Node& dock_node = m_submitted_desc.nodes[i];
                if(dock_node.layer != layer_index) continue;
                if(!dock_space_layout(dock_node)) continue;
                if(!point_in_rect(pos, m_layouts[i].rect) || !point_in_rect(pos, m_layouts[i].clip_rect)) continue;
                for(u32 child = dock_node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
                {
                    const NodeLayout& layout = m_layouts[child];
                    if(!layout.dock_panel_child || !layout.dock_panel_visible) continue;
                    if(point_in_rect(pos, layout.dock_panel_rect) && point_in_rect(pos, layout.dock_panel_clip_rect))
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        bool Context::hit_test_dock_panel(const Float2U& pos, id_t& out_space_id, id_t& out_panel_id) const
        {
            bool found = false;
            u32 best_z = 0;
            if(m_layouts.size() != m_submitted_desc.nodes.size()) return false;
            u32 hit_layer = hit_test_layer_index(pos);
            if(hit_layer == U32_MAX) return false;
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                const Node& dock_node = m_submitted_desc.nodes[i];
                if(dock_node.layer != hit_layer) continue;
                if(!dock_space_layout(dock_node)) continue;
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

        bool Context::hit_test_dock_panel_chrome(const Float2U& pos, id_t& out_space_id, id_t& out_panel_id, bool& out_resize, bool& out_close) const
        {
            bool found = false;
            u32 best_z = 0;
            out_resize = false;
            out_close = false;
            if(m_layouts.size() != m_submitted_desc.nodes.size()) return false;
            u32 hit_layer = hit_test_layer_index(pos);
            if(hit_layer == U32_MAX) return false;
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                const Node& dock_node = m_submitted_desc.nodes[i];
                if(dock_node.layer != hit_layer) continue;
                if(!dock_space_layout(dock_node)) continue;
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

        bool Context::hit_test_dock_panel_tab(const Float2U& pos, id_t& out_space_id, id_t& out_panel_id, u32& out_leaf_index) const
        {
            bool found = false;
            u32 best_z = 0;
            if(m_layouts.size() != m_submitted_desc.nodes.size()) return false;
            u32 hit_layer = hit_test_layer_index(pos);
            if(hit_layer == U32_MAX) return false;
            id_t top_space = 0;
            id_t top_panel = 0;
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
                const Node& dock_node = m_submitted_desc.nodes[i];
                if(dock_node.layer != hit_layer) continue;
                if(!dock_space_layout(dock_node)) continue;
                const DockSpaceState* dock_state = get_widget_state<DockSpaceState>(dock_node.id);
                if(!dock_state) continue;
                if(!point_in_rect(pos, m_layouts[i].rect) || !point_in_rect(pos, m_layouts[i].clip_rect)) continue;
                for(u32 child = dock_node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
                {
                    const NodeLayout& layout = m_layouts[child];
                    if(!layout.dock_panel_child || !layout.dock_panel_visible || layout.dock_panel_floating) continue;
                    if(layout.dock_leaf_index >= dock_state->dock_nodes.size()) continue;
                    const DockTreeNode& leaf = dock_state->dock_nodes[layout.dock_leaf_index];
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

        bool Context::hit_test_dock_splitter(const Float2U& pos, id_t& out_space_id, u32& out_node_index, DockSplitAxis& out_axis) const
        {
            if(m_layouts.size() != m_submitted_desc.nodes.size()) return false;
            u32 hit_layer = hit_test_layer_index(pos);
            if(hit_layer == U32_MAX) return false;
            id_t top_space = 0;
            id_t top_panel = 0;
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
                const Node& dock_node = m_submitted_desc.nodes[i];
                if(dock_node.layer != hit_layer) continue;
                if(!dock_space_layout(dock_node)) continue;
                const DockSpaceState* dock_state = get_widget_state<DockSpaceState>(dock_node.id);
                if(!dock_state) continue;
                if(!point_in_rect(pos, m_layouts[i].rect) || !point_in_rect(pos, m_layouts[i].clip_rect)) continue;
                if(dock_state->dock_root_node == U32_MAX || dock_state->dock_root_node >= dock_state->dock_nodes.size()) continue;
                Vector<u32> stack;
                stack.push_back(dock_state->dock_root_node);
                while(!stack.empty())
                {
                    u32 node_index = stack.back();
                    stack.pop_back();
                    if(node_index >= dock_state->dock_nodes.size()) continue;
                    const DockTreeNode& tree_node = dock_state->dock_nodes[node_index];
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

        void Context::update_dock_splitter_from_pointer(const Float2U& pos)
        {
            if(!dock_interaction_state().active_dock_split_space_id || dock_interaction_state().active_dock_split_node == U32_MAX) return;
            Ref<DockSpaceState> dock_state = get_or_create_widget_state<DockSpaceState>(dock_interaction_state().active_dock_split_space_id);
            if(dock_interaction_state().active_dock_split_node >= dock_state->dock_nodes.size()) return;
            DockTreeNode& tree_node = dock_state->dock_nodes[dock_interaction_state().active_dock_split_node];
            if(!tree_node.split) return;
            f32 splitter_size = dock_panel_splitter_size();
            f32 axis_size = tree_node.split_axis == DockSplitAxis::x ? tree_node.rect.width : tree_node.rect.height;
            f32 available = max(axis_size - splitter_size, 1.0f);
            f32 delta = tree_node.split_axis == DockSplitAxis::x ? pos.x - dock_interaction_state().active_dock_split_start_pos.x : pos.y - dock_interaction_state().active_dock_split_start_pos.y;
            f32 ratio = dock_interaction_state().active_dock_split_start_ratio + delta / available;
            tree_node.split_ratio = clamp(ratio, 0.08f, 0.92f);
            m_layout_dirty = true;
        }

        bool Context::find_dock_drop_target(id_t payload_panel, const Float2U& pos, id_t& out_space_id, u32& out_leaf_index, DockDropDirection& out_direction) const
        {
            if(m_layouts.size() != m_submitted_desc.nodes.size()) return false;
            u32 hit_layer = hit_test_layer_index(pos);
            if(hit_layer == U32_MAX) return false;
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                const Node& dock_node = m_submitted_desc.nodes[i];
                if(dock_node.layer != hit_layer) continue;
                if(!dock_space_layout(dock_node)) continue;
                const DockSpaceState* dock_state = get_widget_state<DockSpaceState>(dock_node.id);
                if(!dock_state) continue;
                if(!point_in_rect(pos, m_layouts[i].rect) || !point_in_rect(pos, m_layouts[i].clip_rect)) continue;
                if(dock_state->dock_root_node == U32_MAX || dock_state->dock_root_node >= dock_state->dock_nodes.size())
                {
                    out_space_id = dock_node.id;
                    out_leaf_index = U32_MAX;
                    out_direction = point_in_rect(pos, dock_drop_icon_rect(m_layouts[i].rect, DockDropDirection::center)) ?
                        DockDropDirection::center :
                        DockDropDirection::none;
                    return true;
                }
                Vector<u32> stack;
                stack.push_back(dock_state->dock_root_node);
                while(!stack.empty())
                {
                    u32 node_index = stack.back();
                    stack.pop_back();
                    if(node_index >= dock_state->dock_nodes.size()) continue;
                    const DockTreeNode& leaf = dock_state->dock_nodes[node_index];
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
                    static const DockDropDirection directions[] = {
                        DockDropDirection::center,
                        DockDropDirection::left,
                        DockDropDirection::right,
                        DockDropDirection::up,
                        DockDropDirection::down
                    };
                    for(DockDropDirection direction : directions)
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
                    out_direction = DockDropDirection::none;
                    return true;
                }
            }
            return false;
        }

        void Context::update_dock_panel_from_pointer(const Float2U& pos)
        {
            if(!dock_interaction_state().active_dock_space_id || !dock_interaction_state().active_dock_panel_id) return;
            DockPanelPersistentState* panel_state = find_dock_panel_state(dock_interaction_state().active_dock_space_id, dock_interaction_state().active_dock_panel_id);
            if(!panel_state) return;
            if(dock_interaction_state().active_dock_panel_close) return;
            if(dock_interaction_state().active_dock_panel_title_drag && !dock_interaction_state().active_dock_panel_was_floating && !dock_interaction_state().active_dock_panel_undocked)
            {
                RectF release_rect = dock_interaction_state().active_dock_panel_start_title_rect;
                release_rect.offset_x -= 8.0f;
                release_rect.offset_y -= 8.0f;
                release_rect.width += 16.0f;
                release_rect.height += 16.0f;
                if(point_in_rect(pos, release_rect))
                {
                    return;
                }
                Ref<DockSpaceState> dock_state = get_or_create_widget_state<DockSpaceState>(dock_interaction_state().active_dock_space_id);
                dock_tree_remove_panel(*dock_state, dock_interaction_state().active_dock_panel_id);
                panel_state->mode = DockPanelMode::floating;
                panel_state->rect = dock_interaction_state().active_dock_panel_restore_rect;
                panel_state->rect.width = max(panel_state->rect.width, 1.0f);
                panel_state->rect.height = max(panel_state->rect.height, 1.0f);
                panel_state->z_order = dock_state->dock_next_z_order++;
                dock_interaction_state().active_dock_panel_start_rect = panel_state->rect;
                dock_interaction_state().active_dock_panel_grab_offset.x = clamp(dock_interaction_state().active_dock_panel_grab_offset.x, 8.0f, max(panel_state->rect.width - 8.0f, 8.0f));
                dock_interaction_state().active_dock_panel_grab_offset.y = clamp(dock_interaction_state().active_dock_panel_grab_offset.y, 4.0f, max(panel_state->rect.height - 4.0f, 4.0f));
                dock_interaction_state().active_dock_panel_undocked = true;
            }
            if(dock_interaction_state().active_dock_panel_resize && !dock_interaction_state().active_dock_panel_was_floating)
            {
                DockPanelPersistentState* neighbor_state = find_dock_panel_state(dock_interaction_state().active_dock_space_id, dock_interaction_state().active_dock_panel_resize_neighbor_id);
                if(!neighbor_state) return;
                f32 active_min_height = 32.0f;
                f32 neighbor_min_height = 32.0f;
                for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
                {
                    const Node& node = m_submitted_desc.nodes[i];
                    if(node.id == dock_interaction_state().active_dock_panel_id)
                    {
                        active_min_height = dock_panel_min_height(m_layouts[i].dock_panel_style);
                    }
                    else if(node.id == dock_interaction_state().active_dock_panel_resize_neighbor_id)
                    {
                        neighbor_min_height = dock_panel_min_height(m_layouts[i].dock_panel_style);
                    }
                }
                f32 total_height = max(dock_interaction_state().active_dock_panel_start_rect.height + dock_interaction_state().active_dock_panel_start_neighbor_height, 1.0f);
                f32 delta = pos.y - (dock_interaction_state().active_dock_panel_start_rect.offset_y + dock_interaction_state().active_dock_panel_start_rect.height);
                f32 active_height = dock_interaction_state().active_dock_panel_start_rect.height + delta;
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

            panel_state->mode = DockPanelMode::floating;
            if(dock_interaction_state().active_dock_panel_resize)
            {
                f32 min_width = 1.0f;
                f32 min_height = 1.0f;
                for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
                {
                    if(m_submitted_desc.nodes[i].id == dock_interaction_state().active_dock_panel_id)
                    {
                        min_width = max(m_layouts[i].dock_panel_style.min_floating_size.x, 1.0f);
                        min_height = dock_panel_min_height(m_layouts[i].dock_panel_style);
                        break;
                    }
                }
                panel_state->rect = dock_interaction_state().active_dock_panel_start_rect;
                panel_state->rect.width = max(pos.x - dock_interaction_state().active_dock_panel_start_rect.offset_x, min_width);
                panel_state->rect.height = max(pos.y - dock_interaction_state().active_dock_panel_start_rect.offset_y, min_height);
            }
            else if(!dock_interaction_state().active_dock_panel_close)
            {
                panel_state->rect.offset_x = pos.x - dock_interaction_state().active_dock_panel_grab_offset.x;
                panel_state->rect.offset_y = pos.y - dock_interaction_state().active_dock_panel_grab_offset.y;
                panel_state->rect.width = dock_interaction_state().active_dock_panel_start_rect.width;
                panel_state->rect.height = dock_interaction_state().active_dock_panel_start_rect.height;
            }
            m_layout_dirty = true;
        }

        void Context::clamp_scroll_state(id_t id)
        {
            if(!id || m_layouts.size() != m_submitted_desc.nodes.size()) return;
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                const Node& node = m_submitted_desc.nodes[i];
                if(node.id != id || !scroll_layout(node)) continue;
                const NodeLayout& layout = m_layouts[i];
                Ref<ScrollState> state = get_or_create_widget_state<ScrollState>(id);
                state->scroll_x = clamp(state->scroll_x, 0.0f, scroll_max_x(layout));
                state->scroll_y = clamp(state->scroll_y, 0.0f, scroll_max_y(layout));
                return;
            }
        }

        bool Context::hit_test_scrollbar(const Float2U& pos, id_t& out_id, bool& out_vertical, RectF& out_thumb_rect) const
        {
            if(m_layouts.size() != m_submitted_desc.nodes.size()) return false;
            u32 hit_layer = hit_test_layer_index(pos);
            if(hit_layer == U32_MAX) return false;
            for(usize i = m_submitted_desc.nodes.size(); i > 0; --i)
            {
                u32 node_index = (u32)(i - 1);
                const Node& node = m_submitted_desc.nodes[node_index];
                if(node.layer != hit_layer) continue;
                if(!scroll_layout(node)) continue;
                if(!node.enabled_state()) continue;
                const NodeLayout& layout = m_layouts[node_index];
                if(!point_in_rect(pos, layout.rect) || !point_in_rect(pos, layout.clip_rect)) continue;

                ScrollState empty_state;
                const ScrollState* stored_state = get_widget_state<ScrollState>(node.id);
                const ScrollState& state = stored_state ? *stored_state : empty_state;
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

        void Context::update_scrollbar_from_pointer(const Float2U& pos)
        {
            if(!scrollbar_interaction_state().active_scrollbar_id || m_layouts.size() != m_submitted_desc.nodes.size()) return;
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                const Node& node = m_submitted_desc.nodes[i];
                if(node.id != scrollbar_interaction_state().active_scrollbar_id || !scroll_layout(node)) continue;
                const NodeLayout& layout = m_layouts[i];
                Ref<ScrollState> state = get_or_create_widget_state<ScrollState>(node.id);
                f32 old_scroll_x = state->scroll_x;
                f32 old_scroll_y = state->scroll_y;
                if(scrollbar_interaction_state().active_scrollbar_vertical)
                {
                    RectF thumb = scroll_vertical_thumb_rect(layout, *state);
                    RectF track = scroll_vertical_track_rect(layout);
                    f32 travel = max(track.height - thumb.height, 0.0f);
                    f32 t = travel > 0.0f ? (pos.y - track.offset_y - scrollbar_interaction_state().active_scrollbar_grab_offset) / travel : 0.0f;
                    state->scroll_y = clamp(t, 0.0f, 1.0f) * scroll_max_y(layout);
                }
                else
                {
                    RectF thumb = scroll_horizontal_thumb_rect(layout, *state);
                    RectF track = scroll_horizontal_track_rect(layout);
                    f32 travel = max(track.width - thumb.width, 0.0f);
                    f32 t = travel > 0.0f ? (pos.x - track.offset_x - scrollbar_interaction_state().active_scrollbar_grab_offset) / travel : 0.0f;
                    state->scroll_x = clamp(t, 0.0f, 1.0f) * scroll_max_x(layout);
                }
                clamp_scroll_state(node.id);
                if(state->scroll_x != old_scroll_x || state->scroll_y != old_scroll_y)
                {
                    Ref<ItemQueryState> result = get_or_create_query_state(node.id);
                    result->states.insert_or_assign(Name("gui.value_changed"), Any(true));
                    m_layout_dirty = true;
                }
                return;
            }
        }

        bool Context::hit_test_tab_header(const Float2U& pos, id_t& out_tab_bar_id, id_t& out_tab_item_id, bool& out_close) const
        {
            if(m_layouts.size() != m_submitted_desc.nodes.size()) return false;
            u32 hit_layer = hit_test_layer_index(pos);
            if(hit_layer == U32_MAX) return false;
            for(usize i = m_submitted_desc.nodes.size(); i > 0; --i)
            {
                u32 node_index = (u32)(i - 1);
                const Node& node = m_submitted_desc.nodes[node_index];
                if(node.layer != hit_layer) continue;
                if(!tab_item_layout(node)) continue;
                if(!node.enabled_state()) continue;
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

        bool Context::hit_test_tab_scroll_button(const Float2U& pos, id_t& out_tab_bar_id, bool& out_left) const
        {
            if(m_layouts.size() != m_submitted_desc.nodes.size()) return false;
            u32 hit_layer = hit_test_layer_index(pos);
            if(hit_layer == U32_MAX) return false;
            for(usize i = m_submitted_desc.nodes.size(); i > 0; --i)
            {
                u32 node_index = (u32)(i - 1);
                const Node& node = m_submitted_desc.nodes[node_index];
                if(node.layer != hit_layer) continue;
                if(!tab_bar_layout(node)) continue;
                if(!node.enabled_state()) continue;
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

        id_t Context::hit_test_tab_scroll_area(const Float2U& pos) const
        {
            if(m_layouts.size() != m_submitted_desc.nodes.size()) return 0;
            u32 hit_layer = hit_test_layer_index(pos);
            if(hit_layer == U32_MAX) return 0;
            for(usize i = m_submitted_desc.nodes.size(); i > 0; --i)
            {
                u32 node_index = (u32)(i - 1);
                const Node& node = m_submitted_desc.nodes[node_index];
                if(node.layer != hit_layer) continue;
                if(!tab_bar_layout(node)) continue;
                if(!node.enabled_state()) continue;
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

        id_t Context::fallback_tab_item(id_t tab_bar_id, id_t excluded_tab_item_id) const
        {
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                const Node& tab_bar = m_submitted_desc.nodes[i];
                if(tab_bar.id != tab_bar_id || !tab_bar_layout(tab_bar)) continue;
                id_t fallback = 0;
                for(u32 child = tab_bar.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
                {
                    const Node& tab = m_submitted_desc.nodes[child];
                    if(!tab_item_layout(tab) || tab.id == excluded_tab_item_id) continue;
                    const TabItemNode* tab_item = tab_item_node(tab);
                    if(!tab_item || test_flags(tab_item->flags, TabItemFlag::button)) continue;
                    if(!bool_value_open(tab)) continue;
                    fallback = tab.id;
                    break;
                }
                return fallback;
            }
            return 0;
        }

        void Context::select_tab_item(id_t tab_bar_id, id_t tab_item_id)
        {
            if(!tab_bar_id || !tab_item_id) return;
            Ref<TabBarState> state = get_or_create_widget_state<TabBarState>(tab_bar_id);
            if(state->tab_selected_id == tab_item_id) return;
            state->tab_selected_id = tab_item_id;
            Ref<ItemQueryState> result = get_or_create_query_state(tab_bar_id);
            result->states.insert_or_assign(Name("gui.value_changed"), Any(true));
            m_layout_dirty = true;
        }

        bool Context::reorder_tab_item_from_pointer(id_t tab_bar_id, id_t tab_item_id, const Float2U& pos)
        {
            if(!tab_bar_id || !tab_item_id) return false;
            Ref<TabBarState> state = get_or_create_widget_state<TabBarState>(tab_bar_id);
            usize old_index = USIZE_MAX;
            for(usize i = 0; i < state->tab_order.size(); ++i)
            {
                if(state->tab_order[i] == tab_item_id)
                {
                    old_index = i;
                    break;
                }
            }
            if(old_index == USIZE_MAX) return false;

            usize new_index = state->tab_order.size();
            for(usize i = 0; i < state->tab_order.size(); ++i)
            {
                id_t id = state->tab_order[i];
                if(id == tab_item_id) continue;
                for(usize node_index = 0; node_index < m_submitted_desc.nodes.size(); ++node_index)
                {
                    const Node& node = m_submitted_desc.nodes[node_index];
                    if(node.id != id || !tab_item_layout(node)) continue;
                    if(node.parent == U32_MAX || node.parent >= m_submitted_desc.nodes.size() ||
                        m_submitted_desc.nodes[node.parent].id != tab_bar_id) break;
                    const RectF& rect = m_layouts[node_index].tab_header_rect;
                    if(pos.x < rect.offset_x + rect.width * 0.5f)
                    {
                        new_index = i;
                    }
                    break;
                }
                if(new_index != state->tab_order.size()) break;
            }
            if(new_index > old_index) --new_index;
            new_index = min(new_index, state->tab_order.size() - 1);
            if(new_index == old_index) return false;
            state->tab_order.erase(state->tab_order.begin() + old_index);
            state->tab_order.insert(state->tab_order.begin() + new_index, tab_item_id);
            Ref<ItemQueryState> result = get_or_create_query_state(tab_bar_id);
            result->states.insert_or_assign(Name("gui.value_changed"), Any(true));
            return true;
        }

        void Context::scroll_tab_bar(id_t tab_bar_id, f32 delta)
        {
            if(!tab_bar_id || delta == 0.0f) return;
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                const Node& node = m_submitted_desc.nodes[i];
                if(node.id != tab_bar_id || !tab_bar_layout(node)) continue;
                const NodeLayout& layout = m_layouts[i];
                if(!layout.tab_scrollable) return;
                Ref<TabBarState> state = get_or_create_widget_state<TabBarState>(tab_bar_id);
                f32 old_scroll = state->tab_scroll_x;
                state->tab_scroll_x = clamp(state->tab_scroll_x + delta, 0.0f, layout.tab_scroll_max);
                if(state->tab_scroll_x != old_scroll)
                {
                    Ref<ItemQueryState> result = get_or_create_query_state(tab_bar_id);
                    result->states.insert_or_assign(Name("gui.value_changed"), Any(true));
                    m_layout_dirty = true;
                }
                return;
            }
        }

        void Context::open_menu_popup(id_t menu_id)
        {
            Node* menu = find_node(menu_id);
            MenuItemNode* item = menu ? menu_item_node(*menu) : nullptr;
            if(!menu || !item || !item->enabled_state() || !item->popup_id) return;
            m_popup_stack.next_opener_id = menu_id;
            open_popup(ItemHandle{get_object(), item->popup_id, m_generation});
            Ref<ItemQueryState> result = get_or_create_query_state(menu->id);
            result->states.insert_or_assign(Name("gui.open"), Any(true));
        }

        void Context::update_menu_hover()
        {
            if(m_popup_stack.open_stack.empty()) return;
            i32 popup_level = popup_level_at_pos(m_pointer_pos);
            Node* hovered = m_hovered_id ? find_node(m_hovered_id) : nullptr;
            MenuItemNode* hovered_menu = hovered ? menu_item_node(*hovered) : nullptr;
            if(hovered_menu && hovered_menu->enabled_state() && hovered_menu->popup_id)
            {
                if(is_popup_open(hovered_menu->popup_id)) return;
                open_menu_popup(hovered->id);
                return;
            }
            if(popup_level >= 0 && (usize)popup_level + 1 < m_popup_stack.open_stack.size())
            {
                close_popup_stack_from((usize)popup_level + 1);
            }
        }

        id_t Context::hit_test_node(u32 node_index, const Float2U& pos, HitTestFilter filter) const
        {
            id_t ret = 0;
            const Node& node = m_submitted_desc.nodes[node_index];
            if(popup_layer(node) && !popup_node_visible(node))
            {
                return 0;
            }
            if(tooltip_layer(node))
            {
                return 0;
            }
            const RectF& rect = m_layouts[node_index].rect;
            const RectF& clip = m_layouts[node_index].clip_rect;
            if(tab_item_layout(node))
            {
                const NodeLayout& layout = m_layouts[node_index];
                if(filter == HitTestFilter::none && node.interactive &&
                    node.enabled_state() &&
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
            else if((filter == HitTestFilter::none && node.hit_test(rect, clip, pos)) ||
                (filter == HitTestFilter::scroll_view && node.enabled_state() && scroll_layout(node) && point_in_rect(pos, rect) && point_in_rect(pos, clip)))
            {
                ret = node.id;
            }
            if(dock_space_layout(node))
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
                        id_t child_hit = hit_test_node(child, pos, filter);
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
                    id_t child_hit = hit_test_node(child, pos, filter);
                    if(child_hit) ret = child_hit;
                }
                return ret;
            }
            for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
            {
                if(absolute_node(m_submitted_desc.nodes[child])) continue;
                id_t child_hit = hit_test_node(child, pos, filter);
                if(child_hit)
                {
                    ret = child_hit;
                }
            }
            for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
            {
                if(!absolute_node(m_submitted_desc.nodes[child])) continue;
                id_t child_hit = hit_test_node(child, pos, filter);
                if(child_hit)
                {
                    ret = child_hit;
                }
            }
            return ret;
        }

        id_t Context::hit_test(const Float2U& pos) const
        {
            if(m_layouts.size() == m_submitted_desc.nodes.size())
            {
                for(usize i = m_submitted_desc.layers.size(); i > 0; --i)
                {
                    const Layer& layer = m_submitted_desc.layers[i - 1];
                    if(layer.root == U32_MAX || layer.root >= m_submitted_desc.nodes.size()) continue;
                    id_t layer_hit = hit_test_node(layer.root, pos, HitTestFilter::none);
                    if(layer_hit) return layer_hit;
                    if(hit_test_dock_panel_layer((u32)(i - 1), pos)) return 0;
                }
            }
            return 0;
        }

        u32 Context::hit_test_layer_index(const Float2U& pos) const
        {
            if(m_layouts.size() == m_submitted_desc.nodes.size())
            {
                for(usize i = m_submitted_desc.layers.size(); i > 0; --i)
                {
                    const Layer& layer = m_submitted_desc.layers[i - 1];
                    if(layer.root == U32_MAX || layer.root >= m_submitted_desc.nodes.size()) continue;
                    if(hit_test_node(layer.root, pos, HitTestFilter::none))
                    {
                        return (u32)(i - 1);
                    }
                    if(hit_test_dock_panel_layer((u32)(i - 1), pos))
                    {
                        return (u32)(i - 1);
                    }
                }
            }
            return U32_MAX;
        }

        id_t Context::hit_test_filtered(const Float2U& pos, HitTestFilter filter) const
        {
            if(m_layouts.size() == m_submitted_desc.nodes.size())
            {
                for(usize i = m_submitted_desc.layers.size(); i > 0; --i)
                {
                    const Layer& layer = m_submitted_desc.layers[i - 1];
                    if(layer.root == U32_MAX || layer.root >= m_submitted_desc.nodes.size()) continue;
                    id_t layer_hit = hit_test_node(layer.root, pos, filter);
                    if(layer_hit) return layer_hit;
                    if(hit_test_dock_panel_layer((u32)(i - 1), pos)) return 0;
                }
            }
            return 0;
        }

        id_t Context::hit_test_drag_drop_source(const Float2U& pos, Name& out_type) const
        {
            if(m_layouts.size() != m_submitted_desc.nodes.size()) return 0;
            for(usize layer_iter = m_submitted_desc.layers.size(); layer_iter > 0; --layer_iter)
            {
                u32 layer_index = (u32)(layer_iter - 1);
                for(usize i = m_submitted_desc.nodes.size(); i > 0; --i)
                {
                    u32 node_index = (u32)(i - 1);
                    const Node& node = m_submitted_desc.nodes[node_index];
                    if(node.layer != layer_index) continue;
                    if(node.drag_drop_source_types.empty()) continue;
                    if(!node.enabled_state()) continue;
                    const NodeLayout& layout = m_layouts[node_index];
                    if(layout.dock_panel_child && !layout.dock_panel_visible) continue;
                    if(!point_in_rect(pos, layout.rect) || !point_in_rect(pos, layout.clip_rect)) continue;
                    out_type = node.drag_drop_source_types[0];
                    return node.id;
                }
            }
            return 0;
        }

        id_t Context::hit_test_drag_drop_target(const Name& type, const Float2U& pos) const
        {
            if(!type || m_layouts.size() != m_submitted_desc.nodes.size()) return 0;
            for(usize layer_iter = m_submitted_desc.layers.size(); layer_iter > 0; --layer_iter)
            {
                u32 layer_index = (u32)(layer_iter - 1);
                id_t best = 0;
                f32 best_area = F32_MAX;
                for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
                {
                    const Node& node = m_submitted_desc.nodes[i];
                    if(node.layer != layer_index) continue;
                    if(!contains_name(node.drag_drop_target_types, type)) continue;
                    if(!node.enabled_state()) continue;
                    if(node.id == m_drag_drop.source_id) continue;
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
                if(best) return best;
            }
            return 0;
        }

        void Context::start_drag_drop(id_t source_id, const Name& type)
        {
            if(!source_id || !type) return;
            m_drag_drop.start(source_id, type);
            Ref<ItemQueryState> result = get_or_create_query_state(source_id);
            result->states.insert_or_assign(Name("gui.active"), Any(true));
        }

        void Context::clear_drag_drop()
        {
            m_drag_drop.clear();
        }

        void Context::deliver_drag_drop_payload(id_t target_id)
        {
            if(!m_drag_drop.active || !target_id || !m_drag_drop.payload_set) return;
            DragDropPayloadStorage storage;
            storage.type = m_drag_drop.type;
            storage.data = m_drag_drop.payload_data;
            storage.source = ItemHandle{get_object(), m_drag_drop.source_id, m_generation};
            storage.target = ItemHandle{get_object(), target_id, m_generation};
            storage.preview = true;
            storage.delivery = true;
            m_drag_drop.current_deliveries.insert_or_assign(target_id, move(storage));
            Ref<ItemQueryState> result = get_or_create_query_state(target_id);
            result->states.insert_or_assign(Name("gui.value_changed"), Any(true));
            result->states.insert_or_assign(Name("gui.drag_drop_delivered"), Any(true));
        }

        Node* Context::find_node(id_t id)
        {
            for(Node& node : m_submitted_desc.nodes)
            {
                if(node.id == id) return &node;
            }
            return nullptr;
        }

        void Context::mark_value_changed(id_t id)
        {
            if(!id) return;
            Ref<ItemQueryState> result = get_or_create_query_state(id);
            result->states.insert_or_assign(Name("gui.value_changed"), Any(true));
            Node* node = find_node(id);
            NumericBinding* binding = node ? numeric_binding(*node) : nullptr;
            if(binding && binding->color_owner_id)
            {
                apply_color_picker_numeric_state(binding->color_owner_id, binding->color_part);
                Ref<ItemQueryState> owner_result = get_or_create_query_state(binding->color_owner_id);
                owner_result->states.insert_or_assign(Name("gui.value_changed"), Any(true));
            }
            while(node && node->parent != U32_MAX)
            {
                node = &m_submitted_desc.nodes[node->parent];
                id_t owner = popup_owner(*node);
                if(popup_layer(*node) && owner)
                {
                    Ref<ItemQueryState> owner_result = get_or_create_query_state(owner);
                    owner_result->states.insert_or_assign(Name("gui.value_changed"), Any(true));
                    break;
                }
            }
        }

        void Context::sync_color_picker_numeric_state(id_t owner_id)
        {
            Node* owner = find_node(owner_id);
            if(!owner || !color_picker_node(*owner)) return;
            ColorBinding* binding = color_binding(*owner);
            if(!binding) return;
            Ref<ColorPickerState> state = get_or_create_widget_state<ColorPickerState>(owner_id);
            ensure_color_picker_state_channels(*state);
            Float4U color = read_color_value(*owner);
            state->color_picker_rgb[0] = (i32)color_channel_to_u8(color.x);
            state->color_picker_rgb[1] = (i32)color_channel_to_u8(color.y);
            state->color_picker_rgb[2] = (i32)color_channel_to_u8(color.z);
            state->color_picker_rgb[3] = (i32)color_channel_to_u8(color.w);
            f32 h = 0.0f;
            f32 s = 0.0f;
            f32 v = 0.0f;
            color_rgb_to_hsv(color.x, color.y, color.z, h, s, v);
            state->color_picker_hsv[0] = (i32)color_channel_to_u8(h);
            state->color_picker_hsv[1] = (i32)color_channel_to_u8(s);
            state->color_picker_hsv[2] = (i32)color_channel_to_u8(v);
        }

        void Context::apply_color_picker_numeric_state(id_t owner_id, ColorChannelPart part)
        {
            Node* owner = find_node(owner_id);
            if(!owner || !color_picker_node(*owner)) return;
            ColorBinding* binding = color_binding(*owner);
            if(!binding) return;
            Ref<ColorPickerState> state = get_or_create_widget_state<ColorPickerState>(owner_id);
            ensure_color_picker_state_channels(*state);
            Float4U color = read_color_value(*owner);
            if(part == ColorChannelPart::rgb)
            {
                color.x = color_u8_to_channel((u8)clamp(state->color_picker_rgb[0], 0, 255));
                color.y = color_u8_to_channel((u8)clamp(state->color_picker_rgb[1], 0, 255));
                color.z = color_u8_to_channel((u8)clamp(state->color_picker_rgb[2], 0, 255));
                if(f32_value_count(*owner) > 3)
                {
                    color.w = color_u8_to_channel((u8)clamp(state->color_picker_rgb[3], 0, 255));
                }
            }
            else if(part == ColorChannelPart::hsv)
            {
                f32 h = color_u8_to_channel((u8)clamp(state->color_picker_hsv[0], 0, 255));
                f32 s = color_u8_to_channel((u8)clamp(state->color_picker_hsv[1], 0, 255));
                f32 v = color_u8_to_channel((u8)clamp(state->color_picker_hsv[2], 0, 255));
                color = color_hsv_to_rgb(h, s, v, color.w);
            }
            write_color_value(*owner, color);
            sync_color_picker_numeric_state(owner_id);
        }

        void Context::update_color_picker_from_pointer(id_t id, const Float2U& pos)
        {
            Node* node = find_node(id);
            if(!node || !color_picker_node(*node)) return;
            ColorBinding* node_binding = color_binding(*node);
            id_t owner_id = node_binding && node_binding->owner_id ? node_binding->owner_id : id;
            Node* owner = find_node(owner_id);
            if(!owner) owner = node;
            if(!color_binding(*owner)) return;
            RectF rect;
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                if(m_submitted_desc.nodes[i].id == id)
                {
                    rect = m_layouts[i].rect;
                    break;
                }
            }
            Ref<ColorPickerState> state = get_or_create_widget_state<ColorPickerState>(owner_id);
            if(color_picker_interaction_state().active_color_part == 3)
            {
                if(state->color_picker_original_valid)
                {
                    write_color_value(*owner, state->color_picker_original);
                    sync_color_picker_numeric_state(owner_id);
                    mark_value_changed(owner_id);
                }
                return;
            }
            Float4U color = read_color_value(*owner);
            f32 x = 0.0f;
            f32 y = 0.0f;
            f32 bar = 0.0f;
            i32 axis = color_picker_axis_ref(*state);
            color_picker_channels_from_color(axis, color, x, y, bar);
            if(color_picker_interaction_state().active_color_part == 1)
            {
                RectF square = color_picker_square_rect(rect);
                x = clamp((pos.x - square.offset_x) / max(square.width, 1.0f), 0.0f, 1.0f);
                y = 1.0f - clamp((pos.y - square.offset_y) / max(square.height, 1.0f), 0.0f, 1.0f);
            }
            else if(color_picker_interaction_state().active_color_part == 2)
            {
                RectF bar_rect = color_picker_bar_rect(rect);
                f32 bar_t = clamp((pos.y - bar_rect.offset_y) / max(bar_rect.height, 1.0f), 0.0f, 1.0f);
                bar = axis == 0 ? bar_t : 1.0f - bar_t;
            }
            else
            {
                return;
            }
            color = color_from_picker_channels(axis, x, y, bar, color.w);
            write_color_value(*owner, color);
            sync_color_picker_numeric_state(owner_id);
            mark_value_changed(owner_id);
        }

        u32 Context::hit_test_numeric_component(const Node& node, const RectF& rect, const Float2U& pos) const
        {
            u32 value_count = numeric_value_count(node);
            if(value_count <= 1) return 0;
            f32 label_w = numeric_label_width(node, rect);
            f32 gap = 4.0f;
            f32 value_area_x = rect.offset_x + label_w;
            f32 value_area_w = max(rect.width - label_w - 8.0f, 1.0f);
            f32 component_w = max((value_area_w - gap * (f32)(value_count - 1)) / (f32)value_count, 1.0f);
            f32 rel = max(pos.x - value_area_x, 0.0f);
            return min((u32)(rel / (component_w + gap)), value_count - 1);
        }

        void Context::update_numeric_node_from_pointer(id_t id, const Float2U& pos, const Float2U* old_pos)
        {
            Node* node = find_node(id);
            if(!node || !numeric_pointer_editable(*node)) return;
            NumericBinding* binding = numeric_binding(*node);
            if(!binding) return;
            InputEditState* edit_state = get_widget_state<InputEditState>(id);
            if(edit_state && edit_state->numeric_editing) return;
            f32* f32_values = binding->f32_value;
            i32* i32_values = binding->i32_value;
            if(numeric_value_f32(*node) && !f32_values) return;
            if(numeric_value_i32(*node) && !i32_values) return;

            RectF rect;
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                if(m_submitted_desc.nodes[i].id == id)
                {
                    rect = m_layouts[i].rect;
                    break;
                }
            }
            f32 label_w = numeric_label_width(*node, rect);
            u32 value_count = numeric_value_count(*node);
            f32 gap = 4.0f;
            f32 value_area_x = rect.offset_x + label_w;
            f32 value_area_w = max(rect.width - label_w - 8.0f, 1.0f);
            f32 component_w = max((value_area_w - gap * (f32)(value_count - 1)) / (f32)value_count, 1.0f);
            u32 component = hit_test_numeric_component(*node, rect, pos);
            if(m_active_id == id && numeric_interaction_state().active_float_component != U32_MAX)
            {
                component = min(numeric_interaction_state().active_float_component, value_count - 1);
            }
            f32 component_x = value_area_x + (component_w + gap) * (f32)component;
            f32 new_value = numeric_value_f32(*node) ? f32_values[component] : (f32)i32_values[component];
            if(numeric_drag(*node))
            {
                if(!old_pos) return;
                f32 speed = binding->step_value == 0.0f ? 1.0f : binding->step_value;
                new_value += (pos.x - old_pos->x) * speed;
                if(binding->max_value > binding->min_value)
                {
                    new_value = clamp(new_value, binding->min_value, binding->max_value);
                }
            }
            else
            {
                f32 t = clamp((pos.x - component_x) / component_w, 0.0f, 1.0f);
                new_value = binding->min_value + (binding->max_value - binding->min_value) * t;
            }
            bool changed = false;
            if(numeric_value_f32(*node))
            {
                if(f32_values[component] != new_value)
                {
                    f32_values[component] = new_value;
                    changed = true;
                }
            }
            else
            {
                i32 int_value = round_to_i32(new_value);
                if(binding->max_value > binding->min_value)
                {
                    int_value = clamp(int_value, (i32)binding->min_value, (i32)binding->max_value);
                }
                if(i32_values[component] != int_value)
                {
                    i32_values[component] = int_value;
                    changed = true;
                }
            }
            if(changed)
            {
                mark_value_changed(id);
            }
        }

        bool Context::input_text_cursor_from_pointer(id_t id, const Float2U& pos, usize& out_cursor)
        {
            Node* node = find_node(id);
            String* string_value = node ? input_text_value(*node) : nullptr;
            if(!node || !input_text_node(*node) || !string_value) return false;
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                if(m_submitted_desc.nodes[i].id == id)
                {
                    RectF text_rect(m_layouts[i].rect.offset_x + 8.0f, m_layouts[i].rect.offset_y,
                        max(m_layouts[i].rect.width - 16.0f, 1.0f), m_layouts[i].rect.height);
                    f32 font_size = get_style_value_unlocked(node->style, Name("gui.input_text.font_size"), StyleValue::f32_1(16.0f)).value.x;
                    out_cursor = text_cursor_from_x(*string_value, pos.x - text_rect.offset_x, font_size, node_font_id(*node));
                    return true;
                }
            }
            return false;
        }

        bool Context::update_input_text_selection_from_pointer(id_t id, const Float2U& pos)
        {
            Node* node = find_node(id);
            String* string_value = node ? input_text_value(*node) : nullptr;
            if(!node || !input_text_node(*node) || !string_value) return false;
            usize cursor = 0;
            if(!input_text_cursor_from_pointer(id, pos, cursor)) return false;
            Ref<InputEditState> state = get_or_create_widget_state<InputEditState>(id);
            if(state->text_select_anchor == USIZE_MAX)
            {
                state->text_select_anchor = clamp_utf8_cursor(*string_value, state->text_cursor);
            }
            state->text_cursor = cursor;
            state->text_cursor_blink_start = m_time;
            return true;
        }

        bool Context::numeric_text_cursor_from_pointer(id_t id, const Float2U& pos, usize& out_cursor)
        {
            Node* node = find_node(id);
            if(!node || !numeric_text_editable(*node)) return false;
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                if(m_submitted_desc.nodes[i].id == id)
                {
                    Ref<InputEditState> state = get_or_create_widget_state<InputEditState>(id);
                    RectF component = numeric_component_rect(*node, m_layouts[i].rect, state->numeric_edit_component);
                    RectF text_rect(component.offset_x + 6.0f, component.offset_y, max(component.width - 12.0f, 1.0f), component.height);
                    f32 font_size = get_style_value_unlocked(node->style, Name("gui.numeric.font_size"), StyleValue::f32_1(15.0f)).value.x;
                    out_cursor = text_cursor_from_x(state->numeric_edit_text, pos.x - text_rect.offset_x, font_size, node_font_id(*node));
                    return true;
                }
            }
            return false;
        }

        bool Context::update_numeric_text_selection_from_pointer(id_t id, const Float2U& pos)
        {
            Node* node = find_node(id);
            if(!node || !numeric_text_editable(*node)) return false;
            Ref<InputEditState> state = get_or_create_widget_state<InputEditState>(id);
            if(!state->numeric_editing) return false;
            usize cursor = 0;
            if(!numeric_text_cursor_from_pointer(id, pos, cursor)) return false;
            if(state->text_select_anchor == USIZE_MAX)
            {
                state->text_select_anchor = clamp_utf8_cursor(state->numeric_edit_text, state->text_cursor);
            }
            state->text_cursor = cursor;
            state->text_cursor_blink_start = m_time;
            return true;
        }

        void Context::begin_numeric_text_edit(id_t id, const Float2U& pos, u32 component, bool select_all)
        {
            Node* node = find_node(id);
            if(!node || !numeric_text_editable(*node)) return;
            Ref<InputEditState> state = get_or_create_widget_state<InputEditState>(id);
            state->numeric_edit_component = min(component, numeric_value_count(*node) - 1);
            state->numeric_edit_text = numeric_value_text(*node, state->numeric_edit_component);
            state->numeric_editing = true;
            state->text_select_anchor = USIZE_MAX;
            state->text_selecting = true;
            state->text_cursor = state->numeric_edit_text.size();
            if(select_all)
            {
                state->text_select_anchor = 0;
            }
            else
            {
                numeric_text_cursor_from_pointer(id, pos, state->text_cursor);
                state->text_select_anchor = state->text_cursor;
            }
            state->text_cursor_blink_start = m_time;
        }

        void Context::process_input_events()
        {
            m_pointer_delta = Float2U(0.0f);
#ifdef LUNA_GUI_ENABLE_DEBUG
            m_debug_input_events.clear();
            for(const InputEvent& event : m_input_events)
            {
                DebugInputEventInfo debug_event;
                debug_event.event = event;
                debug_event.hovered_before = m_hovered_id;
                debug_event.active_before = m_active_id;
                debug_event.focused_before = m_focused_id;
                m_debug_input_events.push_back(move(debug_event));
            }
#endif
            auto update_pointer_position = [&](const Float2U& position)
            {
                m_pointer_delta.x += position.x - m_pointer_pos.x;
                m_pointer_delta.y += position.y - m_pointer_pos.y;
                m_pointer_pos = position;
            };
            auto clear_text_edit_state_for_id = [&](id_t id)
            {
                if(InputEditState* state = get_widget_state<InputEditState>(id))
                {
                    clear_text_edit_state(*state);
                }
            };
            auto set_interaction_down = [&](id_t id)
            {
                Ref<InteractionState> state = get_or_create_widget_state<InteractionState>(id);
                state->pointer_down = true;
                state->active = true;
                state->focused = true;
            };
            auto clear_interaction_active = [&](id_t id)
            {
                if(InteractionState* state = get_widget_state<InteractionState>(id))
                {
                    state->pointer_down = false;
                    state->active = false;
                }
                if(InputEditState* input_state = get_widget_state<InputEditState>(id))
                {
                    input_state->text_selecting = false;
                }
            };

            for(const InputEvent& e : m_input_events)
            {
                if(e.type == InputEventType::pointer_enter)
                {
                    m_pointer_inside = true;
                    update_pointer_position(e.position);
                }
                else if(e.type == InputEventType::pointer_leave)
                {
                    m_pointer_inside = false;
                    m_hovered_id = 0;
                }
                else if(e.type == InputEventType::pointer_move)
                {
                    Float2U old_pos = m_pointer_pos;
                    m_pointer_inside = true;
                    update_pointer_position(e.position);
                    if(m_drag_drop.candidate_source_id && !m_drag_drop.active)
                    {
                        f32 dx = e.position.x - m_drag_drop.start_pos.x;
                        f32 dy = e.position.y - m_drag_drop.start_pos.y;
                        if(dx * dx + dy * dy >= 16.0f)
                        {
                            start_drag_drop(m_drag_drop.candidate_source_id, m_drag_drop.candidate_type);
                        }
                    }
                    if(dock_interaction_state().active_dock_split_space_id)
                    {
                        update_dock_splitter_from_pointer(e.position);
                    }
                    else if(dock_interaction_state().active_dock_panel_id)
                    {
                        update_dock_panel_from_pointer(e.position);
                    }
                    else if(scrollbar_interaction_state().active_scrollbar_id)
                    {
                        update_scrollbar_from_pointer(e.position);
                    }
                    else if(table_resize_interaction_state().active_table_resize_id)
                    {
                        update_table_resize_from_pointer(e.position);
                    }
                    else if(tab_interaction_state().active_tab_scroll_id)
                    {
                    }
                    else if(tab_interaction_state().active_tab_item_id)
                    {
                        if(tab_interaction_state().active_tab_reorder_allowed)
                        {
                            f32 dx = e.position.x - tab_interaction_state().active_tab_start_pos.x;
                            f32 dy = e.position.y - tab_interaction_state().active_tab_start_pos.y;
                            if(!tab_interaction_state().active_tab_reordering && dx * dx + dy * dy >= 16.0f)
                            {
                                tab_interaction_state().active_tab_reordering = true;
                                select_tab_item(tab_interaction_state().active_tab_bar_id, tab_interaction_state().active_tab_item_id);
                            }
                            if(tab_interaction_state().active_tab_reordering && reorder_tab_item_from_pointer(tab_interaction_state().active_tab_bar_id, tab_interaction_state().active_tab_item_id, e.position))
                            {
                                m_layout_dirty = true;
                            }
                        }
                    }
                    else if(m_active_id)
                    {
                        Node* active_node = find_node(m_active_id);
                        if(active_node && color_picker_node(*active_node))
                        {
                            update_color_picker_from_pointer(m_active_id, e.position);
                        }
                        else
                        {
                            update_input_text_selection_from_pointer(m_active_id, e.position);
                            update_numeric_text_selection_from_pointer(m_active_id, e.position);
                            if(numeric_interaction_state().active_numeric_defer_until_drag)
                            {
                                f32 dx = e.position.x - numeric_interaction_state().active_numeric_start_pos.x;
                                f32 dy = e.position.y - numeric_interaction_state().active_numeric_start_pos.y;
                                if(dx * dx + dy * dy >= 16.0f)
                                {
                                    Float2U start_pos = numeric_interaction_state().active_numeric_start_pos;
                                    numeric_interaction_state().active_numeric_defer_until_drag = false;
                                    update_numeric_node_from_pointer(m_active_id, e.position, &start_pos);
                                }
                            }
                            else
                            {
                                update_numeric_node_from_pointer(m_active_id, e.position, &old_pos);
                            }
                        }
                    }
                }
                else if(e.type == InputEventType::pointer_down)
                {
                    m_pointer_inside = true;
                    update_pointer_position(e.position);
                    if((u32)e.button < 5)
                    {
                        m_pointer_button_down[(u32)e.button] = true;
                    }
                    numeric_interaction_state().active_float_component = U32_MAX;
                    numeric_interaction_state().active_numeric_defer_until_drag = false;
                    color_picker_interaction_state().active_color_part = 0;
                    id_t old_focused_id = m_focused_id;
                    if(close_popups_for_pointer_down(e.position))
                    {
                        if(old_focused_id)
                        {
                            clear_text_edit_state_for_id(old_focused_id);
                        }
                        m_active_id = 0;
                        numeric_interaction_state().active_float_component = U32_MAX;
                        numeric_interaction_state().active_numeric_defer_until_drag = false;
                        color_picker_interaction_state().active_color_part = 0;
                        continue;
                    }
                    if(e.button != PointerButton::left)
                    {
                        id_t target = hit_test(e.position);
                        if(target)
                        {
                            m_focused_id = target;
                            if(old_focused_id && old_focused_id != target)
                            {
                                clear_text_edit_state_for_id(old_focused_id);
                            }
                        }
                        continue;
                    }
                    id_t split_space_id = 0;
                    u32 split_node_index = U32_MAX;
                    DockSplitAxis split_axis = DockSplitAxis::x;
                    if(hit_test_dock_splitter(e.position, split_space_id, split_node_index, split_axis))
                    {
                        m_active_id = split_space_id;
                        m_focused_id = split_space_id;
                        dock_interaction_state().active_dock_split_space_id = split_space_id;
                        dock_interaction_state().active_dock_split_node = split_node_index;
                        dock_interaction_state().active_dock_split_axis = split_axis;
                        dock_interaction_state().active_dock_split_start_pos = e.position;
                        Ref<DockSpaceState> dock_state = get_or_create_widget_state<DockSpaceState>(split_space_id);
                        if(split_node_index < dock_state->dock_nodes.size())
                        {
                            dock_interaction_state().active_dock_split_start_ratio = dock_state->dock_nodes[split_node_index].split_ratio;
                        }
                        set_interaction_down(split_space_id);
                        continue;
                    }
                    id_t tab_space_id = 0;
                    id_t tab_panel_id = 0;
                    u32 tab_leaf_index = U32_MAX;
                    if(hit_test_dock_panel_tab(e.position, tab_space_id, tab_panel_id, tab_leaf_index))
                    {
                        Ref<DockSpaceState> dock_state = get_or_create_widget_state<DockSpaceState>(tab_space_id);
                        if(tab_leaf_index < dock_state->dock_nodes.size())
                        {
                            dock_state->dock_nodes[tab_leaf_index].selected_tab = tab_panel_id;
                        }
                        m_active_id = tab_panel_id;
                        m_focused_id = tab_panel_id;
                        dock_interaction_state().active_dock_space_id = tab_space_id;
                        dock_interaction_state().active_dock_panel_id = tab_panel_id;
                        dock_interaction_state().active_dock_panel_resize = false;
                        dock_interaction_state().active_dock_panel_close = false;
                        dock_interaction_state().active_dock_panel_title_drag = true;
                        dock_interaction_state().active_dock_panel_was_floating = false;
                        dock_interaction_state().active_dock_panel_undocked = false;
                        dock_interaction_state().active_dock_panel_resize_neighbor_id = 0;
                        dock_interaction_state().active_dock_panel_start_neighbor_height = 0.0f;
                        DockPanelPersistentState* panel_state = find_dock_panel_state(tab_space_id, tab_panel_id);
                        dock_interaction_state().active_dock_panel_restore_rect = panel_state ? panel_state->rect : RectF(0.0f, 0.0f, 320.0f, 220.0f);
                        if(tab_leaf_index < dock_state->dock_nodes.size())
                        {
                            dock_interaction_state().active_dock_panel_start_rect = dock_state->dock_nodes[tab_leaf_index].rect;
                            Node* tab_node = find_node(tab_panel_id);
                            DockPanelStyle style;
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
                            dock_interaction_state().active_dock_panel_start_title_rect = dock_panel_title_rect(dock_interaction_state().active_dock_panel_start_rect, style);
                            dock_interaction_state().active_dock_panel_grab_offset = Float2U(
                                e.position.x - dock_interaction_state().active_dock_panel_start_rect.offset_x,
                                e.position.y - dock_interaction_state().active_dock_panel_start_rect.offset_y);
                        }
                        set_interaction_down(tab_panel_id);
                        m_layout_dirty = true;
                        continue;
                    }
                    id_t dock_space_id = 0;
                    id_t dock_panel_id = 0;
                    bool dock_resize = false;
                    bool dock_close = false;
                    if(hit_test_dock_panel_chrome(e.position, dock_space_id, dock_panel_id, dock_resize, dock_close))
                    {
                        m_active_id = dock_panel_id;
                        m_focused_id = dock_panel_id;
                        dock_interaction_state().active_dock_space_id = dock_space_id;
                        dock_interaction_state().active_dock_panel_id = dock_panel_id;
                        dock_interaction_state().active_dock_panel_resize = dock_resize;
                        dock_interaction_state().active_dock_panel_close = dock_close;
                        dock_interaction_state().active_dock_panel_title_drag = !dock_resize && !dock_close;
                        dock_interaction_state().active_dock_panel_was_floating = false;
                        dock_interaction_state().active_dock_panel_undocked = false;
                        dock_interaction_state().active_dock_panel_resize_neighbor_id = 0;
                        dock_interaction_state().active_dock_panel_start_neighbor_height = 0.0f;
                        raise_dock_panel(dock_space_id, dock_panel_id);
                        DockPanelPersistentState* panel_state = find_dock_panel_state(dock_space_id, dock_panel_id);
                        if(panel_state && !dock_close)
                        {
                            dock_interaction_state().active_dock_panel_start_rect = panel_state->rect;
                            dock_interaction_state().active_dock_panel_restore_rect = panel_state->rect;
                        }
                        else
                        {
                            dock_interaction_state().active_dock_panel_restore_rect = RectF(0.0f, 0.0f, 320.0f, 220.0f);
                        }
                        for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
                        {
                            if(m_submitted_desc.nodes[i].id == dock_panel_id)
                            {
                                dock_interaction_state().active_dock_panel_start_rect = m_layouts[i].dock_panel_rect;
                                dock_interaction_state().active_dock_panel_start_title_rect = m_layouts[i].dock_panel_title_rect;
                                dock_interaction_state().active_dock_panel_was_floating = m_layouts[i].dock_panel_floating;
                                dock_interaction_state().active_dock_panel_grab_offset = Float2U(
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
                                        dock_interaction_state().active_dock_panel_resize_neighbor_id = m_submitted_desc.nodes[sibling].id;
                                        dock_interaction_state().active_dock_panel_start_neighbor_height = m_layouts[sibling].dock_panel_rect.height;
                                        break;
                                    }
                                }
                                break;
                            }
                        }
                        set_interaction_down(dock_panel_id);
                        m_layout_dirty = true;
                        continue;
                    }
                    id_t scrollbar_id = 0;
                    bool scrollbar_vertical = false;
                    RectF scrollbar_thumb;
                    if(hit_test_scrollbar(e.position, scrollbar_id, scrollbar_vertical, scrollbar_thumb))
                    {
                        m_active_id = scrollbar_id;
                        m_focused_id = scrollbar_id;
                        if(old_focused_id && old_focused_id != scrollbar_id)
                        {
                            clear_text_edit_state_for_id(old_focused_id);
                        }
                        scrollbar_interaction_state().active_scrollbar_id = scrollbar_id;
                        scrollbar_interaction_state().active_scrollbar_vertical = scrollbar_vertical;
                        if(point_in_rect(e.position, scrollbar_thumb))
                        {
                            scrollbar_interaction_state().active_scrollbar_grab_offset = scrollbar_vertical ?
                                e.position.y - scrollbar_thumb.offset_y :
                                e.position.x - scrollbar_thumb.offset_x;
                        }
                        else
                        {
                            scrollbar_interaction_state().active_scrollbar_grab_offset = scrollbar_vertical ?
                                scrollbar_thumb.height * 0.5f :
                                scrollbar_thumb.width * 0.5f;
                        }
                        set_interaction_down(scrollbar_id);
                        update_scrollbar_from_pointer(e.position);
                        continue;
                    }
                    id_t resize_table = 0;
                    bool resize_column = false;
                    u32 resize_index = U32_MAX;
                    if(hit_test_table_separator(e.position, resize_table, resize_column, resize_index))
                    {
                        m_active_id = resize_table;
                        m_focused_id = resize_table;
                        if(old_focused_id && old_focused_id != resize_table)
                        {
                            clear_text_edit_state_for_id(old_focused_id);
                        }
                        table_resize_interaction_state().active_table_resize_id = resize_table;
                        table_resize_interaction_state().active_table_resize_column = resize_column;
                        table_resize_interaction_state().active_table_resize_index = resize_index;
                        set_interaction_down(resize_table);
                        continue;
                    }
                    id_t tab_scroll_bar_id = 0;
                    bool tab_scroll_left = false;
                    if(hit_test_tab_scroll_button(e.position, tab_scroll_bar_id, tab_scroll_left))
                    {
                        m_active_id = tab_scroll_bar_id;
                        m_focused_id = tab_scroll_bar_id;
                        if(old_focused_id && old_focused_id != tab_scroll_bar_id)
                        {
                            clear_text_edit_state_for_id(old_focused_id);
                        }
                        tab_interaction_state().active_tab_scroll_id = tab_scroll_bar_id;
                        tab_interaction_state().active_tab_scroll_left = tab_scroll_left;
                        set_interaction_down(tab_scroll_bar_id);
                        scroll_tab_bar(tab_scroll_bar_id, tab_scroll_left ? -96.0f : 96.0f);
                        continue;
                    }
                    id_t tab_bar_id = 0;
                    id_t tab_item_id = 0;
                    bool tab_close = false;
                    if(hit_test_tab_header(e.position, tab_bar_id, tab_item_id, tab_close))
                    {
                        m_active_id = tab_item_id;
                        m_focused_id = tab_item_id;
                        if(old_focused_id && old_focused_id != tab_item_id)
                        {
                            clear_text_edit_state_for_id(old_focused_id);
                        }
                        tab_interaction_state().active_tab_bar_id = tab_bar_id;
                        tab_interaction_state().active_tab_item_id = tab_item_id;
                        tab_interaction_state().active_tab_close = tab_close;
                        tab_interaction_state().active_tab_start_pos = e.position;
                        tab_interaction_state().active_tab_reordering = false;
                        tab_interaction_state().active_tab_reorder_allowed = false;
                        Node* tab_bar = find_node(tab_bar_id);
                        Node* tab_item = find_node(tab_item_id);
                        TabBarNode* tab_bar_typed = tab_bar ? tab_bar_node(*tab_bar) : nullptr;
                        TabItemNode* tab_item_typed = tab_item ? tab_item_node(*tab_item) : nullptr;
                        if(tab_bar_typed && tab_item_typed)
                        {
                            tab_interaction_state().active_tab_reorder_allowed = test_flags(tab_bar_typed->flags, TabBarFlag::reorderable) &&
                                !tab_close &&
                                !test_flags(tab_item_typed->flags, TabItemFlag::button) &&
                                !test_flags(tab_item_typed->flags, TabItemFlag::no_reorder);
                        }
                        set_interaction_down(tab_item_id);
                        continue;
                    }
                    id_t target = hit_test(e.position);
                    Name drag_drop_type;
                    id_t drag_drop_source = hit_test_drag_drop_source(e.position, drag_drop_type);
                    m_drag_drop.candidate_source_id = drag_drop_source;
                    m_drag_drop.candidate_type = drag_drop_type;
                    m_drag_drop.start_pos = e.position;
                    m_active_id = target;
                    m_focused_id = target;
                    if(old_focused_id && old_focused_id != target)
                    {
                        clear_text_edit_state_for_id(old_focused_id);
                    }
                    if(target)
                    {
                        set_interaction_down(target);
                        Node* node = find_node(target);
                        if(node && input_text_node(*node) && input_text_value(*node))
                        {
                            Ref<InputEditState> input_state = get_or_create_widget_state<InputEditState>(target);
                            usize cursor = 0;
                            input_text_cursor_from_pointer(target, e.position, cursor);
                            input_state->text_cursor = cursor;
                            input_state->text_select_anchor = cursor;
                            input_state->text_selecting = true;
                            input_state->text_cursor_blink_start = m_time;
                        }
                        if(node && color_picker_node(*node))
                        {
                            RectF rect;
                            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
                            {
                                if(m_submitted_desc.nodes[i].id == target)
                                {
                                    rect = m_layouts[i].rect;
                                    break;
                                }
                            }
                            if(point_in_rect(e.position, color_picker_square_rect(rect)))
                            {
                                color_picker_interaction_state().active_color_part = 1;
                                update_color_picker_from_pointer(target, e.position);
                            }
                            else if(point_in_rect(e.position, color_picker_bar_rect(rect)))
                            {
                                color_picker_interaction_state().active_color_part = 2;
                                update_color_picker_from_pointer(target, e.position);
                            }
                            else if(point_in_rect(e.position, color_picker_original_rect(rect)))
                            {
                                color_picker_interaction_state().active_color_part = 3;
                                update_color_picker_from_pointer(target, e.position);
                            }
                        }
                        if(node && numeric_node(*node))
                        {
                            Ref<InputEditState> input_state = get_or_create_widget_state<InputEditState>(target);
                            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
                            {
                                if(m_submitted_desc.nodes[i].id == target)
                                {
                                    numeric_interaction_state().active_float_component = hit_test_numeric_component(*node, m_layouts[i].rect, e.position);
                                    break;
                                }
                            }
                            if(numeric_text_editable(*node) && !numeric_pointer_editable(*node))
                            {
                                begin_numeric_text_edit(target, e.position, numeric_interaction_state().active_float_component == U32_MAX ? 0 : numeric_interaction_state().active_float_component, false);
                            }
                            else if(input_state->numeric_editing && numeric_text_editable(*node))
                            {
                                begin_numeric_text_edit(target, e.position, numeric_interaction_state().active_float_component == U32_MAX ? 0 : numeric_interaction_state().active_float_component, false);
                            }
                            else if(numeric_text_editable(*node))
                            {
                                numeric_interaction_state().active_numeric_defer_until_drag = true;
                                numeric_interaction_state().active_numeric_start_pos = e.position;
                            }
                        }
                        InputEditState* input_state = get_widget_state<InputEditState>(target);
                        if((!input_state || !input_state->numeric_editing) && !numeric_interaction_state().active_numeric_defer_until_drag)
                        {
                            update_numeric_node_from_pointer(target, e.position);
                        }
                    }
                }
                else if(e.type == InputEventType::pointer_up)
                {
                    m_pointer_inside = true;
                    update_pointer_position(e.position);
                    if((u32)e.button < 5)
                    {
                        m_pointer_button_down[(u32)e.button] = false;
                    }
                    if(e.button == PointerButton::right)
                    {
                        id_t target = hit_test(e.position);
                        if(target)
                        {
                            Ref<ItemQueryState> result = get_or_create_query_state(target);
                            result->states.insert_or_assign(Name("gui.right_clicked"), Any(true));
                            get_or_create_widget_state<InteractionState>(target)->last_right_click_time = m_time;
                        }
                        continue;
                    }
                    if(e.button != PointerButton::left)
                    {
                        continue;
                    }
                    if(m_drag_drop.active)
                    {
                        id_t drop_target = hit_test_drag_drop_target(m_drag_drop.type, e.position);
                        deliver_drag_drop_payload(drop_target);
                        if(m_active_id)
                        {
                            clear_interaction_active(m_active_id);
                        }
                        clear_drag_drop();
                        m_active_id = 0;
                        numeric_interaction_state().active_float_component = U32_MAX;
                        numeric_interaction_state().active_numeric_defer_until_drag = false;
                        continue;
                    }
                    m_drag_drop.candidate_source_id = 0;
                    m_drag_drop.candidate_type.reset();
                    if(dock_interaction_state().active_dock_split_space_id)
                    {
                        clear_interaction_active(dock_interaction_state().active_dock_split_space_id);
                        dock_interaction_state().active_dock_split_space_id = 0;
                        dock_interaction_state().active_dock_split_node = U32_MAX;
                        m_active_id = 0;
                        numeric_interaction_state().active_numeric_defer_until_drag = false;
                        continue;
                    }
                    if(dock_interaction_state().active_dock_panel_id)
                    {
                        if(dock_interaction_state().active_dock_panel_close)
                        {
                            DockPanelPersistentState* panel_state = find_dock_panel_state(dock_interaction_state().active_dock_space_id, dock_interaction_state().active_dock_panel_id);
                            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
                            {
                                Node& node = m_submitted_desc.nodes[i];
                                if(node.id != dock_interaction_state().active_dock_panel_id) continue;
                                if(point_in_rect(e.position, m_layouts[i].dock_panel_close_rect))
                                {
                                    const DockPanelAttachment* attachment = nullptr;
                                    if(node.parent != U32_MAX && node.parent < m_submitted_desc.nodes.size())
                                    {
                                        attachment = dock_panel_attachment(m_submitted_desc.nodes[node.parent], (u32)i);
                                    }
                                    bool* panel_open = attachment ? attachment->open : nullptr;
                                    if(panel_open)
                                    {
                                        *panel_open = false;
                                    }
                                    else if(panel_state)
                                    {
                                        panel_state->closed = true;
                                    }
                                    Ref<ItemQueryState> result = get_or_create_query_state(node.id);
                                    result->states.insert_or_assign(Name("gui.value_changed"), Any(true));
                                    result->states.insert_or_assign(Name("gui.open"), Any(false));
                                    m_layout_dirty = true;
                                }
                                break;
                            }
                        }
                        else if(dock_interaction_state().active_dock_panel_title_drag && !dock_interaction_state().active_dock_panel_resize)
                        {
                            id_t target_space_id = 0;
                            u32 target_leaf = U32_MAX;
                            DockDropDirection drop_direction = DockDropDirection::none;
                            DockPanelPersistentState* panel_state = find_dock_panel_state(dock_interaction_state().active_dock_space_id, dock_interaction_state().active_dock_panel_id);
                            if(panel_state && panel_state->mode == DockPanelMode::floating &&
                                find_dock_drop_target(dock_interaction_state().active_dock_panel_id, e.position, target_space_id, target_leaf, drop_direction) &&
                                target_space_id == dock_interaction_state().active_dock_space_id && drop_direction != DockDropDirection::none)
                            {
                                Ref<DockSpaceState> dock_state = get_or_create_widget_state<DockSpaceState>(target_space_id);
                                dock_tree_dock_panel(*dock_state, dock_interaction_state().active_dock_panel_id, target_leaf, drop_direction);
                                panel_state->mode = DockPanelMode::docking;
                                panel_state->closed = false;
                                Ref<ItemQueryState> result = get_or_create_query_state(dock_interaction_state().active_dock_panel_id);
                                result->states.insert_or_assign(Name("gui.value_changed"), Any(true));
                                result->states.insert_or_assign(Name("gui.open"), Any(true));
                                m_layout_dirty = true;
                            }
                        }
                        clear_interaction_active(dock_interaction_state().active_dock_panel_id);
                        dock_interaction_state().active_dock_space_id = 0;
                        dock_interaction_state().active_dock_panel_id = 0;
                        dock_interaction_state().active_dock_panel_resize = false;
                        dock_interaction_state().active_dock_panel_close = false;
                        dock_interaction_state().active_dock_panel_title_drag = false;
                        dock_interaction_state().active_dock_panel_was_floating = false;
                        dock_interaction_state().active_dock_panel_undocked = false;
                        dock_interaction_state().active_dock_panel_resize_neighbor_id = 0;
                        dock_interaction_state().active_dock_panel_start_neighbor_height = 0.0f;
                        m_active_id = 0;
                        numeric_interaction_state().active_float_component = U32_MAX;
                        numeric_interaction_state().active_numeric_defer_until_drag = false;
                        continue;
                    }
                    if(scrollbar_interaction_state().active_scrollbar_id)
                    {
                        clear_interaction_active(scrollbar_interaction_state().active_scrollbar_id);
                        scrollbar_interaction_state().active_scrollbar_id = 0;
                        scrollbar_interaction_state().active_scrollbar_vertical = false;
                        scrollbar_interaction_state().active_scrollbar_grab_offset = 0.0f;
                        m_active_id = 0;
                        numeric_interaction_state().active_float_component = U32_MAX;
                        continue;
                    }
                    if(table_resize_interaction_state().active_table_resize_id)
                    {
                        clear_interaction_active(table_resize_interaction_state().active_table_resize_id);
                        table_resize_interaction_state().active_table_resize_id = 0;
                        table_resize_interaction_state().active_table_resize_column = false;
                        table_resize_interaction_state().active_table_resize_index = U32_MAX;
                        m_active_id = 0;
                        numeric_interaction_state().active_float_component = U32_MAX;
                        continue;
                    }
                    if(tab_interaction_state().active_tab_scroll_id)
                    {
                        clear_interaction_active(tab_interaction_state().active_tab_scroll_id);
                        tab_interaction_state().active_tab_scroll_id = 0;
                        tab_interaction_state().active_tab_scroll_left = false;
                        m_active_id = 0;
                        numeric_interaction_state().active_float_component = U32_MAX;
                        continue;
                    }
                    if(tab_interaction_state().active_tab_item_id)
                    {
                        id_t tab_bar_id = 0;
                        id_t tab_item_id = 0;
                        bool tab_close = false;
                        bool hit_tab = hit_test_tab_header(e.position, tab_bar_id, tab_item_id, tab_close);
                        if(hit_tab && tab_item_id == tab_interaction_state().active_tab_item_id && !tab_interaction_state().active_tab_reordering)
                        {
                            Ref<ItemQueryState> item_result = get_or_create_query_state(tab_item_id);
                            item_result->states.insert_or_assign(Name("gui.clicked"), Any(true));
                            Ref<InteractionState> item_state = get_or_create_widget_state<InteractionState>(tab_item_id);
                            bool dbl = (m_time - item_state->last_click_time) <= 0.4;
                            item_result->states.insert_or_assign(Name("gui.double_clicked"), Any(dbl));
                            item_state->last_click_time = m_time;
                            for(Node& node : m_submitted_desc.nodes)
                            {
                                if(node.id != tab_item_id || !tab_item_layout(node)) continue;
                                TabItemNode* tab = tab_item_node(node);
                                luassert(tab);
                                bool* open = tab->open;
                                if((tab_interaction_state().active_tab_close || tab_close) && open && !test_flags(tab->flags, TabItemFlag::no_close_button))
                                {
                                    *open = false;
                                    item_result->states.insert_or_assign(Name("gui.open"), Any(false));
                                    item_result->states.insert_or_assign(Name("gui.value_changed"), Any(true));
                                    m_layout_dirty = true;
                                    Ref<TabBarState> bar_state = get_or_create_widget_state<TabBarState>(tab_bar_id);
                                    if(bar_state->tab_selected_id == tab_item_id)
                                    {
                                        bar_state->tab_selected_id = fallback_tab_item(tab_bar_id, tab_item_id);
                                        Ref<ItemQueryState> bar_result = get_or_create_query_state(tab_bar_id);
                                        bar_result->states.insert_or_assign(Name("gui.value_changed"), Any(true));
                                        m_layout_dirty = true;
                                    }
                                }
                                else if(!test_flags(tab->flags, TabItemFlag::button))
                                {
                                    select_tab_item(tab_bar_id, tab_item_id);
                                }
                                break;
                            }
                        }
                        clear_interaction_active(tab_interaction_state().active_tab_item_id);
                        tab_interaction_state().active_tab_bar_id = 0;
                        tab_interaction_state().active_tab_item_id = 0;
                        tab_interaction_state().active_tab_close = false;
                        tab_interaction_state().active_tab_reorder_allowed = false;
                        tab_interaction_state().active_tab_reordering = false;
                        m_active_id = 0;
                        numeric_interaction_state().active_float_component = U32_MAX;
                        continue;
                    }
                    id_t target = hit_test(e.position);
                    id_t target_dock_space = 0;
                    id_t target_dock_panel = 0;
                    if(hit_test_dock_panel(e.position, target_dock_space, target_dock_panel))
                    {
                        raise_dock_panel(target_dock_space, target_dock_panel);
                    }
                    if(target && target == m_active_id)
                    {
                        Ref<ItemQueryState> result = get_or_create_query_state(target);
                        result->states.insert_or_assign(Name("gui.clicked"), Any(true));
                        Ref<InteractionState> state = get_or_create_widget_state<InteractionState>(target);
                        bool dbl = (m_time - state->last_click_time) <= 0.4;
                        result->states.insert_or_assign(Name("gui.double_clicked"), Any(dbl));
                        state->last_click_time = m_time;
                        for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
                        {
                            Node& node = m_submitted_desc.nodes[i];
                            if(node.id != target) continue;
                            if(numeric_node(node) && numeric_text_editable(node) && dbl)
                            {
                                begin_numeric_text_edit(target, e.position, numeric_interaction_state().active_float_component == U32_MAX ? 0 : numeric_interaction_state().active_float_component, true);
                            }
                            else
                            {
                                ContextNodeInputContext node_input_context;
                                node_input_context.context = this;
                                node_input_context.result = result.get();
                                node_input_context.node_id = node.id;
                                node_input_context.current_pointer_position = e.position;
                                node_input_context.current_rect = m_layouts[i].rect;
                                node.on_click(node_input_context);
                            }
                            break;
                        }
                    }
                    if(m_active_id)
                    {
                        clear_interaction_active(m_active_id);
                        InputEditState* input_state = get_widget_state<InputEditState>(m_active_id);
                        if(input_state && input_state->text_select_anchor == input_state->text_cursor)
                        {
                            input_state->text_select_anchor = USIZE_MAX;
                        }
                    }
                    m_active_id = 0;
                    numeric_interaction_state().active_float_component = U32_MAX;
                    numeric_interaction_state().active_numeric_defer_until_drag = false;
                    color_picker_interaction_state().active_color_part = 0;
                }
                else if(e.type == InputEventType::pointer_wheel)
                {
                    m_pointer_inside = true;
                    update_pointer_position(e.position);
                    id_t tab_scroll_area = hit_test_tab_scroll_area(e.position);
                    if(tab_scroll_area)
                    {
                        f32 delta = e.wheel_delta.x != 0.0f ? -e.wheel_delta.x * 48.0f : -e.wheel_delta.y * 48.0f;
                        scroll_tab_bar(tab_scroll_area, delta);
                        continue;
                    }
                    id_t scroll_target = 0;
                    bool scrollbar_vertical = false;
                    RectF scrollbar_thumb;
                    if(!hit_test_scrollbar(e.position, scroll_target, scrollbar_vertical, scrollbar_thumb))
                    {
                        scroll_target = hit_test_filtered(e.position, HitTestFilter::scroll_view);
                    }
                    if(scroll_target)
                    {
                        Ref<ScrollState> state = get_or_create_widget_state<ScrollState>(scroll_target);
                        f32 old_scroll_x = state->scroll_x;
                        f32 old_scroll_y = state->scroll_y;
                        state->scroll_x -= e.wheel_delta.x * 24.0f;
                        state->scroll_y -= e.wheel_delta.y * 24.0f;
                        clamp_scroll_state(scroll_target);
                        if(state->scroll_x != old_scroll_x || state->scroll_y != old_scroll_y)
                        {
                            Ref<ItemQueryState> result = get_or_create_query_state(scroll_target);
                            result->states.insert_or_assign(Name("gui.value_changed"), Any(true));
                            m_layout_dirty = true;
                        }
                    }
                }
                else if(e.type == InputEventType::text_utf8)
                {
                    if(!m_focused_id) continue;
                    for(Node& node : m_submitted_desc.nodes)
                    {
                        if(node.id != m_focused_id) continue;
                        String* string_value = input_text_value(node);
                        if(input_text_node(node) && string_value)
                        {
                            String filtered = filter_input_text(e.text);
                            if(!filtered.empty())
                            {
                                Ref<InputEditState> state = get_or_create_widget_state<InputEditState>(node.id);
                                state->text_cursor = clamp_utf8_cursor(*string_value, state->text_cursor);
                                delete_input_text_selection(*string_value, *state);
                                string_value->insert(state->text_cursor, filtered);
                                state->text_cursor += filtered.size();
                                input_text_clear_selection(*state);
                                state->text_cursor_blink_start = m_time;
                                Ref<ItemQueryState> result = get_or_create_query_state(node.id);
                                result->states.insert_or_assign(Name("gui.value_changed"), Any(true));
                            }
                            break;
                        }
                        if(numeric_text_editable(node))
                        {
                            Ref<InputEditState> state = get_or_create_widget_state<InputEditState>(node.id);
                            if(!state->numeric_editing) break;
                            String filtered = filter_numeric_text(e.text, numeric_value_f32(node));
                            if(!filtered.empty())
                            {
                                state->text_cursor = clamp_utf8_cursor(state->numeric_edit_text, state->text_cursor);
                                delete_input_text_selection(state->numeric_edit_text, *state);
                                state->numeric_edit_text.insert(state->text_cursor, filtered);
                                state->text_cursor += filtered.size();
                                input_text_clear_selection(*state);
                                state->text_cursor_blink_start = m_time;
                                apply_numeric_edit_text(*this, node, *state);
                            }
                            break;
                        }
                    }
                }
                else if(e.type == InputEventType::key_down)
                {
                    if((u32)e.key < 256)
                    {
                        m_key_down[(u32)e.key] = true;
                    }
                    m_key_modifiers = e.modifiers;
                    if(e.key == KeyCode::esc && !m_popup_stack.open_stack.empty())
                    {
                        if(test_flags(m_popup_stack.open_stack.back().flags, PopupFlag::close_on_escape))
                        {
                            close_current_popup();
                        }
                        continue;
                    }
                    if(!m_focused_id) continue;
                    for(Node& node : m_submitted_desc.nodes)
                    {
                        if(node.id != m_focused_id) continue;
                        String* string_value = input_text_value(node);
                        bool edit_input_text = input_text_node(node) && string_value;
                        bool edit_numeric = numeric_text_editable(node);
                        if(!edit_input_text && !edit_numeric) continue;
                        Ref<InputEditState> state = get_or_create_widget_state<InputEditState>(node.id);
                        if(edit_numeric && !state->numeric_editing)
                        {
                            state->numeric_edit_component = min(state->numeric_edit_component, numeric_value_count(node) - 1);
                            state->numeric_edit_text = numeric_value_text(node, state->numeric_edit_component);
                            state->numeric_editing = true;
                        }
                        String& edit_value = edit_input_text ? *string_value : state->numeric_edit_text;
                        state->text_cursor = clamp_utf8_cursor(edit_value, state->text_cursor);
                        bool changed = false;
                        bool shortcut = has_modifier(e.modifiers, KeyModifierFlag::ctrl) || has_modifier(e.modifiers, KeyModifierFlag::system);
                        bool shift = has_modifier(e.modifiers, KeyModifierFlag::shift);
                        if(shortcut && e.key == KeyCode::c)
                        {
                            if(input_text_has_selection(edit_value, *state) && m_clipboard_io.set_text)
                            {
                                usize begin = 0;
                                usize end = 0;
                                input_text_selection_range(edit_value, *state, begin, end);
                                String selected = edit_value.substr(begin, end - begin);
                                RV clipboard_result = m_clipboard_io.set_text(selected.c_str(), selected.size(), m_clipboard_io.userdata);
                                (void)clipboard_result;
                            }
                        }
                        else if(shortcut && e.key == KeyCode::v)
                        {
                            if(m_clipboard_io.get_text)
                            {
                                String clipboard_text;
                                RV r = m_clipboard_io.get_text(clipboard_text, m_clipboard_io.userdata);
                                if(succeeded(r))
                                {
                                    String filtered = edit_input_text ? filter_input_text(clipboard_text) : filter_numeric_text(clipboard_text, numeric_value_f32(node));
                                    if(!filtered.empty() || input_text_has_selection(edit_value, *state))
                                    {
                                        delete_input_text_selection(edit_value, *state);
                                        edit_value.insert(state->text_cursor, filtered);
                                        state->text_cursor += filtered.size();
                                        input_text_clear_selection(*state);
                                        state->text_cursor_blink_start = m_time;
                                        changed = edit_numeric ? apply_numeric_edit_text(*this, node, *state) : true;
                                    }
                                }
                            }
                        }
                        else if(e.key == KeyCode::backspace)
                        {
                            if(input_text_has_selection(edit_value, *state))
                            {
                                changed = delete_input_text_selection(edit_value, *state);
                            }
                            else
                            {
                                usize old_size = edit_value.size();
                                erase_previous_utf8_codepoint(edit_value, state->text_cursor);
                                changed = edit_value.size() != old_size;
                            }
                            state->text_cursor_blink_start = m_time;
                            if(edit_numeric) changed = apply_numeric_edit_text(*this, node, *state);
                        }
                        else if(e.key == KeyCode::del)
                        {
                            if(input_text_has_selection(edit_value, *state))
                            {
                                changed = delete_input_text_selection(edit_value, *state);
                            }
                            else
                            {
                                usize old_size = edit_value.size();
                                erase_utf8_codepoint_at(edit_value, state->text_cursor);
                                changed = edit_value.size() != old_size;
                            }
                            state->text_cursor_blink_start = m_time;
                            if(edit_numeric) changed = apply_numeric_edit_text(*this, node, *state);
                        }
                        else if(e.key == KeyCode::left)
                        {
                            if(shift && state->text_select_anchor == USIZE_MAX)
                            {
                                state->text_select_anchor = state->text_cursor;
                            }
                            if(!shift && input_text_has_selection(edit_value, *state))
                            {
                                usize begin = 0;
                                usize end = 0;
                                input_text_selection_range(edit_value, *state, begin, end);
                                state->text_cursor = begin;
                                input_text_clear_selection(*state);
                            }
                            else
                            {
                                state->text_cursor = previous_utf8_cursor(edit_value, state->text_cursor);
                                if(!shift) input_text_clear_selection(*state);
                            }
                            state->text_cursor_blink_start = m_time;
                        }
                        else if(e.key == KeyCode::right)
                        {
                            if(shift && state->text_select_anchor == USIZE_MAX)
                            {
                                state->text_select_anchor = state->text_cursor;
                            }
                            if(!shift && input_text_has_selection(edit_value, *state))
                            {
                                usize begin = 0;
                                usize end = 0;
                                input_text_selection_range(edit_value, *state, begin, end);
                                state->text_cursor = end;
                                input_text_clear_selection(*state);
                            }
                            else
                            {
                                state->text_cursor = next_utf8_cursor(edit_value, state->text_cursor);
                                if(!shift) input_text_clear_selection(*state);
                            }
                            state->text_cursor_blink_start = m_time;
                        }
                        else if(e.key == KeyCode::enter || e.key == KeyCode::esc)
                        {
                            m_focused_id = 0;
                            if(InteractionState* interaction = get_widget_state<InteractionState>(node.id))
                            {
                                interaction->focused = false;
                            }
                            input_text_clear_selection(*state);
                            state->numeric_editing = false;
                        }
                        if(changed && edit_input_text)
                        {
                            Ref<ItemQueryState> result = get_or_create_query_state(node.id);
                            result->states.insert_or_assign(Name("gui.value_changed"), Any(true));
                        }
                        break;
                    }
                }
                else if(e.type == InputEventType::key_up)
                {
                    if((u32)e.key < 256)
                    {
                        m_key_down[(u32)e.key] = false;
                    }
                    m_key_modifiers = e.modifiers;
                }
                else if(e.type == InputEventType::blur)
                {
                    if(m_focused_id)
                    {
                        clear_text_edit_state_for_id(m_focused_id);
                    }
                    m_focused_id = 0;
                    m_active_id = 0;
                    numeric_interaction_state().active_float_component = U32_MAX;
                    numeric_interaction_state().active_numeric_defer_until_drag = false;
                    table_resize_interaction_state().active_table_resize_id = 0;
                    table_resize_interaction_state().active_table_resize_column = false;
                    table_resize_interaction_state().active_table_resize_index = U32_MAX;
                    scrollbar_interaction_state().active_scrollbar_id = 0;
                    scrollbar_interaction_state().active_scrollbar_vertical = false;
                    scrollbar_interaction_state().active_scrollbar_grab_offset = 0.0f;
                    tab_interaction_state().active_tab_bar_id = 0;
                    tab_interaction_state().active_tab_item_id = 0;
                    tab_interaction_state().active_tab_close = false;
                    tab_interaction_state().active_tab_reorder_allowed = false;
                    tab_interaction_state().active_tab_reordering = false;
                    tab_interaction_state().active_tab_scroll_id = 0;
                    tab_interaction_state().active_tab_scroll_left = false;
                    dock_interaction_state().active_dock_space_id = 0;
                    dock_interaction_state().active_dock_panel_id = 0;
                    dock_interaction_state().active_dock_panel_resize = false;
                    dock_interaction_state().active_dock_panel_close = false;
                    dock_interaction_state().active_dock_panel_title_drag = false;
                    dock_interaction_state().active_dock_panel_was_floating = false;
                    dock_interaction_state().active_dock_panel_undocked = false;
                    dock_interaction_state().active_dock_panel_resize_neighbor_id = 0;
                    dock_interaction_state().active_dock_panel_start_neighbor_height = 0.0f;
                    dock_interaction_state().active_dock_split_space_id = 0;
                    dock_interaction_state().active_dock_split_node = U32_MAX;
                    m_key_modifiers = KeyModifierFlag::none;
                    for(bool& down : m_key_down)
                    {
                        down = false;
                    }
                    for(bool& down : m_pointer_button_down)
                    {
                        down = false;
                    }
                    for(usize i = 0; i < m_popup_stack.open_stack.size(); ++i)
                    {
                        if(test_flags(m_popup_stack.open_stack[i].flags, PopupFlag::close_on_blur))
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
                id_t scrollbar_id = 0;
                bool scrollbar_vertical = false;
                RectF scrollbar_thumb;
                id_t dock_space_id = 0;
                id_t dock_panel_id = 0;
                bool dock_resize = false;
                bool dock_close = false;
                u32 dock_split_node = U32_MAX;
                DockSplitAxis dock_split_axis = DockSplitAxis::x;
                u32 dock_leaf_index = U32_MAX;
                id_t tab_bar_id = 0;
                id_t tab_item_id = 0;
                bool tab_close = false;
                id_t tab_scroll_bar_id = 0;
                bool tab_scroll_left = false;
                if(hit_test_tab_scroll_button(m_pointer_pos, tab_scroll_bar_id, tab_scroll_left))
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
            update_menu_hover();
            if(m_hovered_id != tooltip_interaction_state().tooltip_hovered_id)
            {
                tooltip_interaction_state().tooltip_hovered_id = m_hovered_id;
                tooltip_interaction_state().tooltip_hover_start = m_time;
            }
#ifdef LUNA_GUI_ENABLE_DEBUG
            for(DebugInputEventInfo& event_info : m_debug_input_events)
            {
                event_info.hovered_after = m_hovered_id;
                event_info.active_after = m_active_id;
                event_info.focused_after = m_focused_id;
                if(event_info.event.type == InputEventType::pointer_enter ||
                    event_info.event.type == InputEventType::pointer_move ||
                    event_info.event.type == InputEventType::pointer_down ||
                    event_info.event.type == InputEventType::pointer_up ||
                    event_info.event.type == InputEventType::pointer_wheel)
                {
                    event_info.hit_node = hit_test(event_info.event.position);
                    event_info.hit_layer = hit_test_layer_index(event_info.event.position);
                    event_info.stage = DebugHitTestStage::generic;
                }
            }
#endif
        }

        RV Context::submit(const Description& desc)
        {
            lutsassert();
            lutry
            {
                m_submitted_desc = desc;
                m_layouts.clear();
                m_layouts.resize(m_submitted_desc.nodes.size());
                m_popup_stack.submitted_infos.clear();
                for(auto& info : m_popup_stack.build_infos)
                {
                    m_popup_stack.submitted_infos.insert_or_assign(info.first, info.second);
                }
                rebuild_popup_node_indices();
                prune_popup_stack();
                HashSet<id_t> ids;
                bool tooltip_submitted = false;
                for(const Node& node : m_submitted_desc.nodes)
                {
                    if(tooltip_layer(node))
                    {
                        tooltip_submitted = true;
                    }
                    if(!node.interactive) continue;
                    auto r = ids.insert(node.id);
                    luassert_msg(r.second, "Duplicate GUI item ID detected.");
                    Ref<ItemQueryState> result = get_or_create_query_state(node.id);
                    result->states.insert_or_assign(Name("gui.clicked"), Any(false));
                    result->states.insert_or_assign(Name("gui.right_clicked"), Any(false));
                    result->states.insert_or_assign(Name("gui.double_clicked"), Any(false));
                    result->states.insert_or_assign(Name("gui.hovered"), Any(false));
                    result->states.insert_or_assign(Name("gui.active"), Any(false));
                    result->states.insert_or_assign(Name("gui.focused"), Any(false));
                    result->states.insert_or_assign(Name("gui.enabled"), Any(node.enabled_state()));
                    result->states.insert_or_assign(Name("gui.value_changed"), Any(false));
                    if(popup_layer(node))
                    {
                        Ref<DisclosureState> disclosure = get_or_create_widget_state<DisclosureState>(node.id);
                        disclosure->open = popup_node_visible(node);
                        result->states.insert_or_assign(Name("gui.open"), Any(disclosure->open));
                    }
                    else if(tab_item_layout(node))
                    {
                        bool open = bool_value_open(node);
                        result->states.insert_or_assign(Name("gui.open"), Any(open));
                    }
                    else if(input_text_node(node) && input_text_value(node))
                    {
                        Ref<InputEditState> input_state = get_or_create_widget_state<InputEditState>(node.id);
                        input_state->text_cursor = clamp_utf8_cursor(*input_text_value(node), input_state->text_cursor);
                    }
                    else if(numeric_text_editable(node))
                    {
                        InputEditState* input_state = get_widget_state<InputEditState>(node.id);
                        if(input_state && input_state->numeric_editing)
                        {
                            input_state->text_cursor = clamp_utf8_cursor(input_state->numeric_edit_text, input_state->text_cursor);
                        }
                    }
                }
                if(m_drag_drop.active)
                {
                    bool source_live = false;
                    for(const Node& node : m_submitted_desc.nodes)
                    {
                        if(node.id == m_drag_drop.source_id && contains_name(node.drag_drop_source_types, m_drag_drop.type))
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
                m_layout_dirty = false;
                layout_layers();
                process_input_events();
                if(tooltip_submitted)
                {
                    m_layout_dirty = true;
                }
                if(m_layout_dirty)
                {
                    for(NodeLayout& layout : m_layouts)
                    {
                        layout.metrics_valid = false;
                    }
                    layout_layers();
                    if(m_pointer_inside)
                    {
                        id_t scrollbar_id = 0;
                        bool scrollbar_vertical = false;
                        RectF scrollbar_thumb;
                        id_t dock_space_id = 0;
                        id_t dock_panel_id = 0;
                        bool dock_resize = false;
                        bool dock_close = false;
                        u32 dock_split_node = U32_MAX;
                        DockSplitAxis dock_split_axis = DockSplitAxis::x;
                        u32 dock_leaf_index = U32_MAX;
                        id_t tab_bar_id = 0;
                        id_t tab_item_id = 0;
                        bool tab_close = false;
                        id_t tab_scroll_bar_id = 0;
                        bool tab_scroll_left = false;
                        if(hit_test_tab_scroll_button(m_pointer_pos, tab_scroll_bar_id, tab_scroll_left))
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
                if(m_hovered_id != tooltip_interaction_state().tooltip_hovered_id)
                {
                    tooltip_interaction_state().tooltip_hovered_id = m_hovered_id;
                    tooltip_interaction_state().tooltip_hover_start = m_time;
                }
                for(const Node& node : m_submitted_desc.nodes)
                {
                    if(!node.interactive) continue;
                    Ref<ItemQueryState> result = get_or_create_query_state(node.id);
                    ContextNodeInputContext node_input_context;
                    node_input_context.context = this;
                    node_input_context.result = result.get();
                    node_input_context.node_id = node.id;
                    node_input_context.current_pointer_position = m_pointer_pos;
                    node.update_state(node_input_context);

                    InteractionState* interaction = get_widget_state<InteractionState>(node.id);
                    if(interaction)
                    {
                        touch_widget_state<InteractionState>(node.id);
                    }
                    bool enabled = node.enabled_state();
                    result->states.insert_or_assign(Name("gui.enabled"), Any(enabled));
                    result->states.insert_or_assign(Name("gui.hovered"), Any(enabled && node.id == m_hovered_id));
                    result->states.insert_or_assign(Name("gui.active"), Any(enabled && (node.id == m_active_id || (interaction && interaction->active))));
                    result->states.insert_or_assign(Name("gui.focused"), Any(enabled && node.id == m_focused_id));
                    if(popup_layer(node))
                    {
                        Ref<DisclosureState> disclosure = get_or_create_widget_state<DisclosureState>(node.id);
                        disclosure->open = popup_node_visible(node);
                        result->states.insert_or_assign(Name("gui.open"), Any(disclosure->open));
                    }
                    else if(tab_item_layout(node))
                    {
                        bool open = bool_value_open(node);
                        result->states.insert_or_assign(Name("gui.open"), Any(open));
                    }
                }
                m_submitted = true;
            }
            lucatchret;
            return ok;
        }
    }
}
