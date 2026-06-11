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
            FontResource default_resource;
            default_resource.font = Font::get_default_font();
            default_resource.font_index = 0;
            m_fonts.insert(make_pair(default_font_id(), move(default_resource)));
        }

        void Context::begin_frame(const FrameDesc& desc)
        {
            lutsassert();
            m_time += desc.delta_time;
            m_frame_desc = desc;
            m_submitted = false;
            ++m_generation;
            m_perf_counters = PerformanceCounters();
            m_perf_counters.frame_generation = m_generation;
            gc_states();
            m_drag_drop.begin_frame();
            m_build_desc = Description();
            m_build_desc.generation = m_generation;
            m_parent_stack.clear();
            m_layer_stack.clear();
            m_id_stack.clear();
            m_clip_stack.clear();
            m_style_stack.clear();
            m_enabled_stack.clear();
            m_child_ordinals.clear();
            BuildHintState& build_hints = build_hint_state();
            build_hints.has_next_item_layout = false;
            build_hints.has_next_item_enabled = false;
            build_hints.next_item_enabled = true;
            build_hints.has_next_canvas_item_layout = false;
            build_hints.has_next_table_cell_color = false;
            build_hints.has_next_dock_panel_style = false;
            build_hints.next_dock_panel_open = nullptr;
            build_hints.has_next_render_proxy = false;
            build_hints.next_render_proxy = RenderProxyDesc();
            tab_build_state().stack.clear();
            m_popup_stack.begin_frame();
            m_last_item_id = 0;
            m_tree_depth = 0;

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
                Ref<InputEditState> state = get_or_create_widget_state<InputEditState>(node.id);
                const String* string_value = input_text_value(node);
                if(input_text_node(node) && string_value)
                {
                    f32 font_size = get_style_value_unlocked(node.style, Name("gui.input_text.font_size"), StyleValue::f32_1(16.0f)).value.x;
                    const RectF& rect = m_layouts[i].rect;
                    state->text_cursor = clamp_utf8_cursor(*string_value, state->text_cursor);
                    RectF text_rect(rect.offset_x + 8.0f, rect.offset_y, max(rect.width - 16.0f, 1.0f), rect.height);
                    ret.active = true;
                    ret.rect = text_rect;
                    ret.cursor = (i32)(text_cursor_x(*string_value, state->text_cursor, font_size, node_font_id(node)) + 0.5f);
                    return ret;
                }
                if(numeric_text_editable(node) && state->numeric_editing)
                {
                    f32 font_size = get_style_value_unlocked(node.style, Name("gui.numeric.font_size"), StyleValue::f32_1(15.0f)).value.x;
                    state->text_cursor = clamp_utf8_cursor(state->numeric_edit_text, state->text_cursor);
                    RectF component = numeric_component_rect(node, m_layouts[i].rect, state->numeric_edit_component);
                    RectF text_rect(component.offset_x + 6.0f, component.offset_y, max(component.width - 12.0f, 1.0f), component.height);
                    ret.active = true;
                    ret.rect = text_rect;
                    ret.cursor = (i32)(text_cursor_x(state->numeric_edit_text, state->text_cursor, font_size, node_font_id(node)) + 0.5f);
                    return ret;
                }
            }
            return ret;
        }

        PerformanceCounters Context::get_performance_counters()
        {
            lutsassert();
            return m_perf_counters;
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
            bool stack_enabled = m_enabled_stack.empty() ? true : m_enabled_stack.back();
            bool next_enabled = build_hint_state().has_next_item_enabled ? build_hint_state().next_item_enabled : true;
            node.item_enabled = node.item_enabled && stack_enabled && next_enabled;
            build_hint_state().has_next_item_enabled = false;
            build_hint_state().next_item_enabled = true;
            if(!m_style_stack.empty())
            {
                node.style = m_style_stack.back();
            }
            if(build_hint_state().has_next_render_proxy)
            {
                node.render_proxy = build_hint_state().next_render_proxy;
                build_hint_state().has_next_render_proxy = false;
                build_hint_state().next_render_proxy = RenderProxyDesc();
            }
            node.interactive = node.interactive || interactive || node.default_interactive();
            if(!layer_root && dock_space_layout(m_build_desc.nodes[parent]))
            {
                node.interactive = true;
            }
            if(!m_clip_stack.empty())
            {
                node.has_user_clip_rect = true;
                node.user_clip_rect = m_clip_stack.back();
            }
            if(build_hint_state().has_next_item_layout)
            {
                node.layout_style = build_hint_state().next_item_layout;
                build_hint_state().has_next_item_layout = false;
            }

            u32 index = (u32)m_build_desc.nodes.size();
            m_build_desc.nodes.push_back(move(node_ref));
            m_child_ordinals.push_back(0);

            if(layer_root)
            {
                m_build_desc.layers[layer_index].root = index;
                build_hint_state().has_next_canvas_item_layout = false;
                build_hint_state().has_next_table_cell_color = false;
                build_hint_state().has_next_dock_panel_style = false;
                build_hint_state().next_dock_panel_open = nullptr;
                build_hint_state().has_next_item_enabled = false;
                build_hint_state().next_item_enabled = true;
                build_hint_state().has_next_render_proxy = false;
                build_hint_state().next_render_proxy = RenderProxyDesc();
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

                if(build_hint_state().has_next_canvas_item_layout)
                {
                    if(CanvasLayoutNode* canvas = canvas_layout_node(parent_node))
                    {
                        canvas->item_attachments.push_back(CanvasItemAttachment{index, m_build_desc.nodes[index].id, build_hint_state().next_canvas_item_layout});
                    }
                    build_hint_state().has_next_canvas_item_layout = false;
                }
                if(TableLayoutNode* table = table_layout_node(parent_node))
                {
                    luassert_msg(table->active_row_attachment != U32_MAX &&
                        table->active_row_attachment < table->row_attachments.size(),
                        "Table cells must be submitted between begin_table_row and end_table_row.");
                    TableRowAttachment& row = table->row_attachments[table->active_row_attachment];
                    u32 column = row.cell_count++;
                    TableCellAttachment cell;
                    cell.child_index = index;
                    cell.child_id = m_build_desc.nodes[index].id;
                    cell.row = table->active_row_attachment;
                    cell.column = column;
                    if(build_hint_state().has_next_table_cell_color)
                    {
                        cell.color_enabled = true;
                        cell.color = build_hint_state().next_table_cell_color;
                    }
                    table->cell_attachments.push_back(cell);
                    build_hint_state().has_next_table_cell_color = false;
                }
                else if(build_hint_state().has_next_table_cell_color)
                {
                    build_hint_state().has_next_table_cell_color = false;
                }
                if(build_hint_state().has_next_dock_panel_style)
                {
                    if(DockSpaceNode* dock_space = cast_node<DockSpaceNode>(parent_node))
                    {
                        dock_space->panel_attachments.push_back(DockPanelAttachment{index, m_build_desc.nodes[index].id, build_hint_state().next_dock_panel_style, build_hint_state().next_dock_panel_open});
                    }
                    build_hint_state().has_next_dock_panel_style = false;
                    build_hint_state().next_dock_panel_open = nullptr;
                }
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

        bool Context::table_row_visible(const TableLayoutNode& table, u32 row) const
        {
            if(!table_virtual_rows_enabled(table))
            {
                return true;
            }
            u32 previous_index = find_submitted_node_index(table.id);
            if(previous_index == U32_MAX || previous_index >= m_layouts.size())
            {
                return true;
            }
            const NodeLayout& layout = m_layouts[previous_index];
            if(layout.rect.width <= 0.0f || layout.rect.height <= 0.0f || layout.clip_rect.width <= 0.0f || layout.clip_rect.height <= 0.0f)
            {
                return true;
            }
            const TableStyle& style = table.desc.style;
            f32 row_height = max(table.desc.fixed_row_height, 1.0f);
            f32 separator = style.row_separators ? max(style.separator_size, 0.0f) : 0.0f;
            f32 stride = row_height + separator;
            f32 row_top = layout.rect.offset_y + style.border_size + stride * (f32)row;
            f32 row_bottom = row_top + row_height;
            f32 overscan = stride * (f32)table.desc.virtualization_overscan_rows;
            f32 clip_top = layout.clip_rect.offset_y - overscan;
            f32 clip_bottom = layout.clip_rect.offset_y + layout.clip_rect.height + overscan;
            return row_top < clip_bottom && row_bottom > clip_top;
        }

        bool Context::begin_table_row()
        {
            lutsassert();
            luassert(!m_parent_stack.empty());
            u32 parent = m_parent_stack.back();
            luassert(parent < m_build_desc.nodes.size());
            TableLayoutNode* table = table_layout_node(m_build_desc.nodes[parent]);
            luassert_msg(table, "begin_table_row must be called inside a table layout.");
            luassert_msg(table->active_row_attachment == U32_MAX, "Nested table rows are not allowed.");
            u32 row = (u32)table->row_attachments.size();
            bool visible = table_row_visible(*table, row);
            table->row_attachments.push_back(TableRowAttachment());
            if(visible)
            {
                table->active_row_attachment = (u32)table->row_attachments.size() - 1;
                return true;
            }
            return false;
        }

        void Context::end_table_row()
        {
            lutsassert();
            luassert(!m_parent_stack.empty());
            u32 parent = m_parent_stack.back();
            luassert(parent < m_build_desc.nodes.size());
            TableLayoutNode* table = table_layout_node(m_build_desc.nodes[parent]);
            luassert_msg(table, "end_table_row must be called inside a table layout.");
            luassert_msg(table->active_row_attachment != U32_MAX, "end_table_row called without a matching begin_table_row.");
            table->active_row_attachment = U32_MAX;
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
            if(!m_drag_drop.active || m_drag_drop.source_id != source.id || m_drag_drop.type != payload_type)
            {
                return false;
            }

            m_drag_drop.preview_built = true;
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
            if(!m_drag_drop.active) return;
            m_drag_drop.set_payload(data, data_size);
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
            if(!m_drag_drop.active || m_drag_drop.type != payload_type)
            {
                return false;
            }
            m_drag_drop.target_stack.push_back({target, payload_type});
            return true;
        }

        const DragDropPayload* Context::accept_drag_drop_payload(const Name& payload_type)
        {
            lutsassert();
            if(m_drag_drop.target_stack.empty()) return nullptr;
            return accept_drag_drop_payload(m_drag_drop.target_stack.back().target, payload_type);
        }

        const DragDropPayload* Context::accept_drag_drop_payload(ItemHandle target, const Name& payload_type)
        {
            lutsassert();
            if(!payload_type || target.context != get_object()) return nullptr;
            const HashMap<id_t, DragDropPayloadStorage, IdHash>& deliveries = m_submitted ? m_drag_drop.current_deliveries : m_drag_drop.last_deliveries;
            auto iter = deliveries.find(target.id);
            if(iter == deliveries.end() || iter->second.type != payload_type) return nullptr;
            return make_drag_drop_payload_view(iter->second);
        }

        void Context::end_drag_drop_target()
        {
            lutsassert();
            if(!m_drag_drop.target_stack.empty())
            {
                m_drag_drop.target_stack.pop_back();
            }
        }

        bool Context::is_drag_drop_active() const
        {
            lutsassert();
            return m_drag_drop.active;
        }

        const DragDropPayload* Context::get_drag_drop_payload()
        {
            lutsassert();
            if(!m_drag_drop.active) return nullptr;
            m_drag_drop.payload_view.type = m_drag_drop.type;
            m_drag_drop.payload_view.data = m_drag_drop.payload_data.empty() ? nullptr : m_drag_drop.payload_data.data();
            m_drag_drop.payload_view.data_size = m_drag_drop.payload_data.size();
            m_drag_drop.payload_view.source = ItemHandle{get_object(), m_drag_drop.source_id, m_generation};
            m_drag_drop.payload_view.target = ItemHandle();
            m_drag_drop.payload_view.preview = true;
            m_drag_drop.payload_view.delivery = false;
            return &m_drag_drop.payload_view;
        }

        const DragDropPayload* Context::make_drag_drop_payload_view(const DragDropPayloadStorage& storage)
        {
            m_drag_drop.payload_view.type = storage.type;
            m_drag_drop.payload_view.data = storage.data.empty() ? nullptr : storage.data.data();
            m_drag_drop.payload_view.data_size = storage.data.size();
            m_drag_drop.payload_view.source = storage.source;
            m_drag_drop.payload_view.target = storage.target;
            m_drag_drop.payload_view.preview = storage.preview;
            m_drag_drop.payload_view.delivery = storage.delivery;
            return &m_drag_drop.payload_view;
        }

        void Context::set_next_dock_panel_style(const DockPanelStyle& style, bool* open)
        {
            lutsassert();
            build_hint_state().has_next_dock_panel_style = true;
            build_hint_state().next_dock_panel_style = style;
            build_hint_state().next_dock_panel_open = open;
        }

        void Context::set_dockspace_layout(id_t dock_space, const DockSpaceLayoutDesc& desc)
        {
            lutsassert();
            if(!dock_space) return;
            Ref<DockSpaceState> dock_state_ref = get_or_create_widget_state<DockSpaceState>(dock_space);
            DockSpaceState& dock_state = *dock_state_ref;
            dock_state.dock_nodes.clear();
            dock_state.dock_root_node = U32_MAX;

            HashSet<id_t, IdHash> floating_panels;
            for(const DockSpaceFloatingPanelDesc& floating_panel : desc.floating_panels)
            {
                if(floating_panel.panel)
                {
                    floating_panels.insert(floating_panel.panel);
                }
            }

            HashSet<id_t, IdHash> docked_panels;
            Vector<u8> visiting;
            visiting.resize(desc.nodes.size());
            auto copy_node = [&](auto&& self, u32 source_index, u32 parent) -> u32 {
                if(source_index >= desc.nodes.size() || visiting[source_index]) return U32_MAX;
                visiting[source_index] = 1;
                u32 target_index = (u32)dock_state.dock_nodes.size();
                DockTreeNode target;
                target.parent = parent;
                dock_state.dock_nodes.push_back(move(target));

                const DockSpaceLayoutNodeDesc& source = desc.nodes[source_index];
                if(source.split)
                {
                    u32 child0 = self(self, source.child0, target_index);
                    u32 child1 = self(self, source.child1, target_index);
                    if(child0 != U32_MAX && child1 != U32_MAX)
                    {
                        DockTreeNode& target_node = dock_state.dock_nodes[target_index];
                        target_node.split = true;
                        target_node.child0 = child0;
                        target_node.child1 = child1;
                        target_node.split_axis = source.split_axis;
                        target_node.split_ratio = clamp(source.split_ratio, 0.08f, 0.92f);
                    }
                }
                else
                {
                    DockTreeNode& target_node = dock_state.dock_nodes[target_index];
                    for(id_t panel : source.tabs)
                    {
                        if(!panel || floating_panels.contains(panel) || docked_panels.contains(panel)) continue;
                        target_node.tabs.push_back(panel);
                        docked_panels.insert(panel);
                        DockPanelPersistentState& panel_state = get_or_create_dock_panel_state(dock_state, panel);
                        panel_state.initialized = true;
                        panel_state.closed = false;
                        panel_state.mode = DockPanelMode::docking;
                    }
                    for(id_t panel : target_node.tabs)
                    {
                        if(panel == source.selected_tab)
                        {
                            target_node.selected_tab = source.selected_tab;
                            break;
                        }
                    }
                    if(!target_node.selected_tab && !target_node.tabs.empty())
                    {
                        target_node.selected_tab = target_node.tabs[0];
                    }
                }
                visiting[source_index] = 0;
                return target_index;
            };

            if(desc.root_node != U32_MAX)
            {
                dock_state.dock_root_node = copy_node(copy_node, desc.root_node, U32_MAX);
            }

            u32 next_z_order = dock_state.dock_next_z_order;
            for(const DockSpaceFloatingPanelDesc& floating_panel : desc.floating_panels)
            {
                if(!floating_panel.panel) continue;
                DockPanelPersistentState& panel_state = get_or_create_dock_panel_state(dock_state, floating_panel.panel);
                panel_state.initialized = true;
                panel_state.closed = false;
                panel_state.mode = DockPanelMode::floating;
                panel_state.rect = floating_panel.rect;
                panel_state.z_order = floating_panel.z_order ? floating_panel.z_order : next_z_order++;
                next_z_order = max(next_z_order, panel_state.z_order + 1);
            }
            dock_state.dock_next_z_order = max(dock_state.dock_next_z_order, next_z_order);
            m_layout_dirty = true;
        }

        u64 Context::generation() const
        {
            lutsassert();
            return m_generation;
        }

        object_t Context::get_state(id_t id)
        {
            lutsassert();
            return get_state_object(id);
        }

        RV Context::set_state(id_t id, object_t data, StateLifetime lifetime)
        {
            lutsassert();
            if(!id) return BasicError::bad_arguments();
            if(!data)
            {
                clear_state(id);
                return ok;
            }
            if(lifetime == StateLifetime::persistent)
            {
                lifetime = StateLifetime::process;
            }
            StateRecord record;
            record.data = data;
            record.lifetime = lifetime;
            record.last_set_generation = m_generation;
            m_states.insert_or_assign(id, move(record));
            return ok;
        }

        void Context::clear_state(id_t id)
        {
            lutsassert();
            m_states.erase(id);
        }

        void Context::define_style(const Name& name, const Name& parent)
        {
            lutsassert();
            if(!name) return;
            auto iter = m_styles.find(name);
            if(iter == m_styles.end())
            {
                Style style;
                style.name = name;
                style.parent = style_parent_cycle(name, parent) ? Name() : parent;
                m_styles.insert(make_pair(name, move(style)));
            }
            else
            {
                iter->second.name = name;
                if(!style_parent_cycle(name, parent))
                {
                    iter->second.parent = parent;
                }
            }
        }

        bool Context::style_parent_cycle(const Name& name, const Name& parent) const
        {
            if(!name || !parent) return false;
            if(name == parent) return true;
            Name cursor = parent;
            for(usize i = 0; i < m_styles.size(); ++i)
            {
                auto iter = m_styles.find(cursor);
                if(iter == m_styles.end() || !iter->second.parent)
                {
                    return false;
                }
                cursor = iter->second.parent;
                if(cursor == name)
                {
                    return true;
                }
            }
            return true;
        }

        void Context::set_style_parent(const Name& name, const Name& parent)
        {
            lutsassert();
            if(!name) return;
            define_style(name);
            auto iter = m_styles.find(name);
            if(iter != m_styles.end() && !style_parent_cycle(name, parent))
            {
                iter->second.parent = parent;
            }
        }

        void Context::set_style_value(const Name& style_name, const Name& entry, const StyleValue& value)
        {
            lutsassert();
            if(!style_name || !entry) return;
            define_style(style_name);
            auto iter = m_styles.find(style_name);
            if(iter == m_styles.end()) return;
            StyleEntry style_entry;
            style_entry.state = StyleEntryState::set;
            style_entry.value = value;
            iter->second.entries.insert_or_assign(entry, move(style_entry));
        }

        void Context::inherit_style_entry(const Name& style_name, const Name& entry)
        {
            lutsassert();
            if(!style_name || !entry) return;
            auto iter = m_styles.find(style_name);
            if(iter == m_styles.end()) return;
            iter->second.entries.erase(entry);
        }

        void Context::unset_style_entry(const Name& style_name, const Name& entry)
        {
            lutsassert();
            if(!style_name || !entry) return;
            define_style(style_name);
            auto iter = m_styles.find(style_name);
            if(iter == m_styles.end()) return;
            StyleEntry style_entry;
            style_entry.state = StyleEntryState::unset;
            iter->second.entries.insert_or_assign(entry, move(style_entry));
        }

        StyleValue Context::get_style_value(const Name& style_name, const Name& entry, const StyleValue& default_value)
        {
            lutsassert();
            return get_style_value_unlocked(style_name, entry, default_value);
        }

        void Context::push_style(const Name& style)
        {
            lutsassert();
            luassert(style);
            m_style_stack.push_back(style);
        }

        void Context::pop_style()
        {
            lutsassert();
            luassert(!m_style_stack.empty());
            m_style_stack.pop_back();
        }

        RV Context::register_font(const Name& id, Font::IFontFile* font, u32 font_index)
        {
            lutsassert();
            if(!id || !font || font_index >= font->get_num_fonts())
            {
                return BasicError::bad_arguments();
            }
            if(m_fonts.find(id) != m_fonts.end())
            {
                return BasicError::already_exists();
            }
            FontResource resource;
            resource.font = font;
            resource.font_index = font_index;
            m_fonts.insert(make_pair(id, move(resource)));
            return ok;
        }

        FontDesc Context::get_font(const Name& id)
        {
            lutsassert();
            auto iter = m_fonts.find(id);
            if(iter == m_fonts.end())
            {
                return FontDesc();
            }
            FontDesc ret;
            ret.font = iter->second.font.get();
            ret.font_index = iter->second.font_index;
            return ret;
        }

        StyleValue Context::get_style_value_unlocked(const Name& style_name, const Name& entry, const StyleValue& default_value) const
        {
            if(!style_name || !entry) return default_value;
            Name cursor = style_name;
            for(usize i = 0; i < m_styles.size(); ++i)
            {
                auto style_iter = m_styles.find(cursor);
                if(style_iter == m_styles.end())
                {
                    return default_value;
                }
                auto entry_iter = style_iter->second.entries.find(entry);
                if(entry_iter != style_iter->second.entries.end())
                {
                    if(entry_iter->second.state == StyleEntryState::set)
                    {
                        return entry_iter->second.value;
                    }
                    if(entry_iter->second.state == StyleEntryState::unset)
                    {
                        return default_value;
                    }
                }
                if(!style_iter->second.parent)
                {
                    return default_value;
                }
                cursor = style_iter->second.parent;
            }
            return default_value;
        }

        Name Context::node_font_id(const Node& node) const
        {
            StyleValue value = get_style_value_unlocked(node.style, font_style_entry_name(), StyleValue::name(Name()));
            return value.type == StyleValueType::name ? value.name_value : Name();
        }

        FontDesc Context::resolve_font(const Name& id) const
        {
            Name cursor = id ? id : default_font_id();
            auto iter = m_fonts.find(cursor);
            if(iter == m_fonts.end())
            {
                iter = m_fonts.find(default_font_id());
            }
            if(iter != m_fonts.end())
            {
                FontDesc ret;
                ret.font = iter->second.font.get();
                ret.font_index = iter->second.font_index;
                return ret;
            }
            FontDesc ret;
            ret.font = Font::get_default_font();
            ret.font_index = 0;
            return ret;
        }

        LayoutMetrics Context::measure_text_with_font(const c8* text, usize text_size, f32 font_size, f32 max_width, const Name& font_id) const
        {
            FontDesc font = resolve_font(font_id);
            VG::TextArrangeSection section;
            section.font_file = font.font;
            section.font_index = font.font_index;
            section.font_size = font_size;
            section.num_chars = text_size;
            f32 arrange_width = max_width < F32_MAX * 0.5f ? max_width : 1000000.0f;
            auto arranged = VG::arrange_text(text ? text : "", text_size, {&section, 1},
                RectF(0.0f, 0.0f, arrange_width, 100000.0f),
                VG::TextAlignment::begin, VG::TextAlignment::begin);
            f32 w = max(arranged.bounding_rect.width, 1.0f);
            f32 h = max(arranged.bounding_rect.height, font_size + 4.0f);
            LayoutMetrics metrics;
            metrics.min_size = Float2U(min(w, 32.0f), h);
            metrics.preferred_size = Float2U(min(w, max_width), h);
            metrics.max_size = Float2U(max_width, h);
            return metrics;
        }

        f32 Context::text_cursor_x(const String& value, usize cursor, f32 font_size, const Name& font_id) const
        {
            cursor = clamp_utf8_cursor(value, cursor);
            if(!cursor) return 0.0f;
            return measure_text_with_font(value.c_str(), cursor, font_size, F32_MAX, font_id).preferred_size.x;
        }

        usize Context::text_cursor_from_x(const String& value, f32 x, f32 font_size, const Name& font_id) const
        {
            if(x <= 0.0f) return 0;
            FontDesc font = resolve_font(font_id);
            VG::TextArrangeSection section;
            section.font_file = font.font;
            section.font_index = font.font_index;
            section.font_size = font_size;
            section.num_chars = value.size();
            VG::TextArrangeResult arranged = VG::arrange_text(value.c_str(), value.size(), {&section, 1},
                RectF(0.0f, 0.0f, 1000000.0f, font_size * 2.0f),
                VG::TextAlignment::center, VG::TextAlignment::begin);
            if(arranged.lines.empty()) return value.size();
            const VG::TextLineArrangeResult& line = arranged.lines[0];
            if(line.glyphs.empty()) return value.size();
            for(usize i = 0; i < line.glyphs.size(); ++i)
            {
                const VG::TextGlyphArrangeResult& glyph = line.glyphs[i];
                f32 next_origin = i + 1 < line.glyphs.size() ?
                    line.glyphs[i + 1].origin_offset :
                    glyph.origin_offset + glyph.advance_length;
                f32 threshold = (glyph.origin_offset + next_origin) * 0.5f;
                if(x < threshold)
                {
                    return glyph.index;
                }
            }
            return value.size();
        }

        object_t Context::get_state_object(id_t id) const
        {
            auto iter = m_states.find(id);
            return iter == m_states.end() ? nullptr : iter->second.data.get();
        }

        void Context::gc_states()
        {
            for(auto iter = m_states.begin(); iter != m_states.end();)
            {
                bool expired = false;
                if(iter->second.lifetime == StateLifetime::current_frame)
                {
                    expired = iter->second.last_set_generation < m_generation;
                }
                else if(iter->second.lifetime == StateLifetime::next_frame)
                {
                    expired = iter->second.last_set_generation + 1 < m_generation;
                }
                if(expired)
                {
                    iter = m_states.erase(iter);
                }
                else
                {
                    ++iter;
                }
            }
        }

        void Context::touch_state(id_t id, StateLifetime lifetime)
        {
            object_t obj = get_state_object(id);
            if(obj)
            {
                RV r = set_state(id, obj, lifetime);
                luassert_always(succeeded(r));
            }
        }

        ItemQueryState* Context::get_query_state(ItemHandle handle)
        {
            if(handle.context != get_object()) return nullptr;
            if(handle.generation != m_generation) return nullptr;
            return get_widget_state<ItemQueryState>(handle.id);
        }

        Ref<ItemQueryState> Context::get_or_create_query_state(id_t id)
        {
            return get_or_create_widget_state<ItemQueryState>(id, StateLifetime::next_frame);
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

        DockPanelPersistentState& Context::get_or_create_dock_panel_state(DockSpaceState& dock_state, id_t panel_id)
        {
            auto iter = dock_state.dock_panels.find(panel_id);
            if(iter == dock_state.dock_panels.end())
            {
                DockPanelPersistentState state;
                iter = dock_state.dock_panels.insert(make_pair(panel_id, state)).first;
            }
            return iter->second;
        }

        const Any* Context::get_item_query_state(ItemHandle handle, const Name& key)
        {
            lutsassert();
            ItemQueryState* state = get_query_state(handle);
            if(!state) return nullptr;
            auto iter = state->states.find(key);
            return iter == state->states.end() ? nullptr : &iter->second;
        }

        void Context::set_item_query_state(ItemHandle handle, const Name& key, const Any& value)
        {
            lutsassert();
            if(handle.context != get_object() || handle.generation != m_generation) return;
            Ref<ItemQueryState> state = get_or_create_query_state(handle.id);
            state->states.insert_or_assign(key, value);
            RV r = set_state(make_state_id<ItemQueryState>(handle.id), state.object(), StateLifetime::next_frame);
            luassert_always(succeeded(r));
        }

        void Context::set_item_query_state_if_absent(id_t id, const Name& key, const Any& value)
        {
            lutsassert();
            if(!id) return;
            Ref<ItemQueryState> state = get_or_create_query_state(id);
            if(state->states.find(key) == state->states.end())
            {
                state->states.insert(make_pair(key, value));
            }
        }

        void Context::remove_item_query_state(ItemHandle handle, const Name& key)
        {
            lutsassert();
            if(handle.context != get_object() || handle.generation != m_generation) return;
            ItemQueryState* state = get_widget_state<ItemQueryState>(handle.id);
            if(state)
            {
                state->states.erase(key);
            }
        }

        void Context::set_next_item_layout(const LayoutStyle& style)
        {
            lutsassert();
            build_hint_state().next_item_layout = style;
            build_hint_state().has_next_item_layout = true;
        }

        void Context::set_next_item_render_proxy(const RenderProxyDesc& proxy)
        {
            lutsassert();
            build_hint_state().next_render_proxy = proxy;
            build_hint_state().has_next_render_proxy = true;
        }

        void Context::set_next_item_enabled(bool enabled)
        {
            lutsassert();
            set_next_item_enabled_internal(enabled);
        }

        void Context::push_enabled(bool enabled)
        {
            lutsassert();
            bool parent_enabled = m_enabled_stack.empty() ? true : m_enabled_stack.back();
            m_enabled_stack.push_back(parent_enabled && enabled);
        }

        void Context::pop_enabled()
        {
            lutsassert();
            luassert(!m_enabled_stack.empty());
            m_enabled_stack.pop_back();
        }

        void Context::set_next_item_enabled_internal(bool enabled)
        {
            lutsassert();
            build_hint_state().next_item_enabled = enabled;
            build_hint_state().has_next_item_enabled = true;
        }

        void Context::set_next_canvas_item_layout(const CanvasItemLayout& layout)
        {
            lutsassert();
            build_hint_state().next_canvas_item_layout = layout;
            build_hint_state().has_next_canvas_item_layout = true;
        }

        void Context::set_next_table_cell_color(const Float4U& color)
        {
            lutsassert();
            build_hint_state().next_table_cell_color = color;
            build_hint_state().has_next_table_cell_color = true;
        }
    }
}
