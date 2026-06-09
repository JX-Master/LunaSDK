/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUIDebugPanel.cpp
* @author JXMaster
* @date 2026/6/3
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "../GUI.hpp"

#ifdef LUNA_GUI_ENABLE_DEBUG

namespace Luna
{
    namespace GUI
    {
        namespace
        {
            bool same_key_exact(const DebugNodeKey& lhs, const DebugNodeKey& rhs)
            {
                return lhs.generation == rhs.generation &&
                    lhs.layer_id == rhs.layer_id &&
                    lhs.node_index == rhs.node_index &&
                    lhs.node_id == rhs.node_id;
            }

            bool same_key_stable(const DebugNodeKey& lhs, const DebugNodeKey& rhs)
            {
                return lhs.layer_id == rhs.layer_id && lhs.node_id == rhs.node_id;
            }

            const DebugNodeInfo* find_node_exact(const DebugInfo& info, const DebugNodeKey& key)
            {
                for(const DebugNodeInfo& node : info.nodes)
                {
                    if(same_key_exact(node.key, key))
                    {
                        return &node;
                    }
                }
                return nullptr;
            }

            const DebugNodeInfo* find_node_stable(const DebugInfo& info, const DebugNodeKey& key)
            {
                for(const DebugNodeInfo& node : info.nodes)
                {
                    if(same_key_stable(node.key, key))
                    {
                        return &node;
                    }
                }
                return nullptr;
            }

            const DebugNodeInfo* find_node_by_index(const DebugInfo& info, u32 index)
            {
                for(const DebugNodeInfo& node : info.nodes)
                {
                    if(node.key.node_index == index)
                    {
                        return &node;
                    }
                }
                return nullptr;
            }

            bool node_hidden_by_debug_filter(const DebugNodeInfo& node, const DebugPanelDesc& desc)
            {
                return node.debug_layer_node && !desc.show_debug_layer_nodes;
            }

            bool has_visible_child(const DebugInfo& info, const DebugNodeInfo& node, const DebugPanelDesc& desc)
            {
                for(u32 child = node.first_child; child != U32_MAX;)
                {
                    const DebugNodeInfo* child_node = find_node_by_index(info, child);
                    if(!child_node) return false;
                    if(!node_hidden_by_debug_filter(*child_node, desc))
                    {
                        return true;
                    }
                    child = child_node->next_sibling;
                }
                return false;
            }

            String format_id(id_t id)
            {
                String ret;
                strprintf(ret, "%016llX", (unsigned long long)id);
                return ret;
            }

            String format_rect(const RectF& rect)
            {
                String ret;
                strprintf(ret, "(%.1f, %.1f, %.1f, %.1f)", rect.offset_x, rect.offset_y, rect.width, rect.height);
                return ret;
            }

            String format_float2(const Float2U& value)
            {
                String ret;
                strprintf(ret, "(%.1f, %.1f)", value.x, value.y);
                return ret;
            }

            String format_style_value(const StyleValue& value)
            {
                String ret;
                switch(value.type)
                {
                case StyleValueType::f32_1:
                    strprintf(ret, "%.3f", value.value.x);
                    break;
                case StyleValueType::f32_2:
                    strprintf(ret, "(%.3f, %.3f)", value.value.x, value.value.y);
                    break;
                case StyleValueType::f32_3:
                    strprintf(ret, "(%.3f, %.3f, %.3f)", value.value.x, value.value.y, value.value.z);
                    break;
                case StyleValueType::name:
                    ret = value.name_value.c_str();
                    break;
                default:
                    strprintf(ret, "(%.3f, %.3f, %.3f, %.3f)", value.value.x, value.value.y, value.value.z, value.value.w);
                    break;
                }
                return ret;
            }

            void draw_debug_outline(IContext* context, const RectF& rect, const Float4U& color, f32 width)
            {
                if(rect.width <= 0.0f || rect.height <= 0.0f) return;
                f32 w = max(width, 1.0f);
                draw_rect(context, RectF(rect.offset_x, rect.offset_y, rect.width, w), color, 0.0f);
                draw_rect(context, RectF(rect.offset_x, rect.offset_y + max(rect.height - w, 0.0f), rect.width, w), color, 0.0f);
                draw_rect(context, RectF(rect.offset_x, rect.offset_y, w, rect.height), color, 0.0f);
                draw_rect(context, RectF(rect.offset_x + max(rect.width - w, 0.0f), rect.offset_y, w, rect.height), color, 0.0f);
            }

            void draw_node_overlay(IContext* context, const DebugInfo& info, const DebugInspectorState& state)
            {
                if(state.has_tree_hovered_node)
                {
                    if(const DebugNodeInfo* node = find_node_stable(info, state.tree_hovered_node))
                    {
                        draw_debug_outline(context, node->layout.screen_rect, Float4U(1.0f, 0.83f, 0.20f, 0.95f), 2.0f);
                    }
                }
                if(state.has_selected_node)
                {
                    if(const DebugNodeInfo* node = find_node_stable(info, state.selected_node))
                    {
                        draw_debug_outline(context, node->layout.screen_rect, Float4U(0.30f, 0.58f, 1.0f, 0.95f), 2.0f);
                        draw_debug_outline(context, node->layout.screen_clip_rect, Float4U(0.30f, 0.58f, 1.0f, 0.35f), 1.0f);
                    }
                }
                if(info.context.has_main_hovered_node)
                {
                    if(const DebugNodeInfo* node = find_node_exact(info, info.context.main_hovered_node))
                    {
                        draw_debug_outline(context, node->layout.screen_rect, Float4U(0.30f, 0.92f, 0.48f, 0.95f), 2.0f);
                    }
                }
            }

            Name debug_section_title_style()
            {
                return Name("gui.debug_panel.section_title");
            }

            Name debug_muted_text_style()
            {
                return Name("gui.debug_panel.muted_text");
            }

            Name debug_section_spacer_style()
            {
                return Name("gui.debug_panel.section_spacer");
            }

            void ensure_debug_panel_styles(IContext* context)
            {
                define_style(context, debug_section_title_style());
                set_style_f32(context, debug_section_title_style(), Name("gui.text.font_size"), 20.0f);
                set_style_f32x4(context, debug_section_title_style(), Name("gui.text.color"), Float4U(0.78f, 0.88f, 1.0f, 1.0f));

                define_style(context, debug_muted_text_style());
                set_style_f32(context, debug_muted_text_style(), Name("gui.text.font_size"), 15.0f);
                set_style_f32x4(context, debug_muted_text_style(), Name("gui.text.color"), Float4U(0.58f, 0.66f, 0.76f, 1.0f));

                define_style(context, debug_section_spacer_style());
                set_style_f32(context, debug_section_spacer_style(), Name("gui.text.font_size"), 6.0f);
                set_style_f32x4(context, debug_section_spacer_style(), Name("gui.text.color"), Float4U(0.0f));
            }

            void debug_section_title(IContext* context, const c8* title)
            {
                push_style(context, debug_section_title_style());
                text(context, title);
                pop_style(context);
            }

            void debug_section_gap(IContext* context)
            {
                push_style(context, debug_section_spacer_style());
                text(context, "");
                pop_style(context);
            }

            void debug_muted_text(IContext* context, const c8* value)
            {
                push_style(context, debug_muted_text_style());
                text(context, value);
                pop_style(context);
            }

            void debug_text(IContext* context, const c8* label, const String& value)
            {
                String line;
                line.reserve(strlen(label) + value.size() + 4);
                line.append(label);
                line.append(": ");
                line.append(value);
                text(context, line.c_str());
            }

            void debug_text_bool(IContext* context, const c8* label, bool value)
            {
                debug_text(context, label, String(value ? "true" : "false"));
            }

            void draw_node_details(IContext* context, const DebugInfo& info, const DebugInspectorState& state)
            {
                const DebugNodeInfo* node = state.has_selected_node ? find_node_stable(info, state.selected_node) : nullptr;
                if(!node)
                {
                    text(context, "Select a node in the tree to inspect it.");
                    return;
                }

                debug_section_title(context, "Identity");
                debug_text(context, "Type", node->type_name);
                debug_text(context, "Text", node->text);
                debug_text(context, "ID", format_id(node->key.node_id));
                debug_text(context, "Layer", format_id(node->key.layer_id));
                debug_text(context, "Index", [&]() { String s; strprintf(s, "%u", node->key.node_index); return s; }());
                debug_text(context, "Parent", node->parent == U32_MAX ? String("none") : [&]() { String s; strprintf(s, "%u", node->parent); return s; }());

                debug_section_gap(context);
                debug_section_title(context, "Flags");
                debug_text_bool(context, "Interactive", node->interactive);
                debug_text_bool(context, "Hit Testable", node->hit_testable);
                debug_text_bool(context, "Visible", node->visible);

                debug_section_gap(context);
                debug_section_title(context, "Layout");
                debug_text(context, "Screen Rect", format_rect(node->layout.screen_rect));
                debug_text(context, "Screen Clip", format_rect(node->layout.screen_clip_rect));
                debug_text(context, "Layer Rect", format_rect(node->layout.layer_rect));
                debug_text(context, "Layer Clip", format_rect(node->layout.layer_clip_rect));
                debug_text(context, "Preferred", format_float2(node->layout.metrics.preferred_size));
                debug_text(context, "Min", format_float2(node->layout.metrics.min_size));
                debug_text(context, "Max", format_float2(node->layout.metrics.max_size));

                debug_section_gap(context);
                debug_section_title(context, "Render Proxy");
                debug_text_bool(context, "Draw", node->render_proxy_has_draw);
                debug_text_bool(context, "Draw After Children", node->render_proxy_has_draw_after_children);

                debug_section_gap(context);
                debug_section_title(context, "Style");
                debug_text(context, "Bound Style", String(node->style.c_str()));
                u32 shown_styles = 0;
                for(const DebugResolvedStyleEntryInfo& entry : node->resolved_style)
                {
                    if(shown_styles++ >= 12)
                    {
                        debug_muted_text(context, "...");
                        break;
                    }
                    String line;
                    if(entry.found)
                    {
                        strprintf(line, "%s = %s", entry.name.c_str(), format_style_value(entry.value).c_str());
                    }
                    else if(entry.unset)
                    {
                        strprintf(line, "%s = <unset>", entry.name.c_str());
                    }
                    else
                    {
                        strprintf(line, "%s = <default>", entry.name.c_str());
                    }
                    text(context, line.c_str());
                }

                debug_section_gap(context);
                debug_section_title(context, "Item State");
                if(node->item_query_states.empty())
                {
                    debug_muted_text(context, "<none>");
                }
                for(const auto& item_state : node->item_query_states)
                {
                    String line;
                    strprintf(line, "%s = %s", item_state.first.c_str(), item_state.second.value.c_str());
                    text(context, line.c_str());
                }
            }

            void draw_debug_tree_node(IContext* context, const DebugInfo& info, const DebugNodeInfo& node,
                DebugInspectorState& state, const DebugPanelDesc& desc)
            {
                if(node_hidden_by_debug_filter(node, desc)) return;
                bool has_child = has_visible_child(info, node, desc);
                bool selected = state.has_selected_node && same_key_stable(state.selected_node, node.key);
                bool main_hovered = info.context.has_main_hovered_node && same_key_stable(info.context.main_hovered_node, node.key);
                TreeNodeFlag flags = TreeNodeFlag::open_on_arrow;
                if(!has_child)
                {
                    flags = (TreeNodeFlag)((u32)flags | (u32)TreeNodeFlag::leaf);
                }
                if(selected || main_hovered)
                {
                    flags = (TreeNodeFlag)((u32)flags | (u32)TreeNodeFlag::selected);
                }
                if(node.depth <= 1)
                {
                    flags = (TreeNodeFlag)((u32)flags | (u32)TreeNodeFlag::default_open);
                }
                String label;
                strprintf(label, "[%u] %s%s%s",
                    node.key.node_index,
                    node.type_name.empty() ? "<Node>" : node.type_name.c_str(),
                    node.text.empty() ? "" : " - ",
                    node.text.empty() ? "" : node.text.c_str());
                push_id(context, (u64)node.key.node_index);
                ItemHandle row = tree_node(context, label.c_str(), flags);
                if(is_item_hovered(row))
                {
                    state.has_tree_hovered_node = true;
                    state.tree_hovered_node = node.key;
                }
                if(is_item_clicked(row))
                {
                    state.has_selected_node = true;
                    state.selected_node = node.key;
                }
                if(has_child && get_item_state(row, State::open()))
                {
                    tree_push(context, row);
                    for(u32 child = node.first_child; child != U32_MAX;)
                    {
                        const DebugNodeInfo* child_node = find_node_by_index(info, child);
                        if(!child_node) break;
                        draw_debug_tree_node(context, info, *child_node, state, desc);
                        child = child_node->next_sibling;
                    }
                    tree_pop(context);
                }
                pop_id(context);
            }

            void draw_debug_tree(IContext* context, const DebugInfo& info, DebugInspectorState& state, const DebugPanelDesc& desc)
            {
                for(const DebugLayerInfo& layer : info.layers)
                {
                    if(layer.debug_layer && !desc.show_debug_layer_nodes)
                    {
                        continue;
                    }
                    String label;
                    strprintf(label, "Layer %u  %s", layer.index, format_id(layer.id).c_str());
                    push_id(context, layer.id);
                    ItemHandle layer_row = tree_node(context, label.c_str(), TreeNodeFlag::default_open | TreeNodeFlag::open_on_arrow);
                    if(get_item_state(layer_row, State::open()))
                    {
                        tree_push(context, layer_row);
                        const DebugNodeInfo* root = find_node_by_index(info, layer.root);
                        if(root)
                        {
                            draw_debug_tree_node(context, info, *root, state, desc);
                        }
                        tree_pop(context);
                    }
                    pop_id(context);
                }
            }
        }

        LUNA_GUI_API ItemHandle show_debug_info(IContext* context, const DebugInfo& info, const DebugPanelDesc& desc)
        {
            ensure_debug_panel_styles(context);
            Ref<DebugInspectorState> state = get_or_create_state<DebugInspectorState>(context, desc.layer_id, StateLifetime::process);
            if(!state)
            {
                return ItemHandle();
            }
            if(state->has_selected_node && !find_node_stable(info, state->selected_node))
            {
                state->has_selected_node = false;
            }
            state->has_main_hovered_node = info.context.has_main_hovered_node;
            state->main_hovered_node = info.context.main_hovered_node;
            DebugInspectorState draw_state = *state.get();
            state->has_tree_hovered_node = false;

            FrameDesc frame = get_frame_desc(context);
            Float2U screen_size(max(frame.surface_size.x, 1.0f), max(frame.surface_size.y, 1.0f));
            CanvasLayoutDesc canvas_desc;
            canvas_desc.clip_children = false;
            push_layer(context, desc.layer_id, Float2U(0.0f));
            ItemHandle root = begin_canvas_layout(context, "GUI Debug Layer", Size::fixed(screen_size.x, screen_size.y), canvas_desc);

            set_next_canvas_item_layout(context, CanvasItemLayout::fixed(Float2U(0.0f), screen_size));
            begin_canvas_layout(context, "GUI Debug Overlay", Size::fixed(screen_size.x, screen_size.y), canvas_desc);
            draw_node_overlay(context, info, draw_state);
            end_canvas_layout(context);

            f32 panel_width = max(desc.size.width, 320.0f);
            f32 panel_height = max(desc.size.height, 220.0f);
            DockPanelStyle panel_style;
            panel_style.close_button = false;
            panel_style.initial_mode = DockPanelMode::floating;
            panel_style.floating_position = desc.screen_position;
            panel_style.floating_size = Float2U(panel_width, panel_height);
            panel_style.min_floating_size = Float2U(320.0f, 220.0f);
            set_next_canvas_item_layout(context, CanvasItemLayout::fixed(Float2U(0.0f), screen_size));
            begin_dock_space(context, "GUI Debug DockSpace", Size::fixed(screen_size.x, screen_size.y));
            begin_dock_panel(context, "GUI Debug Panel", nullptr, panel_style);
            LayoutDesc row;
            row.gap = 8.0f;
            row.cross_axis_alignment = LayoutCrossAxisAlignment::stretch;
            set_next_item_layout(context, LayoutStyle::fill());
            begin_h_layout(context, "Debug Panel Body", row);

            LayoutStyle tree_style = LayoutStyle::fill(0.44f);
            tree_style.min_size = Float2U(240.0f, 120.0f);
            set_next_item_layout(context, tree_style);
            LayoutDesc tree_column;
            tree_column.gap = 6.0f;
            tree_column.cross_axis_alignment = LayoutCrossAxisAlignment::stretch;
            begin_v_layout(context, "Widget Tree Column", tree_column);
            text(context, "Widget Tree");
            set_next_item_layout(context, LayoutStyle::fill());
            begin_scroll_view(context, "Widget Tree Scroll", Size());
            draw_debug_tree(context, info, *state.get(), desc);
            end_scroll_view(context);
            end_v_layout(context);

            LayoutStyle details_style = LayoutStyle::fill(0.56f);
            details_style.min_size = Float2U(240.0f, 120.0f);
            set_next_item_layout(context, details_style);
            begin_scroll_view(context, "Node Details", Size());
            draw_node_details(context, info, *state.get());
            end_scroll_view(context);

            end_h_layout(context);
            end_dock_panel(context);
            end_dock_space(context);
            end_canvas_layout(context);
            pop_layer(context);
            return root;
        }
    }
}

#endif
