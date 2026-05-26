/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUIContext.cpp
* @author JXMaster
* @date 2026/5/21
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "GUI.hpp"
#include <Luna/VG/VG.hpp>

namespace Luna
{
    namespace GUI
    {
        GUIContext::GUIContext()
        {
            m_shape_draw_list = VG::new_shape_draw_list(m_device);
            m_main_draw_list = new_draw_list();
            m_overlay_draw_list = new_draw_list();
            m_shape_renderer = VG::new_fill_shape_renderer();
            m_font_atlas = VG::new_font_atlas();
        }

        void GUIContext::begin_frame(const GUIFrameDesc& desc)
        {
            lutsassert();
            m_time += desc.delta_time;
            m_frame_desc = desc;
            m_submitted = false;
            m_last_results = m_current_results;
            m_current_results.clear();
            ++m_generation;
            m_build_desc = GUIDescription();
            m_build_desc.generation = m_generation;
            m_parent_stack.clear();
            m_id_stack.clear();
            m_clip_stack.clear();
            m_child_ordinals.clear();
            m_has_next_dock_panel_style = false;
            m_next_dock_panel_open = nullptr;

            GUINode root;
            root.id = 1;
            root.kind = GUINodeKind::root;
            root.render_layer = GUIRenderLayer::main;
            root.parent = U32_MAX;
            root.depth = 0;
            apply_requested_size(root, GUISize::fixed(desc.surface_size.x, desc.surface_size.y));
            m_build_desc.nodes.push_back(root);
            m_child_ordinals.push_back(0);
            m_parent_stack.push_back(0);
            m_id_stack.push_back(root.id);
            set_current_context(this);
        }

        void GUIContext::add_input_event(const GUIInputEvent& event)
        {
            lutsassert();
            m_input_events.push_back(event);
        }

        void GUIContext::add_input_events(Span<const GUIInputEvent> events)
        {
            lutsassert();
            m_input_events.insert(m_input_events.end(), events.begin(), events.end());
        }

        void GUIContext::set_clipboard_io(const GUIClipboardIO& io)
        {
            lutsassert();
            m_clipboard_io = io;
        }

        GUITextInputState GUIContext::get_text_input_state()
        {
            lutsassert();
            GUITextInputState ret;
            if(!m_focused_id || m_submitted_desc.nodes.empty() || m_layouts.size() != m_submitted_desc.nodes.size())
            {
                return ret;
            }
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                const GUINode& node = m_submitted_desc.nodes[i];
                if(node.id != m_focused_id || node.kind != GUINodeKind::input_text || !node.string_value)
                {
                    continue;
                }
                const RectF& rect = m_layouts[i].rect;
                PersistentItemState& state = get_or_create_persistent_state(node.id);
                state.text_cursor = clamp_utf8_cursor(*node.string_value, state.text_cursor);
                f32 font_size = 16.0f;
                RectF text_rect(rect.offset_x + 8.0f, rect.offset_y, max(rect.width - 16.0f, 1.0f), rect.height);
                ret.active = true;
                ret.rect = text_rect;
                ret.cursor = (i32)(input_text_cursor_x(*node.string_value, state.text_cursor, font_size) + 0.5f);
                return ret;
            }
            return ret;
        }

        R<GUIDescription> GUIContext::end_build()
        {
            lutsassert();
            return m_build_desc;
        }

        GUIItemHandle GUIContext::add_node(GUINodeKind kind, const c8* text, bool interactive)
        {
            lutsassert();
            luassert(!m_parent_stack.empty());
            u32 parent = m_parent_stack.back();
            u32 ordinal = m_child_ordinals[parent]++;
            u64 h = hash_u64(m_build_desc.nodes[parent].id);
            h = hash_u64(m_id_stack.empty() ? 0 : m_id_stack.back(), h);
            h = hash_u64((u64)kind, h);
            h = hash_u64((u64)ordinal, h);
            h = hash_cstr(text, h);

            GUINode node;
            node.id = h ? h : 1;
            node.kind = kind;
            node.render_layer = m_build_desc.nodes[parent].render_layer;
            node.parent = parent;
            node.depth = m_build_desc.nodes[parent].depth + 1;
            node.text = text ? text : "";
            node.interactive = interactive;
            if(m_build_desc.nodes[parent].kind == GUINodeKind::dock_space)
            {
                node.interactive = true;
            }
            node.layout_style = default_layout_style(kind);
            if(!m_clip_stack.empty())
            {
                node.has_user_clip_rect = true;
                node.user_clip_rect = m_clip_stack.back();
            }
            if(m_has_next_item_layout)
            {
                node.layout_style = m_next_item_layout;
                m_has_next_item_layout = false;
            }
            if(m_has_next_table_cell_color)
            {
                node.has_table_cell_color = true;
                node.table_cell_color = m_next_table_cell_color;
                m_has_next_table_cell_color = false;
            }
            if(m_has_next_dock_panel_style && m_build_desc.nodes[parent].kind == GUINodeKind::dock_space)
            {
                node.has_dock_panel_style = true;
                node.dock_panel_style = m_next_dock_panel_style;
                node.dock_panel_open = m_next_dock_panel_open;
                m_has_next_dock_panel_style = false;
                m_next_dock_panel_open = nullptr;
            }
            else if(m_has_next_dock_panel_style)
            {
                m_has_next_dock_panel_style = false;
                m_next_dock_panel_open = nullptr;
            }

            u32 index = (u32)m_build_desc.nodes.size();
            m_build_desc.nodes.push_back(node);
            m_child_ordinals.push_back(0);

            GUINode& parent_node = m_build_desc.nodes[parent];
            if(parent_node.first_child == U32_MAX)
            {
                parent_node.first_child = index;
            }
            else
            {
                m_build_desc.nodes[parent_node.last_child].next_sibling = index;
            }
            parent_node.last_child = index;

            return GUIItemHandle{get_object(), node.id, m_generation};
        }

        void GUIContext::begin_container(GUINodeKind kind, const c8* label, const GUISize& size, GUIItemHandle* out_handle)
        {
            bool interactive = kind == GUINodeKind::scroll_view || kind == GUINodeKind::table_layout;
            GUIItemHandle handle = add_node(kind, label, interactive);
            u32 index = (u32)m_build_desc.nodes.size() - 1;
            apply_requested_size(m_build_desc.nodes[index], size);
            if(kind == GUINodeKind::window || kind == GUINodeKind::scroll_view)
            {
                m_build_desc.nodes[index].layout_desc.padding = GUIEdgeInsets::all(8.0f);
            }
            if(kind == GUINodeKind::dock_space)
            {
                m_build_desc.nodes[index].layout_desc.padding = GUIEdgeInsets::all(0.0f);
                m_build_desc.nodes[index].layout_desc.gap = 0.0f;
            }
            m_parent_stack.push_back(index);
            m_id_stack.push_back(handle.id);
            if(out_handle) *out_handle = handle;
        }

        void GUIContext::end_container()
        {
            lutsassert();
            luassert(m_parent_stack.size() > 1);
            m_parent_stack.pop_back();
            m_id_stack.pop_back();
        }

        void GUIContext::push_id(GUIID id)
        {
            lutsassert();
            u64 h = hash_u64(m_id_stack.empty() ? 0 : m_id_stack.back());
            h = hash_u64(id, h);
            m_id_stack.push_back(h);
        }

        void GUIContext::pop_id()
        {
            lutsassert();
            luassert(m_id_stack.size() > 1);
            m_id_stack.pop_back();
        }

        void GUIContext::push_clip_rect(const RectF& rect)
        {
            lutsassert();
            RectF clipped = rect;
            if(!m_clip_stack.empty())
            {
                clipped = intersect_rect(m_clip_stack.back(), rect);
            }
            m_clip_stack.push_back(clipped);
        }

        void GUIContext::pop_clip_rect()
        {
            lutsassert();
            luassert(!m_clip_stack.empty());
            m_clip_stack.pop_back();
        }

        void GUIContext::set_next_dock_panel_style(const GUIDockPanelStyle& style, bool* open)
        {
            lutsassert();
            m_has_next_dock_panel_style = true;
            m_next_dock_panel_style = style;
            m_next_dock_panel_open = open;
        }

        ItemResult* GUIContext::get_query_result(GUIItemHandle handle)
        {
            if(handle.context != get_object()) return nullptr;
            if(m_submitted)
            {
                if(handle.generation != m_generation) return nullptr;
                auto iter = m_current_results.find(handle.id);
                return iter == m_current_results.end() ? nullptr : &iter->second;
            }
            auto iter = m_last_results.find(handle.id);
            return iter == m_last_results.end() ? nullptr : &iter->second;
        }

        ItemResult& GUIContext::get_or_create_current_result(GUIID id)
        {
            auto iter = m_current_results.find(id);
            if(iter == m_current_results.end())
            {
                ItemResult result;
                result.generation = m_generation;
                iter = m_current_results.insert(make_pair(id, move(result))).first;
            }
            return iter->second;
        }

        PersistentItemState& GUIContext::get_or_create_persistent_state(GUIID id)
        {
            auto iter = m_persistent_states.find(id);
            if(iter == m_persistent_states.end())
            {
                PersistentItemState state;
                iter = m_persistent_states.insert(make_pair(id, state)).first;
            }
            return iter->second;
        }

        DockPanelPersistentState& GUIContext::get_or_create_dock_panel_state(PersistentItemState& dock_state, GUIID panel_id)
        {
            auto iter = dock_state.dock_panels.find(panel_id);
            if(iter == dock_state.dock_panels.end())
            {
                DockPanelPersistentState state;
                iter = dock_state.dock_panels.insert(make_pair(panel_id, state)).first;
            }
            return iter->second;
        }

        const Any* GUIContext::get_state(GUIItemHandle handle, const Name& key)
        {
            lutsassert();
            ItemResult* result = get_query_result(handle);
            if(!result) return nullptr;
            auto iter = result->states.find(key);
            return iter == result->states.end() ? nullptr : &iter->second;
        }

        void GUIContext::set_state(GUIItemHandle handle, const Name& key, const Any& value)
        {
            lutsassert();
            if(handle.context != get_object()) return;
            ItemResult& result = get_or_create_current_result(handle.id);
            result.states.insert_or_assign(key, value);
        }

        void GUIContext::remove_state(GUIItemHandle handle, const Name& key)
        {
            lutsassert();
            if(handle.context != get_object()) return;
            auto iter = m_current_results.find(handle.id);
            if(iter == m_current_results.end()) return;
            iter->second.states.erase(key);
        }

        void GUIContext::set_next_item_layout(const GUILayoutStyle& style)
        {
            lutsassert();
            m_next_item_layout = style;
            m_has_next_item_layout = true;
        }

        void GUIContext::set_next_table_cell_color(const Float4U& color)
        {
            lutsassert();
            m_next_table_cell_color = color;
            m_has_next_table_cell_color = true;
        }
    }
}
