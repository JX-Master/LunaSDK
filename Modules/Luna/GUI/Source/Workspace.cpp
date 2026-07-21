/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Workspace.cpp
* @author JXMaster
* @date 2026/7/16
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "Internal.hpp"
#include <Luna/VG/TextArranger.hpp>
#include <cstring>

namespace Luna
{
    namespace GUI
    {
        namespace Internal
        {
            struct DockSpaceLayoutData
            {
                DockSpaceAction* action = nullptr;
            };

            struct DockSpaceDrawData
            {
                DockSpaceAction* action = nullptr;
            };

            struct DockPanelDrawData
            {
                DockSpaceAction* action = nullptr;
                DockPanelActionInfo* panel = nullptr;
            };

            static RectF intersect_rect(const RectF& a, const RectF& b)
            {
                f32 min_x = max(a.offset_x, b.offset_x);
                f32 min_y = max(a.offset_y, b.offset_y);
                f32 max_x = min(a.offset_x + a.width, b.offset_x + b.width);
                f32 max_y = min(a.offset_y + a.height, b.offset_y + b.height);
                return RectF(min_x, min_y, max(max_x - min_x, 0.0f), max(max_y - min_y, 0.0f));
            }

            static bool point_in_rect(const RectF& rect, const Float2U& point)
            {
                return point.x >= rect.offset_x && point.y >= rect.offset_y &&
                    point.x < rect.offset_x + rect.width && point.y < rect.offset_y + rect.height;
            }

            static bool rect_valid(const RectF& rect)
            {
                return rect.width > 0.0f && rect.height > 0.0f;
            }

            static Ref<DockSpaceState> dock_space_state(GUICore::IContext* context, id_t id)
            {
                id_t state_id = GUICore::make_state_id<DockSpaceState>(id);
                Ref<DockSpaceState> state;
                if(object_t object = context->get_state(state_id))
                {
                    object_retain(object);
                    state.attach(object);
                }
                else
                {
                    state = new_object<DockSpaceState>();
                }
                lupanic_if_failed(context->set_state(state_id, state.object(), GUICore::StateLifetime::process));
                return state;
            }

            static DockPanelPersistentData* find_panel(DockSpaceState& state, id_t id)
            {
                for(DockPanelPersistentData& panel : state.panels)
                {
                    if(panel.id == id) return &panel;
                }
                return nullptr;
            }

            static const DockPanelPersistentData* find_panel(const DockSpaceState& state, id_t id)
            {
                for(const DockPanelPersistentData& panel : state.panels)
                {
                    if(panel.id == id) return &panel;
                }
                return nullptr;
            }

            static DockPanelPersistentData& get_or_add_panel(DockSpaceState& state, id_t id)
            {
                if(DockPanelPersistentData* panel = find_panel(state, id)) return *panel;
                DockPanelPersistentData panel;
                panel.id = id;
                panel.z_order = state.next_z_order++;
                state.panels.push_back(panel);
                return state.panels.back();
            }

            static u32 new_leaf(DockSpaceState& state, id_t panel = 0)
            {
                DockTreeNode node;
                if(panel)
                {
                    node.tabs.push_back(panel);
                    node.selected_tab = panel;
                }
                state.nodes.push_back(move(node));
                return (u32)state.nodes.size() - 1;
            }

            static u32 find_panel_leaf(const DockSpaceState& state, id_t panel)
            {
                for(u32 i = 0; i < state.nodes.size(); ++i)
                {
                    const DockTreeNode& node = state.nodes[i];
                    if(!node.active || node.split) continue;
                    for(id_t tab : node.tabs)
                    {
                        if(tab == panel) return i;
                    }
                }
                return U32_MAX;
            }

            static u32 first_leaf(const DockSpaceState& state, u32 node_index)
            {
                if(node_index >= state.nodes.size()) return U32_MAX;
                const DockTreeNode& node = state.nodes[node_index];
                if(!node.active) return U32_MAX;
                if(!node.split) return node_index;
                u32 result = first_leaf(state, node.child0);
                return result != U32_MAX ? result : first_leaf(state, node.child1);
            }

            static void normalize_leaf(DockTreeNode& node)
            {
                if(node.split) return;
                bool selected_found = false;
                for(id_t tab : node.tabs)
                {
                    if(tab == node.selected_tab)
                    {
                        selected_found = true;
                        break;
                    }
                }
                if(!selected_found) node.selected_tab = node.tabs.empty() ? 0 : node.tabs.front();
            }

            static void replace_node_with_child(DockSpaceState& state, u32 node_index, u32 child_index)
            {
                if(node_index >= state.nodes.size() || child_index >= state.nodes.size()) return;
                u32 parent = state.nodes[node_index].parent;
                DockTreeNode replacement = state.nodes[child_index];
                replacement.parent = parent;
                state.nodes[child_index].active = false;
                state.nodes[node_index] = move(replacement);
                DockTreeNode& node = state.nodes[node_index];
                if(node.split)
                {
                    if(node.child0 < state.nodes.size()) state.nodes[node.child0].parent = node_index;
                    if(node.child1 < state.nodes.size()) state.nodes[node.child1].parent = node_index;
                }
            }

            static void remove_panel_from_tree(DockSpaceState& state, id_t panel)
            {
                u32 leaf_index = find_panel_leaf(state, panel);
                if(leaf_index == U32_MAX) return;
                DockTreeNode& leaf = state.nodes[leaf_index];
                for(usize i = 0; i < leaf.tabs.size(); ++i)
                {
                    if(leaf.tabs[i] == panel)
                    {
                        leaf.tabs.erase(leaf.tabs.begin() + i);
                        break;
                    }
                }
                normalize_leaf(leaf);
                if(!leaf.tabs.empty()) return;
                if(leaf_index == state.root_node)
                {
                    leaf.active = false;
                    state.root_node = U32_MAX;
                    return;
                }
                u32 parent = leaf.parent;
                if(parent >= state.nodes.size()) return;
                DockTreeNode& branch = state.nodes[parent];
                u32 sibling = branch.child0 == leaf_index ? branch.child1 : branch.child0;
                leaf.active = false;
                replace_node_with_child(state, parent, sibling);
            }

            static void remove_panel(DockSpaceState& state, id_t panel)
            {
                remove_panel_from_tree(state, panel);
                for(usize i = 0; i < state.panels.size(); ++i)
                {
                    if(state.panels[i].id == panel)
                    {
                        state.panels.erase(state.panels.begin() + i);
                        break;
                    }
                }
                if(state.drag_panel == panel)
                {
                    state.drag_panel = 0;
                    state.drag_mode = DockDragMode::none;
                }
            }

            static void add_panel_to_default_leaf(DockSpaceState& state, id_t panel)
            {
                if(find_panel_leaf(state, panel) != U32_MAX) return;
                if(state.root_node == U32_MAX || state.root_node >= state.nodes.size())
                {
                    state.root_node = new_leaf(state, panel);
                    return;
                }
                u32 leaf_index = first_leaf(state, state.root_node);
                if(leaf_index == U32_MAX)
                {
                    state.root_node = new_leaf(state, panel);
                    return;
                }
                DockTreeNode& leaf = state.nodes[leaf_index];
                leaf.tabs.push_back(panel);
                if(!leaf.selected_tab) leaf.selected_tab = panel;
            }

            static void dock_panel(DockSpaceState& state, id_t panel, u32 target, DockDropDirection direction)
            {
                DockPanelPersistentData& panel_state = get_or_add_panel(state, panel);
                if(panel_state.mode == DockPanelMode::floating)
                {
                    panel_state.restored_floating_rect = panel_state.floating_rect;
                }
                remove_panel_from_tree(state, panel);
                panel_state.mode = DockPanelMode::docking;

                if(state.root_node == U32_MAX || target == U32_MAX || target >= state.nodes.size() ||
                    !state.nodes[target].active)
                {
                    state.root_node = new_leaf(state, panel);
                    return;
                }
                if(direction == DockDropDirection::center)
                {
                    DockTreeNode& leaf = state.nodes[target];
                    if(!leaf.split)
                    {
                        leaf.tabs.push_back(panel);
                        leaf.selected_tab = panel;
                        return;
                    }
                }

                DockTreeNode previous = state.nodes[target];
                u32 old_child = (u32)state.nodes.size();
                previous.parent = target;
                state.nodes.push_back(move(previous));
                u32 new_child = new_leaf(state, panel);
                state.nodes[new_child].parent = target;
                DockTreeNode branch;
                branch.split = true;
                branch.parent = state.nodes[target].parent;
                branch.split_axis = direction == DockDropDirection::left || direction == DockDropDirection::right ?
                    DockSplitAxis::x : DockSplitAxis::y;
                branch.split_ratio = 0.5f;
                bool new_first = direction == DockDropDirection::left || direction == DockDropDirection::up;
                branch.child0 = new_first ? new_child : old_child;
                branch.child1 = new_first ? old_child : new_child;
                state.nodes[target] = move(branch);
                state.nodes[old_child].parent = target;
                state.nodes[new_child].parent = target;
            }

            static void arrange_dock_node(DockSpaceState& state, u32 node_index, const RectF& rect,
                f32 splitter_size)
            {
                if(node_index >= state.nodes.size()) return;
                DockTreeNode& node = state.nodes[node_index];
                if(!node.active) return;
                node.rect = rect;
                node.splitter_rect = RectF();
                if(!node.split)
                {
                    normalize_leaf(node);
                    return;
                }
                f32 ratio = clamp(node.split_ratio, 0.01f, 0.99f);
                f32 splitter = max(splitter_size, 0.0f);
                if(node.split_axis == DockSplitAxis::x)
                {
                    f32 available = max(rect.width - splitter, 0.0f);
                    f32 first = available * ratio;
                    node.splitter_rect = RectF(rect.offset_x + first, rect.offset_y, splitter, rect.height);
                    arrange_dock_node(state, node.child0, RectF(rect.offset_x, rect.offset_y, first, rect.height), splitter_size);
                    arrange_dock_node(state, node.child1, RectF(rect.offset_x + first + splitter, rect.offset_y,
                        max(available - first, 0.0f), rect.height), splitter_size);
                }
                else
                {
                    f32 available = max(rect.height - splitter, 0.0f);
                    f32 first = available * ratio;
                    node.splitter_rect = RectF(rect.offset_x, rect.offset_y + first, rect.width, splitter);
                    arrange_dock_node(state, node.child0, RectF(rect.offset_x, rect.offset_y, rect.width, first), splitter_size);
                    arrange_dock_node(state, node.child1, RectF(rect.offset_x, rect.offset_y + first + splitter,
                        rect.width, max(available - first, 0.0f)), splitter_size);
                }
            }

            static DockPanelActionInfo* find_action_panel(DockSpaceAction& action, id_t id)
            {
                for(usize i = 0; i < action.panel_count; ++i)
                {
                    if(action.panels[i].id == id) return action.panels + i;
                }
                return nullptr;
            }

            static const c8* panel_label(DockSpaceAction& action, id_t id)
            {
                DockPanelActionInfo* panel = find_action_panel(action, id);
                return panel && panel->label ? panel->label : "Panel";
            }

            static f32 dock_tab_label_width(GUICore::IContext* context,
                const GUICore::ElementHandle& element, const c8* label, f32 font_size)
            {
                const c8* text = label ? label : "";
                usize text_size = strlen(text);
                if(!text_size) return 0.0f;
                f32 fallback = (f32)text_size * font_size * 0.58f;
                GUICore::FontDesc font = context->get_font(style_name(context, element, "gui.font"));
                if(!font.font)
                {
                    font.font = Font::get_default_font();
                    font.font_index = 0;
                }
                if(!font.font) return fallback;
                VG::TextArrangeSection section;
                section.font_file = font.font;
                section.font_index = font.font_index;
                section.font_size = font_size;
                section.num_chars = text_size;
                VG::TextArrangeResult arranged = VG::arrange_text(text, text_size,
                    Span<const VG::TextArrangeSection>(&section, 1),
                    RectF(0.0f, 0.0f, 1000000.0f, max(font_size * 2.0f, 1.0f)),
                    VG::TextAlignment::begin, VG::TextAlignment::begin);
                f32 width = arranged.bounding_rect.width;
                if(!arranged.lines.empty() && !arranged.lines[0].glyphs.empty())
                {
                    const VG::TextGlyphArrangeResult& last = arranged.lines[0].glyphs.back();
                    width = max(width, last.origin_offset + last.advance_length);
                }
                return width > 0.0f ? ceil(width) + 1.0f : fallback;
            }

            static f32 dock_tab_width(GUICore::IContext* context,
                const GUICore::ElementHandle& element, const c8* label, f32 font_size)
            {
                f32 padding = style_scalar(context, element, "gui.tab.padding_x", 14.0f);
                return dock_tab_label_width(context, element, label, font_size) + padding * 2.0f;
            }

            static f32 dock_tab_width_scale(GUICore::IContext* context,
                const GUICore::ElementHandle& element, DockSpaceAction& action,
                const DockTreeNode& leaf, f32 font_size, f32 available_width)
            {
                f32 natural_width = 0.0f;
                for(id_t tab : leaf.tabs)
                {
                    natural_width += dock_tab_width(context, element, panel_label(action, tab), font_size);
                }
                return natural_width > available_width && natural_width > 0.0f ?
                    max(available_width, 0.0f) / natural_width : 1.0f;
            }

            static RV layout_dock_space(GUICore::IContext* context, const GUICore::ElementHandle& element,
                const RectF& rect, void* userdata)
            {
                DockSpaceLayoutData* data = (DockSpaceLayoutData*)userdata;
                if(!context || !data || !data->action || !data->action->state) return BasicError::bad_arguments();
                DockSpaceAction& action = *data->action;
                DockSpaceState& state = *action.state;
                const GUICore::Element* root = context->get_element(element.index);
                if(!root) return BasicError::bad_arguments();
                RectF content(rect.offset_x + root->layout.padding.x, rect.offset_y + root->layout.padding.y,
                    max(rect.width - root->layout.padding.x - root->layout.padding.z, 0.0f),
                    max(rect.height - root->layout.padding.y - root->layout.padding.w, 0.0f));
                RectF parent_clip = rect_valid(root->layout_result.clip_rect) ? root->layout_result.clip_rect : rect;
                RectF child_clip = intersect_rect(content, parent_clip);
                arrange_dock_node(state, state.root_node, content, action.desc.splitter_size);

                Span<const GUICore::Layer> layers = context->get_layers();
                Float2U layer_position(0.0f);
                if(root->layer < layers.size()) layer_position = layers[root->layer].screen_position;
                state.screen_origin = layer_position;
                state.dock_rect = content;
                context->set_layer_screen_position(action.indicator_layer_id, state.screen_origin);

                for(usize i = 0; i < action.panel_count; ++i)
                {
                    DockPanelActionInfo& panel = action.panels[i];
                    DockPanelPersistentData* persistent = find_panel(state, panel.id);
                    if(!persistent || !panel.visible) continue;
                    if(persistent->mode == DockPanelMode::floating)
                    {
                        context->set_layer_screen_position(panel.layer_id,
                            Float2U(state.screen_origin.x + state.dock_rect.offset_x +
                                persistent->floating_rect.offset_x,
                                state.screen_origin.y + state.dock_rect.offset_y +
                                persistent->floating_rect.offset_y));
                        continue;
                    }
                    u32 leaf_index = find_panel_leaf(state, panel.id);
                    if(leaf_index >= state.nodes.size()) continue;
                    DockTreeNode& leaf = state.nodes[leaf_index];
                    id_t displayed = leaf.selected_tab;
                    DockPanelActionInfo* displayed_panel = find_action_panel(action, displayed);
                    if(!displayed_panel || !displayed_panel->visible)
                    {
                        displayed_panel = &panel;
                    }
                    if(displayed_panel->root.id != panel.root.id) continue;
                    GUICore::LayoutResult result;
                    result.rect = leaf.rect;
                    result.clip_rect = intersect_rect(leaf.rect, child_clip);
                    result.content_size = Float2U(leaf.rect.width, leaf.rect.height);
                    context->set_layout_result(panel.root, result);
                }
                return ok;
            }

            static void draw_rect(GUICore::IContext* context, GUICore::DrawCommandType type,
                const RectF& rect, const Float4U& color, f32 radius = 0.0f,
                GUICore::DrawCommandRectReference reference = GUICore::DrawCommandRectReference::element)
            {
                GUICore::DrawCommand command;
                command.type = type;
                command.rect_reference = reference;
                command.rect = rect;
                command.color = color;
                command.radius = radius;
                context->draw(command);
            }

            static void draw_text(GUICore::IContext* context, const GUICore::ElementHandle& element,
                const RectF& rect, const c8* text, const Float4U& color, f32 size,
                GUICore::DrawCommandRectReference reference)
            {
                GUICore::DrawCommand command;
                command.type = GUICore::DrawCommandType::text;
                command.rect_reference = reference;
                command.rect = rect;
                command.color = color;
                command.font = style_name(context, element, "gui.font");
                command.font_size = size;
                command.horizontal_alignment = VG::TextAlignment::begin;
                command.vertical_alignment = VG::TextAlignment::center;
                command.text = text ? text : "";
                context->draw(command);
            }

            static RectF drop_preview_rect(const RectF& rect, DockDropDirection direction)
            {
                switch(direction)
                {
                case DockDropDirection::left: return RectF(rect.offset_x, rect.offset_y, rect.width * 0.5f, rect.height);
                case DockDropDirection::right: return RectF(rect.offset_x + rect.width * 0.5f, rect.offset_y,
                    rect.width * 0.5f, rect.height);
                case DockDropDirection::up: return RectF(rect.offset_x, rect.offset_y, rect.width, rect.height * 0.5f);
                case DockDropDirection::down: return RectF(rect.offset_x, rect.offset_y + rect.height * 0.5f,
                    rect.width, rect.height * 0.5f);
                case DockDropDirection::center: return rect;
                default: return RectF();
                }
            }

            static RectF dock_drop_target_rect(const RectF& parent, DockDropDirection direction)
            {
                f32 shorter = min(parent.width, parent.height);
                f32 center_size = clamp(shorter * 0.12f, 22.0f, 36.0f);
                f32 side_width = center_size * 1.2f;
                f32 side_height = center_size * 0.78f;
                f32 offset = center_size * 1.95f;
                f32 center_x = parent.offset_x + parent.width * 0.5f;
                f32 center_y = parent.offset_y + parent.height * 0.5f;
                switch(direction)
                {
                case DockDropDirection::center:
                    return RectF(center_x - center_size * 0.5f, center_y - center_size * 0.5f,
                        center_size, center_size);
                case DockDropDirection::left:
                    return RectF(center_x - offset - side_height * 0.5f, center_y - side_width * 0.5f,
                        side_height, side_width);
                case DockDropDirection::right:
                    return RectF(center_x + offset - side_height * 0.5f, center_y - side_width * 0.5f,
                        side_height, side_width);
                case DockDropDirection::up:
                    return RectF(center_x - side_width * 0.5f, center_y - offset - side_height * 0.5f,
                        side_width, side_height);
                case DockDropDirection::down:
                    return RectF(center_x - side_width * 0.5f, center_y + offset - side_height * 0.5f,
                        side_width, side_height);
                default:
                    return RectF();
                }
            }

            static DockDropDirection hit_test_dock_drop_target(const RectF& rect, const Float2U& position,
                bool center_only)
            {
                static const DockDropDirection directions[] = {
                    DockDropDirection::center,
                    DockDropDirection::left,
                    DockDropDirection::right,
                    DockDropDirection::up,
                    DockDropDirection::down
                };
                for(DockDropDirection direction : directions)
                {
                    if(center_only && direction != DockDropDirection::center) continue;
                    if(point_in_rect(dock_drop_target_rect(rect, direction), position)) return direction;
                }
                return DockDropDirection::none;
            }

            static void draw_line(GUICore::IContext* context, const Float2U& begin, const Float2U& end,
                const Float4U& color, f32 width)
            {
                GUICore::DrawCommand command;
                command.type = GUICore::DrawCommandType::line;
                command.rect_reference = GUICore::DrawCommandRectReference::layer;
                command.rect = RectF(begin.x, begin.y, 0.0f, 0.0f);
                command.point1 = end;
                command.color = color;
                command.line_width = width;
                context->draw(command);
            }

            static RV draw_dock_space(GUICore::IContext* context, const GUICore::ElementHandle&,
                GUICore::DrawPhase, void* userdata)
            {
                DockSpaceDrawData* data = (DockSpaceDrawData*)userdata;
                if(!data || !data->action || !data->action->state) return ok;
                DockSpaceAction& action = *data->action;
                DockSpaceState& state = *action.state;
                for(const DockTreeNode& node : state.nodes)
                {
                    if(node.active && node.split && rect_valid(node.splitter_rect))
                    {
                        RectF visual_rect = node.splitter_rect;
                        f32 visual_size = clamp(action.desc.splitter_visual_size, 0.0f,
                            action.desc.splitter_size);
                        if(node.split_axis == DockSplitAxis::x)
                        {
                            visual_rect.offset_x += (visual_rect.width - visual_size) * 0.5f;
                            visual_rect.width = visual_size;
                        }
                        else
                        {
                            visual_rect.offset_y += (visual_rect.height - visual_size) * 0.5f;
                            visual_rect.height = visual_size;
                        }
                        draw_rect(context, GUICore::DrawCommandType::rect, visual_rect,
                            action.desc.splitter_color, 0.0f, GUICore::DrawCommandRectReference::layer);
                    }
                }
                return ok;
            }

            static RV draw_dock_indicators(GUICore::IContext* context, const GUICore::ElementHandle&,
                GUICore::DrawPhase, void* userdata)
            {
                DockSpaceDrawData* data = (DockSpaceDrawData*)userdata;
                if(!data || !data->action || !data->action->state) return ok;
                DockSpaceAction& action = *data->action;
                DockSpaceState& state = *action.state;
                if(state.drag_mode != DockDragMode::floating_move || !state.drop_target_available) return ok;

                RectF target;
                bool empty_dock_space = state.root_node == U32_MAX;
                if(state.drop_target < state.nodes.size()) target = state.nodes[state.drop_target].rect;
                else if(empty_dock_space) target = state.dock_rect;
                if(!rect_valid(target)) return ok;

                if(state.drop_direction != DockDropDirection::none)
                {
                    RectF preview = drop_preview_rect(target, state.drop_direction);
                    Float4U color = action.desc.docking_indicator_color;
                    color.w *= 0.28f;
                    draw_rect(context, GUICore::DrawCommandType::rect, preview, color, 0.0f,
                        GUICore::DrawCommandRectReference::layer);
                }

                static const DockDropDirection directions[] = {
                    DockDropDirection::center,
                    DockDropDirection::left,
                    DockDropDirection::right,
                    DockDropDirection::up,
                    DockDropDirection::down
                };
                for(DockDropDirection direction : directions)
                {
                    if(empty_dock_space && direction != DockDropDirection::center) continue;
                    RectF icon = dock_drop_target_rect(target, direction);
                    bool selected = direction == state.drop_direction;
                    Float4U fill = action.desc.docking_indicator_color;
                    fill.w = selected ? min(fill.w + 0.10f, 1.0f) : fill.w * 0.42f;
                    Float4U stroke = selected ? Float4U(0.74f, 0.87f, 1.0f, 1.0f) :
                        Float4U(0.46f, 0.56f, 0.68f, 0.95f);
                    draw_rect(context, GUICore::DrawCommandType::rounded_rect, icon, fill, 5.0f,
                        GUICore::DrawCommandRectReference::layer);
                    f32 left = icon.offset_x + 5.0f;
                    f32 right = icon.offset_x + max(icon.width - 5.0f, 5.0f);
                    f32 top = icon.offset_y + 5.0f;
                    f32 bottom = icon.offset_y + max(icon.height - 5.0f, 5.0f);
                    f32 center_x = icon.offset_x + icon.width * 0.5f;
                    f32 center_y = icon.offset_y + icon.height * 0.5f;
                    if(direction == DockDropDirection::center)
                    {
                        draw_line(context, Float2U(left, top), Float2U(right, top), stroke, 1.6f);
                        draw_line(context, Float2U(right, top), Float2U(right, bottom), stroke, 1.6f);
                        draw_line(context, Float2U(right, bottom), Float2U(left, bottom), stroke, 1.6f);
                        draw_line(context, Float2U(left, bottom), Float2U(left, top), stroke, 1.6f);
                    }
                    else if(direction == DockDropDirection::left || direction == DockDropDirection::right)
                    {
                        draw_line(context, Float2U(center_x, top), Float2U(center_x, bottom), stroke, 2.0f);
                    }
                    else
                    {
                        draw_line(context, Float2U(left, center_y), Float2U(right, center_y), stroke, 2.0f);
                    }
                }
                return ok;
            }

            static RV draw_dock_panel(GUICore::IContext* context, const GUICore::ElementHandle& element,
                GUICore::DrawPhase, void* userdata)
            {
                DockPanelDrawData* data = (DockPanelDrawData*)userdata;
                if(!data || !data->action || !data->panel || !data->action->state) return ok;
                DockSpaceAction& action = *data->action;
                DockPanelActionInfo& panel = *data->panel;
                DockSpaceState& state = *action.state;
                const DockPanelPersistentData* persistent = find_panel(state, panel.id);
                if(!persistent) return ok;
                const GUICore::Element* panel_element = context->get_element(element.index);
                if(!panel_element) return ok;
                const DockPanelDesc& desc = panel.desc;
                f32 panel_width = panel_element->layout_result.rect.width;
                f32 panel_height = panel_element->layout_result.rect.height;
                Float4U background_color = desc.background_color.w > 0.0f ? desc.background_color :
                    style_color(context, element, "gui.surface.1", Float4U(0.07f, 0.09f, 0.12f, 0.98f));
                Float4U title_bar_color = desc.title_bar_color.w > 0.0f ? desc.title_bar_color :
                    style_color(context, element, "gui.surface.1", Float4U(0.11f, 0.15f, 0.20f, 1.0f));
                Float4U border_color = desc.border_color.w > 0.0f ? desc.border_color :
                    style_color(context, element, "gui.border.strong", Float4U(0.24f, 0.30f, 0.38f, 1.0f));
                bool floating = persistent->mode == DockPanelMode::floating;
                f32 panel_radius = style_scalar(context, element, "gui.radius.medium", 9.0f);
                if(floating)
                {
                    Float4U shadow_color = style_color(context, element, "gui.shadow.dark",
                        Float4U(0.0f, 0.0f, 0.0f, 0.30f));
                    shadow_color.w = min(shadow_color.w * 1.18f, 0.34f);
                    f32 shadow_softness = style_scalar(context, element, "gui.shadow.softness", 4.0f);
                    RoundedRectEffect outer_effects[2];
                    outer_effects[0].shadow = true;
                    outer_effects[0].color = shadow_color;
                    outer_effects[0].shadow_desc.offset = Float2U(0.0f, shadow_softness * 1.6f);
                    outer_effects[0].shadow_desc.softness = shadow_softness * 3.5f;
                    outer_effects[1].color = border_color;
                    if(RV result = draw_rounded_rect_effects(context, element, RectF(), Float4U(), panel_radius,
                        Span<const RoundedRectEffect>(outer_effects, 2)); failed(result))
                    {
                        return result;
                    }

                    RoundedRectEffect inner_effects[2];
                    inner_effects[0].color = background_color;
                    Float4U panel_highlight = style_color(context, element, "gui.shadow.inset_light",
                        Float4U(1.0f, 1.0f, 1.0f, 0.65f));
                    panel_highlight.w *= 0.16f;
                    inner_effects[1].shadow = true;
                    inner_effects[1].color = panel_highlight;
                    inner_effects[1].shadow_desc.offset = Float2U(0.0f, -1.0f);
                    inner_effects[1].shadow_desc.softness = max(shadow_softness * 0.65f, 1.0f);
                    inner_effects[1].shadow_desc.mode = GUICore::ShadowMode::inner;
                    if(RV result = draw_rounded_rect_effects(context, element,
                        RectF(1.0f, 1.0f, -2.0f, -2.0f), Float4U(),
                        max(panel_radius - 1.0f, 0.0f),
                        Span<const RoundedRectEffect>(inner_effects, 2)); failed(result))
                    {
                        return result;
                    }
                }
                else
                {
                    draw_rect(context, GUICore::DrawCommandType::rect, RectF(), background_color);
                }
                if(desc.title_bar)
                {
                    const f32 title_height = desc.title_bar_height;
                    const f32 text_padding = floating ? 12.0f :
                        style_scalar(context, element, "gui.tab.padding_x", 14.0f);
                    const f32 font_size = floating ?
                        style_scalar(context, element, "gui.text.font_size", 16.0f) :
                        style_scalar(context, element, "gui.menu_item.font_size", 13.0f);
                    draw_rect(context, GUICore::DrawCommandType::rect,
                        RectF(floating ? 12.0f : 0.0f, max(title_height - 1.0f, 0.0f),
                        floating ? max(panel_width - 24.0f, 0.0f) : panel_width, 1.0f),
                        style_color(context, element, "gui.border", border_color));
                    if(!floating)
                    {
                        draw_rect(context, GUICore::DrawCommandType::rect,
                            RectF(0.0f, 0.0f, panel_width, max(title_height - 1.0f, 0.0f)), title_bar_color);
                    }

                    auto draw_tab = [&](const RectF& tab_rect, const c8* label, bool selected, bool reserve_close)
                    {
                        if(selected && desc.active_title_bar_color.w > 0.0f)
                        {
                            draw_rect(context, GUICore::DrawCommandType::rect, tab_rect,
                                desc.active_title_bar_color);
                        }
                        f32 close_reserve = reserve_close ? title_height * 0.72f : 0.0f;
                        RectF text_rect(tab_rect.offset_x + text_padding, 0.0f,
                            max(tab_rect.width - text_padding * 2.0f - close_reserve, 0.0f), title_height);
                        draw_text(context, element, text_rect, label, style_color(context, element,
                            selected && !floating ? "gui.focus" : selected ? "gui.text.color" : "gui.text.secondary",
                            selected ? Float4U(0.72f, 0.38f, 0.40f, 1.0f) :
                            Float4U(0.65f, 0.68f, 0.72f, 1.0f)), font_size,
                            GUICore::DrawCommandRectReference::element);
                    };

                    u32 leaf_index = floating ? U32_MAX : find_panel_leaf(state, panel.id);
                    if(leaf_index < state.nodes.size() && !state.nodes[leaf_index].tabs.empty())
                    {
                        DockTreeNode& leaf = state.nodes[leaf_index];
                        f32 width_scale = dock_tab_width_scale(context, element, action, leaf,
                            font_size, panel_width);
                        f32 x = 0.0f;
                        f32 target_indicator_x = 0.0f;
                        f32 target_indicator_width = 0.0f;
                        for(usize i = 0; i < leaf.tabs.size(); ++i)
                        {
                            id_t tab = leaf.tabs[i];
                            const c8* label = panel_label(action, tab);
                            f32 width = dock_tab_width(context, element, label, font_size) * width_scale;
                            RectF tab_rect(x, 0.0f, width, desc.title_bar_height);
                            bool selected = tab == leaf.selected_tab;
                            draw_tab(tab_rect, label, selected,
                                selected && panel.close.id && i + 1 == leaf.tabs.size());
                            if(selected)
                            {
                                f32 indicator_inset = min(10.0f, tab_rect.width * 0.25f);
                                target_indicator_x = tab_rect.offset_x + indicator_inset;
                                target_indicator_width = max(tab_rect.width - indicator_inset * 2.0f, 0.0f);
                            }
                            x += width;
                        }
                        if(!leaf.tab_indicator_initialized)
                        {
                            leaf.tab_indicator_x = target_indicator_x;
                            leaf.tab_indicator_width = target_indicator_width;
                            leaf.tab_indicator_initialized = true;
                        }
                        else
                        {
                            f32 delta_time = max(context->get_frame_desc().delta_time, 0.0f);
                            leaf.tab_indicator_x = smooth_step(leaf.tab_indicator_x,
                                target_indicator_x, 10.0f, delta_time);
                            leaf.tab_indicator_width = smooth_step(leaf.tab_indicator_width,
                                target_indicator_width, 10.0f, delta_time);
                        }
                        draw_rect(context, GUICore::DrawCommandType::rounded_rect,
                            RectF(leaf.tab_indicator_x, max(title_height - 2.0f, 0.0f),
                            leaf.tab_indicator_width, 2.0f), style_color(context, element, "gui.accent",
                            Float4U(0.89f, 0.31f, 0.35f, 1.0f)), 1.0f);
                    }
                    else
                    {
                        draw_tab(RectF(0.0f, 0.0f, panel_width, desc.title_bar_height), panel.label,
                            true, panel.close.id);
                    }
                }
                f32 border = max(desc.border_size, 0.0f);
                if(border > 0.0f && !floating)
                {
                    draw_rect(context, GUICore::DrawCommandType::rect,
                        RectF(0.0f, 0.0f, panel_width, border), border_color);
                    draw_rect(context, GUICore::DrawCommandType::rect,
                        RectF(0.0f, max(panel_height - border, 0.0f), panel_width, border), border_color);
                    draw_rect(context, GUICore::DrawCommandType::rect,
                        RectF(0.0f, 0.0f, border, panel_height), border_color);
                    draw_rect(context, GUICore::DrawCommandType::rect,
                        RectF(max(panel_width - border, 0.0f), 0.0f, border, panel_height), border_color);
                }
                if(panel.close.id)
                {
                    f32 icon_size = clamp(desc.title_bar_height * 0.24f, 8.0f, 12.0f);
                    f32 x = panel_width - desc.title_bar_height +
                        (desc.title_bar_height - icon_size) * 0.5f;
                    f32 y = (desc.title_bar_height - icon_size) * 0.5f;
                    GUICore::DrawCommand line;
                    line.type = GUICore::DrawCommandType::line;
                    line.rect_reference = GUICore::DrawCommandRectReference::element;
                    line.rect = RectF(x, y, 0.0f, 0.0f);
                    line.point1 = Float2U(x + icon_size, y + icon_size);
                    line.color = style_color(context, element,
                        context->get_interaction_state(panel.close.id).hovered ? "gui.accent" : "gui.text.secondary",
                        Float4U(0.72f, 0.74f, 0.76f, 1.0f));
                    line.line_width = 1.5f;
                    context->draw(line);
                    line.rect = RectF(x + icon_size, y, 0.0f, 0.0f);
                    line.point1 = Float2U(x, y + icon_size);
                    context->draw(line);
                }
                if(panel.resize.id && desc.resize_border)
                {
                    GUICore::DrawCommand line;
                    line.type = GUICore::DrawCommandType::line;
                    line.rect_reference = GUICore::DrawCommandRectReference::element;
                    line.rect = RectF(max(panel_width - 13.0f, 0.0f),
                        max(panel_height - 4.0f, 0.0f), 0.0f, 0.0f);
                    line.point1 = Float2U(max(panel_width - 4.0f, 0.0f),
                        max(panel_height - 13.0f, 0.0f));
                    line.color = border_color;
                    line.line_width = 1.5f;
                    context->draw(line);
                }
                return ok;
            }

            static Float2U dock_space_pointer(const DockSpaceAction& action, GUICore::IContext* context,
                const Float2U& screen_position)
            {
                const DockSpaceState& state = *action.state;
                return Float2U(screen_position.x - state.screen_origin.x,
                    screen_position.y - state.screen_origin.y);
            }

            static u32 leaf_at_position(const DockSpaceState& state, const Float2U& position)
            {
                for(u32 i = 0; i < state.nodes.size(); ++i)
                {
                    if(state.nodes[i].active && !state.nodes[i].split && !state.nodes[i].tabs.empty() &&
                        point_in_rect(state.nodes[i].rect, position))
                    {
                        return i;
                    }
                }
                return U32_MAX;
            }

            static bool has_pointer_event(GUICore::IContext* context, id_t id, GUICore::InputEventType type,
                GUICore::InputEvent* out_event = nullptr)
            {
                for(const GUICore::InputEvent& event : context->get_delivered_input_events(id))
                {
                    if(event.type == type && (type != GUICore::InputEventType::pointer_down &&
                        type != GUICore::InputEventType::pointer_up || event.button == GUICore::PointerButton::left))
                    {
                        if(out_event) *out_event = event;
                        return true;
                    }
                }
                return false;
            }

            static void raise_panel(GUICore::IContext* context, DockSpaceState& state, DockPanelActionInfo& panel)
            {
                DockPanelPersistentData* persistent = find_panel(state, panel.id);
                if(!persistent || persistent->mode != DockPanelMode::floating) return;
                persistent->z_order = state.next_z_order++;
                context->bring_layer_to_front(panel.layer_id);
            }

            bool resolve_dock_space_action(GUICore::IContext* context, DockSpaceAction& action)
            {
                if(!context || !action.state) return false;
                DockSpaceState& state = *action.state;
                bool relayout = false;

                for(usize i = 0; i < action.panel_count; ++i)
                {
                    DockPanelActionInfo& panel = action.panels[i];
                    DockPanelPersistentData* persistent = find_panel(state, panel.id);
                    if(!persistent) continue;
                    GUICore::InputEvent event;
                    if(panel.raise.id && has_pointer_event(context, panel.raise.id, GUICore::InputEventType::pointer_down))
                    {
                        raise_panel(context, state, panel);
                    }
                    if(panel.close.id && context->get_interaction_state(panel.close.id).clicked)
                    {
                        if(panel.open) *panel.open = false;
                        remove_panel(state, panel.id);
                        relayout = true;
                        continue;
                    }
                    if(panel.title.id && has_pointer_event(context, panel.title.id,
                        GUICore::InputEventType::pointer_down, &event))
                    {
                        raise_panel(context, state, panel);
                        state.drag_panel = panel.id;
                        state.drag_start_pointer = event.position;
                        state.drag_start_rect = persistent->floating_rect;
                        state.drop_target = U32_MAX;
                        state.drop_target_available = false;
                        state.drop_direction = DockDropDirection::none;
                        state.drag_mode = persistent->mode == DockPanelMode::floating ?
                            DockDragMode::floating_move : DockDragMode::docked_title;

                        if(persistent->mode == DockPanelMode::docking)
                        {
                            u32 leaf_index = find_panel_leaf(state, panel.id);
                            if(leaf_index < state.nodes.size())
                            {
                                DockTreeNode& leaf = state.nodes[leaf_index];
                                const GUICore::Element* panel_element = context->get_element(panel.root.index);
                                f32 panel_width = panel_element ? panel_element->layout_result.rect.width : 0.0f;
                                f32 font_size = style_scalar(context, panel.title,
                                    "gui.menu_item.font_size", 13.0f);
                                f32 width_scale = dock_tab_width_scale(context, panel.title,
                                    action, leaf, font_size, panel_width);
                                for(const GUICore::RoutedInputEvent& routed : context->get_routed_input_events(panel.title.id))
                                {
                                    if(routed.event.type != GUICore::InputEventType::pointer_down || !routed.has_element_position) continue;
                                    f32 x = 0.0f;
                                    for(id_t tab : leaf.tabs)
                                    {
                                        f32 width = dock_tab_width(context, panel.title,
                                            panel_label(action, tab), font_size) * width_scale;
                                        if(routed.element_position.x >= x &&
                                            routed.element_position.x < x + width)
                                        {
                                            leaf.selected_tab = tab;
                                            break;
                                        }
                                        x += width;
                                    }
                                    break;
                                }
                            }
                        }
                    }
                    if(panel.resize.id && has_pointer_event(context, panel.resize.id,
                        GUICore::InputEventType::pointer_down, &event))
                    {
                        raise_panel(context, state, panel);
                        state.drag_panel = panel.id;
                        state.drag_start_pointer = event.position;
                        state.drag_start_rect = persistent->floating_rect;
                        state.drag_mode = DockDragMode::floating_resize;
                    }
                }

                GUICore::InputEvent root_event;
                if(has_pointer_event(context, action.id, GUICore::InputEventType::pointer_down, &root_event))
                {
                    Float2U pointer = dock_space_pointer(action, context, root_event.position);
                    for(u32 i = 0; i < state.nodes.size(); ++i)
                    {
                        if(state.nodes[i].active && state.nodes[i].split &&
                            point_in_rect(state.nodes[i].splitter_rect, pointer))
                        {
                            state.drag_mode = DockDragMode::splitter;
                            state.drag_splitter = i;
                            state.drag_start_pointer = root_event.position;
                            state.drag_start_ratio = state.nodes[i].split_ratio;
                            break;
                        }
                    }
                }

                bool pointer_moved = false;
                GUICore::InputEvent move_event;
                for(const GUICore::InputEvent& event : context->get_input_events())
                {
                    if(event.type == GUICore::InputEventType::pointer_move)
                    {
                        pointer_moved = true;
                        move_event = event;
                    }
                }
                if(pointer_moved && state.drag_mode != DockDragMode::none)
                {
                    Float2U delta(move_event.position.x - state.drag_start_pointer.x,
                        move_event.position.y - state.drag_start_pointer.y);
                    DockPanelPersistentData* persistent = find_panel(state, state.drag_panel);
                    if(state.drag_mode == DockDragMode::docked_title && persistent &&
                        (abs(delta.x) > 5.0f || abs(delta.y) > 5.0f))
                    {
                        remove_panel_from_tree(state, persistent->id);
                        persistent->mode = DockPanelMode::floating;
                        persistent->floating_rect = persistent->restored_floating_rect;
                        persistent->floating_rect.offset_x = move_event.position.x - state.screen_origin.x -
                            state.dock_rect.offset_x -
                            persistent->floating_rect.width * 0.5f;
                        persistent->floating_rect.offset_y = move_event.position.y - state.screen_origin.y -
                            state.dock_rect.offset_y - 12.0f;
                        persistent->z_order = state.next_z_order++;
                        state.drag_start_pointer = move_event.position;
                        state.drag_start_rect = persistent->floating_rect;
                        state.drag_mode = DockDragMode::floating_move;
                        relayout = true;
                    }
                    else if(state.drag_mode == DockDragMode::floating_move && persistent)
                    {
                        persistent->floating_rect.offset_x = state.drag_start_rect.offset_x + delta.x;
                        persistent->floating_rect.offset_y = state.drag_start_rect.offset_y + delta.y;
                        DockPanelActionInfo* panel = find_action_panel(action, persistent->id);
                        if(panel) context->set_layer_screen_position(panel->layer_id,
                            Float2U(state.screen_origin.x + state.dock_rect.offset_x +
                                persistent->floating_rect.offset_x,
                                state.screen_origin.y + state.dock_rect.offset_y +
                                persistent->floating_rect.offset_y));
                        Float2U pointer = dock_space_pointer(action, context, move_event.position);
                        state.drop_target = leaf_at_position(state, pointer);
                        state.drop_target_available = false;
                        if(state.drop_target < state.nodes.size())
                        {
                            state.drop_target_available = true;
                            state.drop_direction = hit_test_dock_drop_target(
                                state.nodes[state.drop_target].rect, pointer, false);
                        }
                        else if(state.root_node == U32_MAX && point_in_rect(state.dock_rect, pointer))
                        {
                            state.drop_target_available = true;
                            state.drop_direction = hit_test_dock_drop_target(state.dock_rect, pointer, true);
                        }
                        else state.drop_direction = DockDropDirection::none;
                    }
                    else if(state.drag_mode == DockDragMode::floating_resize && persistent)
                    {
                        DockPanelActionInfo* panel = find_action_panel(action, persistent->id);
                        Float2U minimum = panel ? panel->desc.minimum_floating_size : Float2U(140.0f, 90.0f);
                        persistent->floating_rect.width = max(state.drag_start_rect.width + delta.x, minimum.x);
                        persistent->floating_rect.height = max(state.drag_start_rect.height + delta.y, minimum.y);
                        persistent->restored_floating_rect = persistent->floating_rect;
                        if(panel && panel->root.id)
                        {
                            lupanic_if_failed(context->apply_layout(panel->root,
                                RectF(0.0f, 0.0f, persistent->floating_rect.width, persistent->floating_rect.height)));
                        }
                    }
                    else if(state.drag_mode == DockDragMode::splitter && state.drag_splitter < state.nodes.size())
                    {
                        DockTreeNode& node = state.nodes[state.drag_splitter];
                        f32 size = node.split_axis == DockSplitAxis::x ? node.rect.width : node.rect.height;
                        f32 movement = node.split_axis == DockSplitAxis::x ? delta.x : delta.y;
                        if(size > action.desc.splitter_size)
                        {
                            node.split_ratio = clamp(state.drag_start_ratio + movement /
                                (size - action.desc.splitter_size), action.desc.minimum_split_ratio,
                                1.0f - action.desc.minimum_split_ratio);
                            relayout = true;
                        }
                    }
                }

                bool pointer_released = false;
                for(const GUICore::InputEvent& event : context->get_input_events())
                {
                    if(event.type == GUICore::InputEventType::pointer_up && event.button == GUICore::PointerButton::left)
                    {
                        pointer_released = true;
                        break;
                    }
                }
                if(pointer_released && state.drag_mode != DockDragMode::none)
                {
                    if(state.drag_mode == DockDragMode::floating_move && state.drag_panel &&
                        state.drop_direction != DockDropDirection::none)
                    {
                        dock_panel(state, state.drag_panel, state.drop_target, state.drop_direction);
                        relayout = true;
                    }
                    state.drag_mode = DockDragMode::none;
                    state.drag_panel = 0;
                    state.drag_splitter = U32_MAX;
                    state.drop_target = U32_MAX;
                    state.drop_target_available = false;
                    state.drop_direction = DockDropDirection::none;
                }
                if(state.drag_mode == DockDragMode::floating_move)
                {
                    context->bring_layer_to_front(action.indicator_layer_id);
                }
                return relayout;
            }
        }

        LUNA_GUI_API GUICore::ElementHandle begin_dock_space(GUICore::IContext* context, id_t id,
            const c8* label, const GUICore::LayoutConfig& layout, const DockSpaceDesc& desc)
        {
            luassert(context && id);
            GUICore::ElementHandle root = Internal::begin_element(context, id,
                label ? label : "Dock Space", layout);
            GUICore::Interactable interactable;
            interactable.pointer_hit_behavior = GUICore::PointerHitBehavior::target;
            set_flags(interactable.flags, GUICore::InteractableFlag::hoverable);
            set_flags(interactable.flags, GUICore::InteractableFlag::activatable);
            context->set_interactable(root, interactable);
            Ref<Internal::DockSpaceState> state = Internal::dock_space_state(context, id);
            Ref<Internal::FrameState> frame = Internal::frame_state(context);
            Internal::DockSpaceBuildScope scope;
            scope.id = id;
            scope.root = root;
            scope.desc = desc;
            scope.state = state.get();
            frame->dock_space_stack.push_back(move(scope));
            return root;
        }

        LUNA_GUI_API void set_dockspace_layout(GUICore::IContext* context, id_t dock_space,
            const DockSpaceLayoutDesc& desc)
        {
            luassert(context && dock_space);
            Ref<Internal::DockSpaceState> state = Internal::dock_space_state(context, dock_space);
            state->nodes.clear();
            state->panels.clear();
            state->root_node = desc.root_node < desc.nodes.size() ? desc.root_node : U32_MAX;
            state->next_z_order = 1;
            state->nodes.reserve(desc.nodes.size());
            for(const DockSpaceLayoutNodeDesc& source : desc.nodes)
            {
                Internal::DockTreeNode node;
                node.split = source.split;
                node.split_axis = source.split_axis;
                node.split_ratio = clamp(source.split_ratio, 0.01f, 0.99f);
                node.child0 = source.child0;
                node.child1 = source.child1;
                node.tabs = source.tabs;
                node.selected_tab = source.selected_tab;
                Internal::normalize_leaf(node);
                state->nodes.push_back(move(node));
            }
            for(u32 i = 0; i < state->nodes.size(); ++i)
            {
                Internal::DockTreeNode& node = state->nodes[i];
                if(!node.split) continue;
                if(node.child0 < state->nodes.size()) state->nodes[node.child0].parent = i;
                if(node.child1 < state->nodes.size()) state->nodes[node.child1].parent = i;
            }
            for(const Internal::DockTreeNode& node : state->nodes)
            {
                if(node.split) continue;
                for(id_t tab : node.tabs)
                {
                    Internal::DockPanelPersistentData& panel = Internal::get_or_add_panel(*state, tab);
                    panel.mode = DockPanelMode::docking;
                }
            }
            for(const DockSpaceFloatingPanelDesc& source : desc.floating_panels)
            {
                if(!source.panel) continue;
                Internal::DockPanelPersistentData& panel = Internal::get_or_add_panel(*state, source.panel);
                Internal::remove_panel_from_tree(*state, source.panel);
                panel.mode = DockPanelMode::floating;
                panel.floating_rect = source.rect;
                panel.restored_floating_rect = source.rect;
                panel.z_order = source.z_order ? source.z_order : state->next_z_order++;
                state->next_z_order = max(state->next_z_order, panel.z_order + 1);
            }
        }

        LUNA_GUI_API bool begin_dock_panel(GUICore::IContext* context, id_t id, const c8* label,
            bool* open, const DockPanelDesc& desc)
        {
            luassert(context && id);
            Ref<Internal::FrameState> frame = Internal::frame_state(context);
            luassert(!frame->dock_space_stack.empty());
            Internal::DockSpaceBuildScope& scope = frame->dock_space_stack.back();
            luassert(scope.open_panel < 0);
            Internal::DockPanelPersistentData& persistent = Internal::get_or_add_panel(*scope.state, id);
            if(persistent.mode == DockPanelMode::docking)
            {
                Internal::add_panel_to_default_leaf(*scope.state, id);
            }

            Internal::DockPanelBuildInfo info;
            info.id = id;
            info.label = label ? label : "Panel";
            info.open = open;
            info.desc = desc;
            if(info.desc.title_bar_height <= 0.0f)
            {
                info.desc.title_bar_height = Internal::style_scalar(context, GUICore::ElementHandle(),
                    "gui.control.height", 32.0f);
            }
            info.submitted = !open || *open;
            if(!info.submitted)
            {
                Internal::remove_panel(*scope.state, id);
                scope.panels.push_back(move(info));
                return false;
            }
            u32 leaf_index = Internal::find_panel_leaf(*scope.state, id);
            bool selected = leaf_index < scope.state->nodes.size() &&
                scope.state->nodes[leaf_index].selected_tab == id;
            info.floating = persistent.mode == DockPanelMode::floating;
            info.visible = info.floating || selected;
            if(!info.visible)
            {
                scope.panels.push_back(move(info));
                return false;
            }

            if(info.floating)
            {
                info.layer_id = GUICore::make_scoped_id(Internal::derived_id(scope.id, "floating.panel"), id);
                context->push_layer(info.layer_id, Float2U(scope.state->screen_origin.x +
                    scope.state->dock_rect.offset_x + persistent.floating_rect.offset_x,
                    scope.state->screen_origin.y + scope.state->dock_rect.offset_y +
                    persistent.floating_rect.offset_y));
                context->set_layer_debug_name(info.layer_id, Name(info.label.c_str()));
            }

            GUICore::LayoutConfig root_layout;
            if(info.floating)
            {
                root_layout.width.kind = GUICore::SizeKind::fixed;
                root_layout.width.value = persistent.floating_rect.width;
                root_layout.height.kind = GUICore::SizeKind::fixed;
                root_layout.height.value = persistent.floating_rect.height;
            }
            else
            {
                root_layout.width.kind = GUICore::SizeKind::percent;
                root_layout.width.value = 1.0f;
                root_layout.height.kind = GUICore::SizeKind::percent;
                root_layout.height.value = 1.0f;
            }
            info.root = Internal::begin_element(context, Internal::derived_id(id, "dock.panel.root"),
                info.label.c_str(), root_layout);

            if(info.desc.title_bar)
            {
                GUICore::LayoutConfig title_layout;
                title_layout.width.kind = GUICore::SizeKind::percent;
                title_layout.width.value = 1.0f;
                title_layout.height.kind = GUICore::SizeKind::fixed;
                title_layout.height.value = info.desc.title_bar_height;
                info.title = Internal::begin_element(context, Internal::derived_id(id, "dock.panel.title"),
                    "Dock Panel Title", title_layout);
                Internal::set_interactable(context, info.title, true);
                context->end_element();
            }

            GUICore::LayoutConfig content_layout;
            content_layout.width.kind = GUICore::SizeKind::percent;
            content_layout.width.value = 1.0f;
            content_layout.height.kind = GUICore::SizeKind::percent;
            content_layout.height.value = 1.0f;
            info.content = begin_v_layout(context, Internal::derived_id(id, "dock.panel.content"),
                "Dock Panel Content", content_layout);
            scope.panels.push_back(move(info));
            scope.open_panel = (i32)scope.panels.size() - 1;
            return true;
        }

        LUNA_GUI_API void end_dock_panel(GUICore::IContext* context)
        {
            luassert(context);
            Ref<Internal::FrameState> frame = Internal::frame_state(context);
            luassert(!frame->dock_space_stack.empty());
            Internal::DockSpaceBuildScope& scope = frame->dock_space_stack.back();
            luassert(scope.open_panel >= 0 && (usize)scope.open_panel < scope.panels.size());
            Internal::DockPanelBuildInfo& panel = scope.panels[(usize)scope.open_panel];
            Internal::DockPanelPersistentData* persistent = Internal::find_panel(*scope.state, panel.id);
            luassert(persistent);

            GUICore::FlexLayoutDesc content_flex;
            content_flex.axis = GUICore::LayoutAxis::y;
            content_flex.cross_alignment = GUICore::FlexAlignment::stretch;
            content_flex.main_axis_gap = 4.0f;
            end_v_layout(context, panel.content, content_flex);

            if(panel.open && panel.desc.close_button && panel.desc.title_bar)
            {
                GUICore::LayoutConfig close_layout;
                close_layout.width.kind = GUICore::SizeKind::fixed;
                close_layout.width.value = panel.desc.title_bar_height;
                close_layout.height.kind = GUICore::SizeKind::fixed;
                close_layout.height.value = panel.desc.title_bar_height;
                panel.close = Internal::begin_element(context, Internal::derived_id(panel.id, "dock.panel.close"),
                    "Dock Panel Close", close_layout);
                Internal::set_interactable(context, panel.close, true);
                context->end_element();
            }
            if(panel.floating && panel.desc.resize_border)
            {
                GUICore::LayoutConfig resize_layout;
                resize_layout.width.kind = GUICore::SizeKind::fixed;
                resize_layout.width.value = panel.desc.resize_border_size * 2.0f;
                resize_layout.height.kind = GUICore::SizeKind::fixed;
                resize_layout.height.value = panel.desc.resize_border_size * 2.0f;
                panel.resize = Internal::begin_element(context, Internal::derived_id(panel.id, "dock.panel.resize"),
                    "Dock Panel Resize", resize_layout);
                Internal::set_interactable(context, panel.resize, true);
                context->end_element();
            }
            if(panel.floating)
            {
                GUICore::LayoutConfig raise_layout;
                raise_layout.width.kind = GUICore::SizeKind::percent;
                raise_layout.width.value = 1.0f;
                raise_layout.height.kind = GUICore::SizeKind::percent;
                raise_layout.height.value = 1.0f;
                panel.raise = Internal::begin_element(context, Internal::derived_id(panel.id, "dock.panel.raise"),
                    "Dock Panel Raise", raise_layout);
                GUICore::Interactable interactable;
                interactable.pointer_hit_behavior = GUICore::PointerHitBehavior::pass_through;
                set_flags(interactable.flags, GUICore::InteractableFlag::hoverable);
                set_flags(interactable.flags, GUICore::InteractableFlag::activatable);
                context->set_interactable(panel.raise, interactable);
                context->end_element();
            }

            GUICore::CanvasLayoutItem* items = Internal::allocate_frame_array<GUICore::CanvasLayoutItem>(context, 5);
            usize item_count = 0;
            auto add_item = [&](const GUICore::ElementHandle& element, const Float2U& anchor_min,
                const Float2U& anchor_max, const Float2U& pivot, const Float4U& offset)
            {
                if(!element.id) return;
                GUICore::CanvasLayoutItem& item = items[item_count++];
                item.element_id = element.id;
                item.anchor_min = anchor_min;
                item.anchor_max = anchor_max;
                item.pivot = pivot;
                item.offset = offset;
            };
            f32 border = max(panel.desc.border_size, 0.0f);
            f32 title_height = panel.desc.title_bar ? panel.desc.title_bar_height : 0.0f;
            add_item(panel.title, Float2U(0.0f), Float2U(1.0f, 0.0f), Float2U(0.0f),
                Float4U(border, border, -border, border + title_height));
            add_item(panel.content, Float2U(0.0f), Float2U(1.0f), Float2U(0.0f),
                Float4U(border, border + title_height, -border, -border));
            add_item(panel.close, Float2U(1.0f, 0.0f), Float2U(1.0f, 0.0f), Float2U(1.0f, 0.0f),
                Float4U(-border, border, 0.0f, 0.0f));
            add_item(panel.resize, Float2U(1.0f), Float2U(1.0f), Float2U(1.0f), Float4U(-border, -border, 0.0f, 0.0f));
            add_item(panel.raise, Float2U(0.0f), Float2U(1.0f), Float2U(0.0f), Float4U(0.0f));
            GUICore::CanvasLayoutDesc* canvas = Internal::allocate_frame<GUICore::CanvasLayoutDesc>(context);
            canvas->items = Span<const GUICore::CanvasLayoutItem>(items, item_count);
            canvas->clip_children = false;
            GUICore::LayoutCallbackConfig callbacks;
            callbacks.algorithm = Name("gui.dock_panel");
            callbacks.callback = GUICore::layout_canvas;
            callbacks.userdata = canvas;
            context->set_layout_callback_config(panel.root, callbacks);
            context->end_element();
            if(panel.floating)
            {
                context->pop_layer();
                lupanic_if_failed(context->apply_layout(panel.root,
                    RectF(0.0f, 0.0f, persistent->floating_rect.width, persistent->floating_rect.height)));
            }
            scope.open_panel = -1;
        }

        LUNA_GUI_API void end_dock_space(GUICore::IContext* context)
        {
            luassert(context);
            Ref<Internal::FrameState> frame = Internal::frame_state(context);
            luassert(!frame->dock_space_stack.empty());
            Internal::DockSpaceBuildScope scope = move(frame->dock_space_stack.back());
            frame->dock_space_stack.pop_back();
            luassert(scope.open_panel < 0);

            for(usize i = scope.state->panels.size(); i > 0; --i)
            {
                id_t id = scope.state->panels[i - 1].id;
                bool submitted = false;
                for(const Internal::DockPanelBuildInfo& panel : scope.panels)
                {
                    if(panel.id == id && panel.submitted)
                    {
                        submitted = true;
                        break;
                    }
                }
                if(!submitted) Internal::remove_panel(*scope.state, id);
            }

            Internal::DockSpaceAction* action = Internal::allocate_frame<Internal::DockSpaceAction>(context);
            action->id = scope.id;
            action->root = scope.root;
            action->desc = scope.desc;
            action->state = scope.state;
            action->indicator_layer_id = Internal::derived_id(scope.id, "dock.indicator.layer");
            action->panel_count = scope.panels.size();
            action->panels = Internal::allocate_frame_array<Internal::DockPanelActionInfo>(context, action->panel_count);
            for(usize i = 0; i < action->panel_count; ++i)
            {
                const Internal::DockPanelBuildInfo& source = scope.panels[i];
                Internal::DockPanelActionInfo& target = action->panels[i];
                target.id = source.id;
                target.label = Internal::copy_frame_string(context, source.label.c_str());
                target.open = source.open;
                target.desc = source.desc;
                target.visible = source.visible;
                target.floating = source.floating;
                target.layer_id = source.layer_id;
                target.root = source.root;
                target.title = source.title;
                target.close = source.close;
                target.resize = source.resize;
                target.raise = source.raise;
            }

            Internal::DockSpaceLayoutData* layout_data = Internal::allocate_frame<Internal::DockSpaceLayoutData>(context);
            layout_data->action = action;
            GUICore::LayoutCallbackConfig layout_callbacks;
            layout_callbacks.algorithm = Name("gui.dock_space");
            layout_callbacks.callback = Internal::layout_dock_space;
            layout_callbacks.userdata = layout_data;
            context->set_layout_callback_config(scope.root, layout_callbacks);

            Internal::DockSpaceDrawData* draw_data = Internal::allocate_frame<Internal::DockSpaceDrawData>(context);
            draw_data->action = action;
            GUICore::DrawConfig dock_draw;
            dock_draw.name = Name("gui.dock_space");
            dock_draw.callback = Internal::draw_dock_space;
            dock_draw.userdata = draw_data;
            dock_draw.phases = GUICore::DrawPhaseFlag::after_children;
            context->set_draw_config(scope.root, dock_draw);

            for(usize i = 0; i < action->panel_count; ++i)
            {
                Internal::DockPanelActionInfo& panel = action->panels[i];
                if(!panel.visible || !panel.root.id) continue;
                Internal::DockPanelDrawData* panel_draw_data = Internal::allocate_frame<Internal::DockPanelDrawData>(context);
                panel_draw_data->action = action;
                panel_draw_data->panel = &panel;
                GUICore::DrawConfig panel_draw;
                panel_draw.name = Name("gui.dock_panel");
                panel_draw.callback = Internal::draw_dock_panel;
                panel_draw.userdata = panel_draw_data;
                context->set_draw_config(panel.root, panel_draw);
            }

            context->end_element();

            Vector<Internal::DockPanelActionInfo*> floating;
            for(usize i = 0; i < action->panel_count; ++i)
            {
                Internal::DockPanelPersistentData* persistent = Internal::find_panel(*scope.state, action->panels[i].id);
                if(persistent && persistent->mode == DockPanelMode::floating && action->panels[i].layer_id)
                {
                    floating.push_back(action->panels + i);
                }
            }
            sort(floating.begin(), floating.end(), [&](const Internal::DockPanelActionInfo* a,
                const Internal::DockPanelActionInfo* b)
            {
                return Internal::find_panel(*scope.state, a->id)->z_order <
                    Internal::find_panel(*scope.state, b->id)->z_order;
            });
            for(Internal::DockPanelActionInfo* panel : floating)
            {
                context->bring_layer_to_front(panel->layer_id);
            }

            context->push_layer(action->indicator_layer_id, scope.state->screen_origin);
            context->set_layer_debug_name(action->indicator_layer_id, Name("Dock Indicators"));
            GUICore::LayoutConfig indicator_layout;
            GUICore::FrameDesc frame_desc = context->get_frame_desc();
            indicator_layout.width.kind = GUICore::SizeKind::fixed;
            indicator_layout.width.value = frame_desc.screen_size.x;
            indicator_layout.height.kind = GUICore::SizeKind::fixed;
            indicator_layout.height.value = frame_desc.screen_size.y;
            GUICore::ElementHandle indicator_root = Internal::begin_element(context,
                Internal::derived_id(scope.id, "dock.indicator.root"), "Dock Indicators", indicator_layout);
            Internal::DockSpaceDrawData* indicator_draw_data =
                Internal::allocate_frame<Internal::DockSpaceDrawData>(context);
            indicator_draw_data->action = action;
            GUICore::DrawConfig indicator_draw;
            indicator_draw.name = Name("gui.dock_indicators");
            indicator_draw.callback = Internal::draw_dock_indicators;
            indicator_draw.userdata = indicator_draw_data;
            context->set_draw_config(indicator_root, indicator_draw);
            context->end_element();
            context->pop_layer();
            Internal::add_action(context, Internal::ActionType::dock_space, scope.id, action);
        }
    }
}
