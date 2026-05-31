/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Context.cpp
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
        Context::Context()
        {
            m_shape_draw_list = VG::new_shape_draw_list(m_device);
            m_feedback_draw_list = new_draw_list();
            m_shape_renderer = VG::new_fill_shape_renderer();
            m_font_atlas = VG::new_font_atlas();
        }

        void Context::begin_frame(const FrameDesc& desc)
        {
            lutsassert();
            m_time += desc.delta_time;
            m_frame_desc = desc;
            m_submitted = false;
            m_last_results = m_current_results;
            m_current_results.clear();
            m_last_drag_drop_deliveries = m_current_drag_drop_deliveries;
            m_current_drag_drop_deliveries.clear();
            ++m_generation;
            m_build_desc = Description();
            m_build_desc.generation = m_generation;
            m_parent_stack.clear();
            m_layer_stack.clear();
            m_id_stack.clear();
            m_clip_stack.clear();
            m_child_ordinals.clear();
            m_has_next_dock_panel_style = false;
            m_next_dock_panel_open = nullptr;
            m_tab_build_stack.clear();
            m_popup_build_stack.clear();
            m_last_item_id = 0;
            m_tree_depth = 0;
            m_drag_drop_preview_built = false;
            m_drag_drop_target_stack.clear();
            if(m_drag_drop_active)
            {
                m_drag_drop_payload_set = false;
                m_drag_drop_payload_data.clear();
            }

            Layer default_layer;
            default_layer.id = 1;
            default_layer.screen_position = Float2U(0.0f);
            m_build_desc.layers.push_back(default_layer);
            m_layer_stack.push_back(0);

            RootNode root;
            root.id = 1;
            root.layer = 0;
            root.parent = U32_MAX;
            root.depth = 0;
            apply_requested_size(root, Size::fixed(desc.surface_size.x, desc.surface_size.y));
            m_build_desc.nodes.push_back(root);
            m_build_desc.layers[0].root = 0;
            m_child_ordinals.push_back(0);
            m_parent_stack.push_back(0);
            m_id_stack.push_back(root.id);
        }

        void Context::add_input_event(const InputEvent& event)
        {
            lutsassert();
            m_input_events.push_back(event);
        }

        void Context::add_input_events(Span<const InputEvent> events)
        {
            lutsassert();
            m_input_events.insert(m_input_events.end(), events.begin(), events.end());
        }

        u32 Context::current_layer_index() const
        {
            luassert(!m_layer_stack.empty());
            return m_layer_stack.back();
        }

        id_t Context::make_node_id(id_t parent_id, const Guid& node_type, u32 ordinal, const c8* text) const
        {
            u64 h = hash_u64(parent_id);
            h = hash_u64(m_id_stack.empty() ? 0 : m_id_stack.back(), h);
            h = hash_u64(node_type.high, h);
            h = hash_u64(node_type.low, h);
            h = hash_u64((u64)ordinal, h);
            h = hash_cstr(text, h);
            return h ? h : 1;
        }

        id_t Context::allocate_detached_layer_id(const Guid& node_type, const c8* text)
        {
            luassert(!m_parent_stack.empty());
            u32 parent = m_parent_stack.back();
            id_t parent_id = 0;
            u32 ordinal = 0;
            if(parent == U32_MAX)
            {
                u32 layer_index = current_layer_index();
                parent_id = m_build_desc.layers[layer_index].id;
                ordinal = 0;
            }
            else
            {
                parent_id = m_build_desc.nodes[parent].id;
                ordinal = m_child_ordinals[parent]++;
            }
            return make_node_id(parent_id, node_type, ordinal, text);
        }

        void Context::push_layer_internal(id_t id, const Float2U& screen_position)
        {
            luassert(id != 0);
            for(const Layer& layer : m_build_desc.layers)
            {
                luassert_msg(layer.id != id, "Duplicate GUI layer ID detected.");
            }
            Layer layer;
            layer.id = id;
            layer.screen_position = screen_position;
            u32 layer_index = (u32)m_build_desc.layers.size();
            m_build_desc.layers.push_back(layer);
            m_layer_stack.push_back(layer_index);
            m_parent_stack.push_back(U32_MAX);
            u64 h = hash_u64(m_id_stack.empty() ? 0 : m_id_stack.back());
            h = hash_u64(id, h);
            m_id_stack.push_back(h);
        }

        void Context::push_layer(id_t id, const Float2U& screen_position)
        {
            lutsassert();
            push_layer_internal(id, screen_position);
        }

        void Context::pop_layer()
        {
            lutsassert();
            luassert(m_layer_stack.size() > 1);
            u32 layer_index = m_layer_stack.back();
            luassert(layer_index < m_build_desc.layers.size());
            luassert_msg(m_build_desc.layers[layer_index].root != U32_MAX, "GUI layer must have a root widget.");
            luassert(!m_parent_stack.empty() && m_parent_stack.back() == U32_MAX);
            luassert(m_id_stack.size() > 1);
            m_parent_stack.pop_back();
            m_layer_stack.pop_back();
            m_id_stack.pop_back();
        }

        void Context::set_clipboard_io(const ClipboardIO& io)
        {
            lutsassert();
            m_clipboard_io = io;
        }

        TextInputState Context::get_text_input_state()
        {
            lutsassert();
            TextInputState ret;
            if(!m_focused_id || m_submitted_desc.nodes.empty() || m_layouts.size() != m_submitted_desc.nodes.size())
            {
                return ret;
            }
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                const Node& node = m_submitted_desc.nodes[i];
                if(node.id != m_focused_id)
                {
                    continue;
                }
                PersistentItemState& state = get_or_create_persistent_state(node.id);
                f32 font_size = 16.0f;
                String* string_value = node.string_value();
                if(node.is_input_text() && string_value)
                {
                    const RectF& rect = m_layouts[i].rect;
                    state.text_cursor = clamp_utf8_cursor(*string_value, state.text_cursor);
                    RectF text_rect(rect.offset_x + 8.0f, rect.offset_y, max(rect.width - 16.0f, 1.0f), rect.height);
                    ret.active = true;
                    ret.rect = text_rect;
                    ret.cursor = (i32)(input_text_cursor_x(*string_value, state.text_cursor, font_size) + 0.5f);
                    return ret;
                }
                if(is_numeric_input_node(node) && state.numeric_editing)
                {
                    state.text_cursor = clamp_utf8_cursor(state.numeric_edit_text, state.text_cursor);
                    RectF component = numeric_component_rect(node, m_layouts[i].rect, state.numeric_edit_component);
                    RectF text_rect(component.offset_x + 6.0f, component.offset_y, max(component.width - 12.0f, 1.0f), component.height);
                    ret.active = true;
                    ret.rect = text_rect;
                    ret.cursor = (i32)(input_text_cursor_x(state.numeric_edit_text, state.text_cursor, font_size) + 0.5f);
                    return ret;
                }
            }
            return ret;
        }

        R<Description> Context::end_build()
        {
            lutsassert();
            return m_build_desc;
        }

        ItemHandle Context::add_node(Ref<Node> node, const c8* label, bool interactive)
        {
            lutsassert();
            return add_node_internal(move(node), label, interactive, 0);
        }

        ItemHandle Context::add_node_internal(Ref<Node> node_ref, const c8* text, bool interactive, id_t forced_id)
        {
            lutsassert();
            luassert(node_ref);
            luassert(!m_parent_stack.empty());
            Node& node = *node_ref.get();
            u32 parent = m_parent_stack.back();
            u32 layer_index = current_layer_index();
            luassert(layer_index < m_build_desc.layers.size());
            bool layer_root = parent == U32_MAX;
            u32 ordinal = 0;
            id_t parent_id = m_build_desc.layers[layer_index].id;
            if(layer_root)
            {
                luassert_msg(m_build_desc.layers[layer_index].root == U32_MAX, "GUI layer can only have one root widget.");
            }
            else
            {
                luassert(parent < m_build_desc.nodes.size());
                parent_id = m_build_desc.nodes[parent].id;
                ordinal = m_child_ordinals[parent]++;
            }

            node.id = forced_id ? forced_id : (node.id ? node.id : make_node_id(parent_id, node.type_guid(), ordinal, text));
            node.layer = layer_index;
            node.parent = layer_root ? U32_MAX : parent;
            node.depth = layer_root ? 0 : m_build_desc.nodes[parent].depth + 1;
            node.text = text ? text : "";
            node.interactive = node.interactive || interactive || node.default_interactive();
            if(!layer_root && m_build_desc.nodes[parent].is_dock_space())
            {
                node.interactive = true;
            }
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
            if(m_has_next_canvas_item_layout)
            {
                node.has_canvas_item_layout = true;
                node.canvas_item_layout = m_next_canvas_item_layout;
                m_has_next_canvas_item_layout = false;
            }
            if(m_has_next_table_cell_color)
            {
                node.has_table_cell_color = true;
                node.table_cell_color = m_next_table_cell_color;
                m_has_next_table_cell_color = false;
            }
            if(!layer_root && m_has_next_dock_panel_style && m_build_desc.nodes[parent].is_dock_space())
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
            m_build_desc.nodes.push_back(move(node_ref));
            m_child_ordinals.push_back(0);

            if(layer_root)
            {
                m_build_desc.layers[layer_index].root = index;
            }
            else
            {
                Node& parent_node = m_build_desc.nodes[parent];
                if(parent_node.first_child == U32_MAX)
                {
                    parent_node.first_child = index;
                }
                else
                {
                    m_build_desc.nodes[parent_node.last_child].next_sibling = index;
                }
                parent_node.last_child = index;
            }

            m_last_item_id = m_build_desc.nodes[index].id;
            return ItemHandle{get_object(), m_build_desc.nodes[index].id, m_generation};
        }

        void Context::begin_container(Ref<Node> node, const c8* label, const Size& size, ItemHandle* out_handle, id_t forced_id)
        {
            luassert(node);
            bool interactive = node->default_interactive();
            ItemHandle handle = add_node_internal(move(node), label, interactive, forced_id);
            u32 index = (u32)m_build_desc.nodes.size() - 1;
            apply_requested_size(m_build_desc.nodes[index], size);
            m_build_desc.nodes[index].apply_container_defaults(m_build_desc.nodes[index].layout_desc);
            m_parent_stack.push_back(index);
            m_id_stack.push_back(handle.id);
            if(out_handle) *out_handle = handle;
        }

        void Context::end_container()
        {
            lutsassert();
            luassert(m_parent_stack.size() > 1);
            m_parent_stack.pop_back();
            m_id_stack.pop_back();
        }

        void Context::push_id(id_t id)
        {
            lutsassert();
            u64 h = hash_u64(m_id_stack.empty() ? 0 : m_id_stack.back());
            h = hash_u64(id, h);
            m_id_stack.push_back(h);
        }

        void Context::pop_id()
        {
            lutsassert();
            luassert(m_id_stack.size() > 1);
            m_id_stack.pop_back();
        }

        void Context::push_clip_rect(const RectF& rect)
        {
            lutsassert();
            RectF clipped = rect;
            if(!m_clip_stack.empty())
            {
                clipped = intersect_rect(m_clip_stack.back(), rect);
            }
            m_clip_stack.push_back(clipped);
        }

        void Context::pop_clip_rect()
        {
            lutsassert();
            luassert(!m_clip_stack.empty());
            m_clip_stack.pop_back();
        }

        void Context::tree_push()
        {
            lutsassert();
            luassert(m_last_item_id != 0);
            ++m_tree_depth;
            m_id_stack.push_back(m_last_item_id);
        }

        void Context::tree_push(ItemHandle node)
        {
            lutsassert();
            luassert(node.context == get_object());
            luassert(node.id != 0);
            ++m_tree_depth;
            m_id_stack.push_back(node.id);
        }

        void Context::tree_pop()
        {
            lutsassert();
            luassert(m_tree_depth > 0);
            --m_tree_depth;
            luassert(m_id_stack.size() > 1);
            m_id_stack.pop_back();
        }

        bool Context::begin_drag_drop_source(ItemHandle source, const Name& payload_type)
        {
            lutsassert();
            if(!payload_type) return false;
            Node* node = find_build_node(source);
            if(!node) return false;
            if(!contains_name(node->drag_drop_source_types, payload_type))
            {
                node->drag_drop_source_types.push_back(payload_type);
            }
            if(!m_drag_drop_active || m_drag_drop_source_id != source.id || m_drag_drop_type != payload_type)
            {
                return false;
            }

            m_drag_drop_preview_built = true;
            ItemHandle preview;
            Float2U preview_pos(
                min(m_pointer_pos.x + 14.0f, max(m_frame_desc.surface_size.x - 8.0f, 0.0f)),
                min(m_pointer_pos.y + 18.0f, max(m_frame_desc.surface_size.y - 8.0f, 0.0f)));
            u64 preview_layer_id = hash_u64(source.id);
            preview_layer_id = hash_cstr("DragDropPreview", preview_layer_id);
            push_layer_internal(preview_layer_id, preview_pos);
            begin_container(Ref<Node>(new_object<PopupNode>()), "DragDropPreview", Size(), &preview, preview_layer_id);
            Node& preview_node = m_build_desc.nodes.back();
            preview_node.layout_desc.padding = EdgeInsets::all(6.0f);
            preview_node.layout_desc.gap = 2.0f;
            return true;
        }

        void Context::set_drag_drop_payload(const void* data, usize data_size)
        {
            lutsassert();
            if(!m_drag_drop_active) return;
            m_drag_drop_payload_data.resize(data_size);
            if(data_size && data)
            {
                memcpy(m_drag_drop_payload_data.data(), data, data_size);
            }
            m_drag_drop_payload_set = true;
        }

        void Context::end_drag_drop_source()
        {
            lutsassert();
            end_container();
            pop_layer();
        }

        bool Context::begin_drag_drop_target(ItemHandle target, const Name& payload_type)
        {
            lutsassert();
            if(!payload_type) return false;
            Node* node = find_build_node(target);
            if(!node) return false;
            if(!contains_name(node->drag_drop_target_types, payload_type))
            {
                node->drag_drop_target_types.push_back(payload_type);
            }
            if(!m_drag_drop_active || m_drag_drop_type != payload_type)
            {
                return false;
            }
            m_drag_drop_target_stack.push_back({target, payload_type});
            return true;
        }

        const DragDropPayload* Context::accept_drag_drop_payload(const Name& payload_type)
        {
            lutsassert();
            if(m_drag_drop_target_stack.empty()) return nullptr;
            return accept_drag_drop_payload(m_drag_drop_target_stack.back().target, payload_type);
        }

        const DragDropPayload* Context::accept_drag_drop_payload(ItemHandle target, const Name& payload_type)
        {
            lutsassert();
            if(!payload_type || target.context != get_object()) return nullptr;
            const HashMap<id_t, DragDropPayloadStorage, IdHash>& deliveries = m_submitted ? m_current_drag_drop_deliveries : m_last_drag_drop_deliveries;
            auto iter = deliveries.find(target.id);
            if(iter == deliveries.end() || iter->second.type != payload_type) return nullptr;
            return make_drag_drop_payload_view(iter->second);
        }

        void Context::end_drag_drop_target()
        {
            lutsassert();
            if(!m_drag_drop_target_stack.empty())
            {
                m_drag_drop_target_stack.pop_back();
            }
        }

        bool Context::is_drag_drop_active() const
        {
            lutsassert();
            return m_drag_drop_active;
        }

        const DragDropPayload* Context::get_drag_drop_payload()
        {
            lutsassert();
            if(!m_drag_drop_active) return nullptr;
            m_drag_drop_payload_view.type = m_drag_drop_type;
            m_drag_drop_payload_view.data = m_drag_drop_payload_data.empty() ? nullptr : m_drag_drop_payload_data.data();
            m_drag_drop_payload_view.data_size = m_drag_drop_payload_data.size();
            m_drag_drop_payload_view.source = ItemHandle{get_object(), m_drag_drop_source_id, m_generation};
            m_drag_drop_payload_view.target = ItemHandle();
            m_drag_drop_payload_view.preview = true;
            m_drag_drop_payload_view.delivery = false;
            return &m_drag_drop_payload_view;
        }

        const DragDropPayload* Context::make_drag_drop_payload_view(const DragDropPayloadStorage& storage)
        {
            m_drag_drop_payload_view.type = storage.type;
            m_drag_drop_payload_view.data = storage.data.empty() ? nullptr : storage.data.data();
            m_drag_drop_payload_view.data_size = storage.data.size();
            m_drag_drop_payload_view.source = storage.source;
            m_drag_drop_payload_view.target = storage.target;
            m_drag_drop_payload_view.preview = storage.preview;
            m_drag_drop_payload_view.delivery = storage.delivery;
            return &m_drag_drop_payload_view;
        }

        void Context::set_next_dock_panel_style(const DockPanelStyle& style, bool* open)
        {
            lutsassert();
            m_has_next_dock_panel_style = true;
            m_next_dock_panel_style = style;
            m_next_dock_panel_open = open;
        }

        ItemResult* Context::get_query_result(ItemHandle handle)
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

        ItemResult& Context::get_or_create_current_result(id_t id)
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

        PersistentItemState& Context::get_or_create_persistent_state(id_t id)
        {
            auto iter = m_persistent_states.find(id);
            if(iter == m_persistent_states.end())
            {
                PersistentItemState state;
                iter = m_persistent_states.insert(make_pair(id, state)).first;
            }
            return iter->second;
        }

        Node* Context::find_build_node(ItemHandle handle)
        {
            if(handle.context != get_object() || handle.generation != m_generation) return nullptr;
            for(Node& node : m_build_desc.nodes)
            {
                if(node.id == handle.id) return &node;
            }
            return nullptr;
        }

        DockPanelPersistentState& Context::get_or_create_dock_panel_state(PersistentItemState& dock_state, id_t panel_id)
        {
            auto iter = dock_state.dock_panels.find(panel_id);
            if(iter == dock_state.dock_panels.end())
            {
                DockPanelPersistentState state;
                iter = dock_state.dock_panels.insert(make_pair(panel_id, state)).first;
            }
            return iter->second;
        }

        const Any* Context::get_state(ItemHandle handle, const Name& key)
        {
            lutsassert();
            ItemResult* result = get_query_result(handle);
            if(!result && !m_submitted && handle.context == get_object() && handle.generation == m_generation && key == Name("gui.open"))
            {
                for(const Node& node : m_build_desc.nodes)
                {
                    if(node.id != handle.id) continue;
                    ItemResult& fallback = get_or_create_current_result(handle.id);
                    struct BuildStateInputContext : NodeInputContext
                    {
                        Context* context = nullptr;
                        ItemResult* result = nullptr;
                        id_t node_id = 0;

                        virtual Float2U pointer_position() const override
                        {
                            return context ? context->m_pointer_pos : Float2U(0.0f);
                        }

                        virtual RectF rect() const override
                        {
                            return RectF(0.0f, 0.0f, 0.0f, 0.0f);
                        }

                        virtual const Any* get_persistent_state(const Name& key) const override
                        {
                            if(!context || !node_id) return nullptr;
                            PersistentItemState& state = context->get_or_create_persistent_state(node_id);
                            auto iter = state.custom_states.find(key);
                            return iter == state.custom_states.end() ? nullptr : &iter->second;
                        }

                        virtual void set_persistent_state(const Name& key, const Any& value) override
                        {
                            if(!context || !node_id) return;
                            PersistentItemState& state = context->get_or_create_persistent_state(node_id);
                            state.custom_states.insert_or_assign(key, value);
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

                        virtual bool is_combo_open(id_t combo_id) const override
                        {
                            if(!context || !combo_id || context->m_open_combo_id != combo_id)
                            {
                                return false;
                            }
                            PersistentItemState& state = context->get_or_create_persistent_state(combo_id);
                            return state.open;
                        }

                        virtual void open_combo_dropdown(id_t combo_id) override {}
                        virtual void close_combo_dropdown(id_t combo_id) override {}
                        virtual void open_menu_popup(id_t menu_id) override {}
                        virtual void close_popup(id_t popup_id) override {}
                        virtual void close_all_popups() override {}
                    };
                    BuildStateInputContext node_context;
                    node_context.context = this;
                    node_context.result = &fallback;
                    node_context.node_id = node.id;
                    node.update_state(node_context);
                    result = &fallback;
                    break;
                }
            }
            if(!result) return nullptr;
            auto iter = result->states.find(key);
            return iter == result->states.end() ? nullptr : &iter->second;
        }

        void Context::set_state(ItemHandle handle, const Name& key, const Any& value)
        {
            lutsassert();
            if(handle.context != get_object()) return;
            ItemResult& result = get_or_create_current_result(handle.id);
            result.states.insert_or_assign(key, value);
        }

        void Context::remove_state(ItemHandle handle, const Name& key)
        {
            lutsassert();
            if(handle.context != get_object()) return;
            auto iter = m_current_results.find(handle.id);
            if(iter == m_current_results.end()) return;
            iter->second.states.erase(key);
        }

        void Context::set_next_item_layout(const LayoutStyle& style)
        {
            lutsassert();
            m_next_item_layout = style;
            m_has_next_item_layout = true;
        }

        void Context::set_next_canvas_item_layout(const CanvasItemLayout& layout)
        {
            lutsassert();
            m_next_canvas_item_layout = layout;
            m_has_next_canvas_item_layout = true;
        }

        void Context::set_next_table_cell_color(const Float4U& color)
        {
            lutsassert();
            m_next_table_cell_color = color;
            m_has_next_table_cell_color = true;
        }
    }
}
