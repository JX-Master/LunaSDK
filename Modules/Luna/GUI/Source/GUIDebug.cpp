/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUIDebug.cpp
* @author JXMaster
* @date 2026/6/3
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "GUI.hpp"

#ifdef LUNA_GUI_ENABLE_DEBUG

namespace Luna
{
    namespace GUI
    {
        namespace
        {
            constexpr id_t DEBUG_LAYER_ID_VALUE = 0xD36D0E6B6F71A11Full;

            bool same_debug_node(const DebugNodeKey& lhs, const DebugNodeKey& rhs)
            {
                return lhs.generation == rhs.generation &&
                    lhs.layer_id == rhs.layer_id &&
                    lhs.node_index == rhs.node_index &&
                    lhs.node_id == rhs.node_id;
            }

            String format_guid(const Guid& guid)
            {
                String ret;
                strprintf(ret, "%016llX%016llX", (unsigned long long)guid.high, (unsigned long long)guid.low);
                return ret;
            }

            String type_name(typeinfo_t type)
            {
                if(!type) return String();
                Name name = get_type_name(type);
                return String(name.c_str());
            }

            String guid_type_name(const Guid& guid)
            {
                typeinfo_t type = get_type_by_guid(guid);
                String name = type_name(type);
                if(!name.empty()) return name;
                return format_guid(guid);
            }

            DebugValue debug_value_from_type(typeinfo_t type)
            {
                DebugValue value;
                if(type)
                {
                    value.type_name = type_name(type);
                    value.type_guid = get_type_guid(type);
                }
                return value;
            }

            DebugValue debug_value_from_object(object_t obj)
            {
                DebugValue value;
                if(!obj)
                {
                    value.value = "<null>";
                    return value;
                }
                value = debug_value_from_type(get_object_type(obj));
                value.value = "<boxed object>";
                value.opaque = true;
                return value;
            }

            DebugValue debug_value_from_any(const Any& any)
            {
                DebugValue value = debug_value_from_type(any.type());
                if(!any.has_value())
                {
                    value.value = "<empty>";
                    return value;
                }
                if(const bool* typed = any.as<bool>())
                {
                    value.value = *typed ? "true" : "false";
                }
                else if(const i32* typed = any.as<i32>())
                {
                    strprintf(value.value, "%d", *typed);
                }
                else if(const u32* typed = any.as<u32>())
                {
                    strprintf(value.value, "%u", *typed);
                }
                else if(const i64* typed = any.as<i64>())
                {
                    strprintf(value.value, "%lld", (long long)*typed);
                }
                else if(const u64* typed = any.as<u64>())
                {
                    strprintf(value.value, "%llu", (unsigned long long)*typed);
                }
                else if(const f32* typed = any.as<f32>())
                {
                    strprintf(value.value, "%.3f", *typed);
                }
                else if(const f64* typed = any.as<f64>())
                {
                    strprintf(value.value, "%.3f", *typed);
                }
                else if(const Float2U* typed = any.as<Float2U>())
                {
                    strprintf(value.value, "(%.3f, %.3f)", typed->x, typed->y);
                }
                else if(const Float3U* typed = any.as<Float3U>())
                {
                    strprintf(value.value, "(%.3f, %.3f, %.3f)", typed->x, typed->y, typed->z);
                }
                else if(const Float4U* typed = any.as<Float4U>())
                {
                    strprintf(value.value, "(%.3f, %.3f, %.3f, %.3f)", typed->x, typed->y, typed->z, typed->w);
                }
                else if(const RectF* typed = any.as<RectF>())
                {
                    strprintf(value.value, "(%.1f, %.1f, %.1f, %.1f)",
                        typed->offset_x, typed->offset_y, typed->width, typed->height);
                }
                else if(const String* typed = any.as<String>())
                {
                    value.value = *typed;
                }
                else
                {
                    value.value = "<opaque>";
                    value.opaque = true;
                }
                return value;
            }

            bool contains_name_local(const Vector<Name>& names, const Name& name)
            {
                for(const Name& item : names)
                {
                    if(item == name) return true;
                }
                return false;
            }

            void collect_style_entry_names(const Context& ctx, const Name& style_name, Vector<Name>& out_names)
            {
                if(!style_name) return;
                Name cursor = style_name;
                for(usize depth = 0; depth < ctx.m_styles.size(); ++depth)
                {
                    auto style_iter = ctx.m_styles.find(cursor);
                    if(style_iter == ctx.m_styles.end()) return;
                    for(const auto& entry : style_iter->second.entries)
                    {
                        if(!contains_name_local(out_names, entry.first))
                        {
                            out_names.push_back(entry.first);
                        }
                    }
                    if(!style_iter->second.parent) return;
                    cursor = style_iter->second.parent;
                }
            }

            DebugResolvedStyleEntryInfo resolve_style_entry(const Context& ctx, const Name& style_name, const Name& entry)
            {
                DebugResolvedStyleEntryInfo info;
                info.name = entry;
                if(!style_name || !entry) return info;
                Name cursor = style_name;
                for(usize depth = 0; depth < ctx.m_styles.size(); ++depth)
                {
                    auto style_iter = ctx.m_styles.find(cursor);
                    if(style_iter == ctx.m_styles.end()) return info;
                    auto entry_iter = style_iter->second.entries.find(entry);
                    if(entry_iter != style_iter->second.entries.end())
                    {
                        if(entry_iter->second.state == StyleEntryState::set)
                        {
                            info.found = true;
                            info.value = entry_iter->second.value;
                        }
                        else if(entry_iter->second.state == StyleEntryState::unset)
                        {
                            info.unset = true;
                        }
                        return info;
                    }
                    if(!style_iter->second.parent) return info;
                    cursor = style_iter->second.parent;
                }
                return info;
            }

            RectF to_layer_rect(const RectF& rect, const Float2U& layer_position)
            {
                return RectF(rect.offset_x - layer_position.x, rect.offset_y - layer_position.y, rect.width, rect.height);
            }

            void copy_layout(DebugLayoutInfo& out, const NodeLayout& layout, const Float2U& layer_position)
            {
                out.screen_rect = layout.rect;
                out.screen_clip_rect = layout.clip_rect;
                out.layer_rect = to_layer_rect(layout.rect, layer_position);
                out.layer_clip_rect = to_layer_rect(layout.clip_rect, layer_position);
                out.metrics = layout.metrics;
                out.metrics_valid = layout.metrics_valid;
                out.table_column_offsets = layout.table_column_offsets;
                out.table_column_widths = layout.table_column_widths;
                out.table_row_offsets = layout.table_row_offsets;
                out.table_row_heights = layout.table_row_heights;
                out.table_columns = layout.table_columns;
                out.table_rows = layout.table_rows;
                out.tab_header_rect = layout.tab_header_rect;
                out.tab_header_clip_rect = layout.tab_header_clip_rect;
                out.tab_close_rect = layout.tab_close_rect;
                out.tab_scroll_left_rect = layout.tab_scroll_left_rect;
                out.tab_scroll_right_rect = layout.tab_scroll_right_rect;
                out.tab_scrollable = layout.tab_scrollable;
                out.tab_scroll_max = layout.tab_scroll_max;
                out.tab_content_visible = layout.tab_content_visible;
                out.scroll_content_size = layout.scroll_content_size;
                out.scroll_viewport_size = layout.scroll_viewport_size;
                out.scroll_has_vertical = layout.scroll_has_vertical;
                out.scroll_has_horizontal = layout.scroll_has_horizontal;
                out.dock_panel_child = layout.dock_panel_child;
                out.dock_panel_visible = layout.dock_panel_visible;
                out.dock_panel_floating = layout.dock_panel_floating;
                out.dock_space_id = layout.dock_space_id;
                out.dock_panel_rect = layout.dock_panel_rect;
                out.dock_panel_clip_rect = layout.dock_panel_clip_rect;
                out.dock_panel_title_rect = layout.dock_panel_title_rect;
                out.dock_panel_close_rect = layout.dock_panel_close_rect;
                out.dock_panel_resize_rect = layout.dock_panel_resize_rect;
                out.dock_panel_style = layout.dock_panel_style;
                out.dock_panel_z_order = layout.dock_panel_z_order;
                out.dock_leaf_index = layout.dock_leaf_index;
            }

            DebugNodeKey make_node_key(const Context& ctx, u32 node_index)
            {
                DebugNodeKey key;
                key.generation = ctx.m_generation;
                key.node_index = node_index;
                if(node_index < ctx.m_submitted_desc.nodes.size())
                {
                    const Node& node = ctx.m_submitted_desc.nodes[node_index];
                    key.node_id = node.id;
                    if(node.layer < ctx.m_submitted_desc.layers.size())
                    {
                        key.layer_id = ctx.m_submitted_desc.layers[node.layer].id;
                    }
                }
                return key;
            }

        }

        LUNA_GUI_API id_t debug_layer_id()
        {
            return DEBUG_LAYER_ID_VALUE;
        }

        R<DebugInfo> Context::dump_debug_info()
        {
            lutsassert();
            DebugInfo info;
            info.context.frame_desc = m_frame_desc;
            info.context.generation = m_generation;
            info.context.time = m_time;
            info.context.submitted = m_submitted;
            info.context.pointer_inside = m_pointer_inside;
            info.context.pointer_position = m_pointer_pos;
            info.context.pointer_delta = m_pointer_delta;
            for(usize i = 0; i < 5; ++i)
            {
                info.context.pointer_button_down[i] = m_pointer_button_down[i];
            }
            for(usize i = 0; i < 256; ++i)
            {
                info.context.key_down[i] = m_key_down[i];
            }
            info.context.key_modifiers = m_key_modifiers;
            info.context.hovered_id = m_hovered_id;
            info.context.active_id = m_active_id;
            info.context.focused_id = m_focused_id;
            info.context.drag_drop_active = m_drag_drop.active;
            info.context.drag_drop_source_id = m_drag_drop.source_id;
            info.context.drag_drop_type = m_drag_drop.type;
            for(const PopupStackEntry& entry : m_popup_stack.open_stack)
            {
                info.context.popup_stack.push_back(entry.id);
            }
            info.input_events = m_debug_input_events;

            info.layers.reserve(m_submitted_desc.layers.size());
            for(u32 i = 0; i < (u32)m_submitted_desc.layers.size(); ++i)
            {
                const Layer& layer = m_submitted_desc.layers[i];
                DebugLayerInfo layer_info;
                layer_info.index = i;
                layer_info.id = layer.id;
                layer_info.root = layer.root;
                layer_info.screen_position = layer.screen_position;
                layer_info.debug_layer = layer.id == DEBUG_LAYER_ID_VALUE;
                if(layer.root != U32_MAX && layer.root < m_layouts.size())
                {
                    layer_info.root_screen_rect = m_layouts[layer.root].rect;
                }
                info.layers.push_back(move(layer_info));
            }

            info.styles.reserve(m_styles.size());
            for(const auto& style_entry : m_styles)
            {
                DebugStyleInfo style_info;
                style_info.name = style_entry.second.name;
                style_info.parent = style_entry.second.parent;
                for(const auto& entry : style_entry.second.entries)
                {
                    DebugStyleEntryInfo entry_info;
                    entry_info.name = entry.first;
                    entry_info.state = entry.second.state;
                    entry_info.value = entry.second.value;
                    style_info.entries.push_back(move(entry_info));
                }
                info.styles.push_back(move(style_info));
            }

            info.nodes.reserve(m_submitted_desc.nodes.size());
            for(u32 i = 0; i < (u32)m_submitted_desc.nodes.size(); ++i)
            {
                const Node& node = m_submitted_desc.nodes[i];
                DebugNodeInfo node_info;
                node_info.key = make_node_key(*this, i);
                node_info.type_guid = node.type_guid();
                node_info.type_name = guid_type_name(node_info.type_guid);
                node_info.text = node.text;
                node_info.parent = node.parent;
                node_info.first_child = node.first_child;
                node_info.last_child = node.last_child;
                node_info.next_sibling = node.next_sibling;
                node_info.depth = node.depth;
                node_info.interactive = node.interactive;
                node_info.default_interactive = node.default_interactive();
                node_info.enabled = node.enabled_state();
                node_info.debug_layer_node = node_info.key.layer_id == DEBUG_LAYER_ID_VALUE;
                node_info.absolute_position = node.absolute_position;
                node_info.position = node.position;
                node_info.has_user_clip_rect = node.has_user_clip_rect;
                node_info.user_clip_rect = node.user_clip_rect;
                node_info.style = node.style;
                node_info.layout_style = node.layout_style;
                node_info.layout_desc = node.layout_desc;
                node_info.requested_size = node.requested_size;
                node_info.render_proxy_has_draw = node.render_proxy.draw != nullptr;
                node_info.render_proxy_has_draw_after_children = node.render_proxy.draw_after_children != nullptr;
                node_info.render_proxy_draw_ptr = (usize)(void*)node.render_proxy.draw;
                node_info.render_proxy_draw_after_children_ptr = (usize)(void*)node.render_proxy.draw_after_children;
                node_info.render_proxy_userdata_ptr = (usize)node.render_proxy.userdata;
                node_info.drag_drop_source_types = node.drag_drop_source_types;
                node_info.drag_drop_target_types = node.drag_drop_target_types;
                if(i < m_layouts.size() && node.layer < m_submitted_desc.layers.size())
                {
                    copy_layout(node_info.layout, m_layouts[i], m_submitted_desc.layers[node.layer].screen_position);
                    node_info.visible = m_layouts[i].rect.width > 0.0f && m_layouts[i].rect.height > 0.0f &&
                        m_layouts[i].clip_rect.width > 0.0f && m_layouts[i].clip_rect.height > 0.0f &&
                        (!m_layouts[i].dock_panel_child || m_layouts[i].dock_panel_visible);
                    node_info.hit_testable = node_info.visible && node.interactive && node.enabled_state() && !node_info.debug_layer_node;
                }

                Vector<Name> style_entry_names;
                collect_style_entry_names(*this, node.style, style_entry_names);
                for(const Name& entry : style_entry_names)
                {
                    node_info.resolved_style.push_back(resolve_style_entry(*this, node.style, entry));
                }
                if(node.render_proxy.style_entries && node.render_proxy.num_style_entries)
                {
                    node_info.style_usage.reserve(node.render_proxy.num_style_entries);
                    for(usize usage_index = 0; usage_index < node.render_proxy.num_style_entries; ++usage_index)
                    {
                        const StyleEntryDesc& desc = node.render_proxy.style_entries[usage_index];
                        if(!desc.name) continue;
                        DebugResolvedStyleEntryInfo resolved = resolve_style_entry(*this, node.style, desc.name);
                        DebugStyleUsageInfo usage;
                        usage.name = desc.name;
                        usage.type = desc.type;
                        usage.default_value = desc.default_value;
                        usage.display_name = desc.display_name ? desc.display_name : "";
                        usage.category = desc.category ? desc.category : "";
                        usage.description = desc.description ? desc.description : "";
                        usage.found = resolved.found;
                        usage.unset = resolved.unset;
                        usage.uses_default = !resolved.found || resolved.unset;
                        usage.value = usage.uses_default ? desc.default_value : resolved.value;
                        node_info.style_usage.push_back(move(usage));
                    }
                }

                object_t state_obj = get_state_object(make_state_id<ItemQueryState>(node.id));
                if(ItemQueryState* query_state = state_obj ? cast_object<ItemQueryState>(state_obj) : nullptr)
                {
                    for(const auto& state_entry : query_state->states)
                    {
                        node_info.item_query_states.push_back(make_pair(state_entry.first, debug_value_from_any(state_entry.second)));
                    }
                }

                if(node.id == m_hovered_id && !node_info.debug_layer_node)
                {
                    info.context.has_main_hovered_node = true;
                    info.context.main_hovered_node = node_info.key;
                }
                info.nodes.push_back(move(node_info));
            }

            info.states.reserve(m_states.size());
            for(const auto& state_entry : m_states)
            {
                DebugStateInfo state_info;
                state_info.id = state_entry.first;
                state_info.lifetime = state_entry.second.lifetime;
                state_info.last_set_generation = state_entry.second.last_set_generation;
                state_info.data = debug_value_from_object(state_entry.second.data.get());
                info.states.push_back(move(state_info));
            }

            if(m_layouts.size() != m_submitted_desc.nodes.size())
            {
                info.warnings.push_back("Layout count does not match node count.");
            }
            for(const auto& style_entry : m_styles)
            {
                const Style& style = style_entry.second;
                if(style.parent && m_styles.find(style.parent) == m_styles.end())
                {
                    String warning;
                    strprintf(warning, "Style '%s' has a missing parent '%s'.", style.name.c_str(), style.parent.c_str());
                    info.warnings.push_back(move(warning));
                }
            }
            return info;
        }
    }
}

#endif
