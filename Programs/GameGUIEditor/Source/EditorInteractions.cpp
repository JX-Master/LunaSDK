/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorInteractions.cpp
* @author JXMaster
* @date 2026/8/28
*/
#include "EditorApp.hpp"
#include <Luna/Window/FileDialog.hpp>

namespace Luna
{
    namespace GameGUIEditor
    {
        namespace Internal
        {
            R<GUI::paint_order_id_t> draw_hierarchy_drop(GUI::IContext* context,
                const GUI::ElementHandle&, GUI::DrawPhase,
                GUI::paint_order_id_t paint_order_id, void* userdata)
            {
                HierarchyDragState* drag = (HierarchyDragState*)userdata;
                if(!drag || !drag->dragging || drag->drop_mode == HierarchyDropMode::none)
                    return paint_order_id;
                const Float4U color(0.96f, 0.34f, 0.44f, 1.0f);
                if(drag->drop_mode == HierarchyDropMode::child)
                {
                    GUI::DrawCommand highlight;
                    highlight.type = GUI::DrawCommandType::rounded_rect;
                    highlight.rect_reference = GUI::DrawCommandRectReference::layer;
                    highlight.rect = drag->feedback_rect;
                    highlight.color = Float4U(color.x, color.y, color.z, 0.22f);
                    highlight.radius = 4.0f;
                    context->draw(highlight, paint_order_id);
                    return paint_order_id;
                }

                f32 y = drag->drop_mode == HierarchyDropMode::before ?
                    drag->feedback_rect.offset_y :
                    drag->feedback_rect.offset_y + drag->feedback_rect.height;
                f32 left = drag->feedback_rect.offset_x + 8.0f +
                    (f32)drag->feedback_depth * 16.0f;
                f32 right = drag->feedback_rect.offset_x + drag->feedback_rect.width - 8.0f;
                GUI::DrawCommand line;
                line.type = GUI::DrawCommandType::line;
                line.rect_reference = GUI::DrawCommandRectReference::layer;
                line.rect = RectF(left, y, 0.0f, 0.0f);
                line.point1 = Float2U(max(right, left), y);
                line.color = color;
                line.line_width = 2.0f;
                context->draw(line, paint_order_id);
                return paint_order_id;
            }

            void EditorApp::apply_inspector_changes(DocumentView& document)
            {
                // A document opened through Explorer may not have a visible Inspector this frame.
                // Never apply uninitialized or stale property-editor buffers to its new snapshot.
                if(document.inspector_revision != document.revision ||
                    document.inspector_node != document.selected_node) return;
                if(!document.snapshot) return;
                const AuthoringNodeRecord* node = find_authoring_node(*document.snapshot,
                    document.selected_node);
                if(!node) return;
                Variant commands(VariantType::array);
                usize changed_field_count = 0;
                String coalesce_key;
                auto mark_changed = [&](const c8* target, const Name& property)
                {
                    ++changed_field_count;
                    if(changed_field_count == 1)
                    {
                        coalesce_key = guid_string(node->id);
                        coalesce_key.append(".");
                        coalesce_key.append(target);
                        coalesce_key.append(".");
                        coalesce_key.append(property.c_str());
                    }
                    else coalesce_key.clear();
                };
                if(Name(document.node_name.c_str()) != node->name)
                {
                    Variant command(VariantType::object);
                    command["kind"] = "set_name";
                    command["node"] = guid_string(node->id).c_str();
                    command["name"] = document.node_name.c_str();
                    commands.push_back(move(command));
                    mark_changed("node", "name");
                }

                Variant attachment(VariantType::object);
                bool attachment_changed = false;
                Guid parent_id;
                usize sibling_index = 0;
                if(find_parent_info(*document.snapshot, node->id, parent_id, sibling_index))
                {
                    const AuthoringNodeRecord* parent = find_authoring_node(*document.snapshot,
                        parent_id);
                    if(parent && sibling_index < parent->children.size() &&
                        parent->children[sibling_index].attachment.type() == VariantType::object)
                    {
                        attachment = parent->children[sibling_index].attachment;
                    }
                }
                for(const PropertyEditor& editor : document.property_editors)
                {
                    auto value = property_value(editor);
                    if(!value.valid())
                    {
                        error_message = explain(value.errcode());
                        continue;
                    }
                    bool changed = editor.desc.editor == EditingPropertyEditor::size ?
                        (editor.size_mode != editor.baseline_size_mode ||
                        (editor.size_mode != 0 && value.get() != editor.baseline)) :
                        value.get() != editor.baseline;
                    if(!changed) continue;
                    mark_changed(editor.target == PropertyTarget::node ? "node" : "attachment",
                        editor.desc.id);
                    if(editor.target == PropertyTarget::attachment)
                    {
                        attachment[editor.desc.id] = value.get();
                        attachment_changed = true;
                        continue;
                    }
                    if(editor.desc.editor == EditingPropertyEditor::size)
                    {
                        Variant remove_fixed(VariantType::object);
                        remove_fixed["kind"] = "remove_property";
                        remove_fixed["node"] = guid_string(node->id).c_str();
                        remove_fixed["property"] = editor.desc.id;
                        commands.push_back(move(remove_fixed));
                        Variant remove_percent(VariantType::object);
                        remove_percent["kind"] = "remove_property";
                        remove_percent["node"] = guid_string(node->id).c_str();
                        remove_percent["property"] = editor.desc.alternate_id;
                        commands.push_back(move(remove_percent));
                        if(editor.size_mode != 0)
                        {
                            Variant set_size(VariantType::object);
                            set_size["kind"] = "set_property";
                            set_size["node"] = guid_string(node->id).c_str();
                            set_size["property"] = editor.size_mode == 1 ?
                                editor.desc.id : editor.desc.alternate_id;
                            set_size["value"] = value.get();
                            commands.push_back(move(set_size));
                        }
                    }
                    else
                    {
                        Variant command(VariantType::object);
                        command["kind"] = "set_property";
                        command["node"] = guid_string(node->id).c_str();
                        command["property"] = editor.desc.id;
                        command["value"] = value.get();
                        commands.push_back(move(command));
                    }
                }
                if(attachment_changed)
                {
                    Variant command(VariantType::object);
                    command["kind"] = "set_attachment";
                    command["node"] = guid_string(node->id).c_str();
                    command["attachment"] = move(attachment);
                    commands.push_back(move(command));
                }
                if(commands.empty()) return;
                Variant params = editing_params(document);
                params["commands"] = move(commands);
                params["label"] = "Edit node properties";
                if(changed_field_count == 1)
                    params["coalesce_key"] = coalesce_key.c_str();
                Variant metadata;
                if(invoke(GameGUIEditor::APPLY_COMMANDS_URL, params, metadata))
                    refresh_snapshot(document);
            }

            void EditorApp::update_hierarchy_drop(DocumentView& document,
                const UIHandles& handles, const Float2U& pointer_position)
            {
                HierarchyDragState& drag = document.hierarchy_drag;
                drag.drop_mode = HierarchyDropMode::none;
                drag.target_node = Guid();
                drag.target_parent = Guid();
                drag.target_index = 0;
                if(!document.snapshot || !drag.dragging) return;

                auto can_insert_at = [&](const Guid& parent, usize target_index)
                {
                    if(drag.source_type != Guid()) return true;
                    if(subtree_contains(*document.snapshot, drag.source, parent)) return false;
                    Guid old_parent;
                    usize old_index = 0;
                    if(!find_parent_info(*document.snapshot, drag.source, old_parent,
                        old_index)) return false;
                    if(old_parent == parent && old_index < target_index) --target_index;
                    return old_parent != parent || old_index != target_index;
                };
                auto set_reorder_target = [&](const NodeHit& hit,
                    HierarchyDropMode mode, const RectF& feedback_rect)
                {
                    if(hit.parent == Guid()) return false;
                    usize target_index = hit.sibling_index +
                        (mode == HierarchyDropMode::after ? 1 : 0);
                    if(!can_insert_at(hit.parent, target_index)) return false;
                    drag.drop_mode = mode;
                    drag.target_node = hit.node;
                    drag.target_parent = hit.parent;
                    drag.target_index = target_index;
                    drag.feedback_rect = feedback_rect;
                    drag.feedback_depth = hit.depth;
                    return true;
                };

                for(const NodeHit& hit : handles.nodes)
                {
                    RectF screen_rect = item_screen_rect(gui, hit.element);
                    RectF screen_clip = item_screen_rect(gui, hit.element, true);
                    if(!point_in_rect(screen_rect, pointer_position) ||
                        !point_in_rect(screen_clip, pointer_position)) continue;

                    HierarchyDropMode mode = HierarchyDropMode::child;
                    if(hit.node != document.snapshot->root)
                    {
                        f32 local_y = pointer_position.y - screen_rect.offset_y;
                        f32 edge_size = min(6.0f, screen_rect.height * 0.25f);
                        if(local_y <= edge_size) mode = HierarchyDropMode::before;
                        else if(local_y >= screen_rect.height - edge_size)
                            mode = HierarchyDropMode::after;
                    }

                    if(mode != HierarchyDropMode::child)
                    {
                        set_reorder_target(hit, mode,
                            EditorGUI::get_item_rect(gui, hit.element));
                        return;
                    }
                    const AuthoringNodeRecord* target = find_authoring_node(
                        *document.snapshot, hit.node);
                    if(!target) return;
                    usize target_index = target->children.size();
                    if(!can_insert_at(hit.node, target_index)) return;

                    drag.drop_mode = mode;
                    drag.target_node = hit.node;
                    drag.target_parent = hit.node;
                    drag.target_index = target_index;
                    drag.feedback_rect = EditorGUI::get_item_rect(gui, hit.element);
                    drag.feedback_depth = hit.depth;
                    return;
                }

                for(usize i = 1; i < handles.nodes.size(); ++i)
                {
                    const NodeHit& previous = handles.nodes[i - 1];
                    const NodeHit& next = handles.nodes[i];
                    RectF previous_rect = item_screen_rect(gui, previous.element);
                    RectF next_rect = item_screen_rect(gui, next.element);
                    RectF next_clip = item_screen_rect(gui, next.element, true);
                    f32 gap_top = previous_rect.offset_y + previous_rect.height;
                    f32 gap_bottom = next_rect.offset_y;
                    if(gap_bottom < gap_top || pointer_position.y < gap_top ||
                        pointer_position.y > gap_bottom ||
                        pointer_position.x < next_rect.offset_x ||
                        pointer_position.x >= next_rect.offset_x + next_rect.width ||
                        !point_in_rect(next_clip, pointer_position)) continue;
                    set_reorder_target(next, HierarchyDropMode::before,
                        EditorGUI::get_item_rect(gui, next.element));
                    return;
                }
            }

            bool EditorApp::apply_hierarchy_drop(DocumentView& document)
            {
                HierarchyDragState& drag = document.hierarchy_drag;
                if(!document.snapshot || drag.drop_mode == HierarchyDropMode::none)
                    return false;
                if(drag.source_type != Guid())
                {
                    Guid type = drag.source_type;
                    Guid parent = drag.target_parent;
                    usize index = drag.target_index;
                    drag = HierarchyDragState();
                    add_node(document, type, parent, index);
                    return true;
                }
                Guid old_parent;
                usize old_index = 0;
                if(!find_parent_info(*document.snapshot, drag.source, old_parent, old_index))
                    return false;
                usize target_index = drag.target_index;
                if(old_parent == drag.target_parent && old_index < target_index)
                    --target_index;
                if(old_parent == drag.target_parent && old_index == target_index)
                    return false;

                Guid source = drag.source;
                Guid target_parent = drag.target_parent;
                Variant command(VariantType::object);
                command["kind"] = "move_node";
                command["node"] = guid_string(source).c_str();
                command["parent"] = guid_string(target_parent).c_str();
                command["index"] = (u64)target_index;
                if(old_parent != target_parent)
                {
                    command["slot"] = "";
                    command["attachment"] = Variant();
                }
                Variant params = editing_params(document);
                params["commands"] = Variant(VariantType::array);
                params["commands"].push_back(move(command));
                params["label"] = old_parent == target_parent ? "Reorder node" :
                    "Reparent node";
                drag = HierarchyDragState();
                Variant result;
                if(invoke(GameGUIEditor::APPLY_COMMANDS_URL, params, result))
                {
                    document.selected_node = source;
                    document.hierarchy_expand_node = target_parent;
                    refresh_snapshot(document);
                }
                return true;
            }

            void EditorApp::add_node(DocumentView& document, const Guid& type,
                Guid parent, usize index)
            {
                Variant command(VariantType::object);
                command["kind"] = "insert_node";
                command["parent"] = guid_string(parent).c_str();
                command["type"] = guid_string(type).c_str();
                if(index != USIZE_MAX) command["index"] = (u64)index;
                Variant params = editing_params(document);
                params["commands"] = Variant(VariantType::array);
                params["commands"].push_back(move(command));
                params["label"] = "Add node";
                Variant result;
                if(invoke(GameGUIEditor::APPLY_COMMANDS_URL, params, result))
                {
                    if(!result["created_nodes"].empty())
                        decode_guid_string(result["created_nodes"][0], document.selected_node);
                    document.hierarchy_expand_node = parent;
                    refresh_snapshot(document);
                }
            }

            bool EditorApp::process_hierarchy_interactions(DocumentView& document,
                const UIHandles& handles)
            {
                if(!document.snapshot) return false;
                HierarchyDragState& drag = document.hierarchy_drag;
                bool pressed_this_frame = false;
                for(const TypeHit& hit : handles.types)
                {
                    for(const GUI::RoutedInputEvent& routed :
                        gui->get_routed_input_events(hit.element.id))
                    {
                        const GUI::InputEvent& event = routed.event;
                        if(event.type == GUI::InputEventType::pointer_down &&
                            event.button == GUI::PointerButton::left)
                        {
                            drag = HierarchyDragState();
                            drag.source_type = hit.type;
                            drag.source_element = hit.element.id;
                            drag.press_position = event.position;
                            drag.pressed = true;
                            pressed_this_frame = true;
                        }
                    }
                }
                for(const NodeHit& hit : handles.nodes)
                {
                    if(EditorGUI::is_item_right_clicked(gui, hit.element))
                    {
                        document.selected_node = hit.node;
                        document.inspector_revision = 0;
                        drag = HierarchyDragState();
                        GUI::id_t popup_id = hierarchy_context_popup_id(gui, document.id);
                        if(hit.node != document.snapshot->root)
                        {
                            document.hierarchy_context_node = hit.node;
                            document.hierarchy_context_position = gui->get_pointer_position();
                            EditorGUI::open_popup(gui, popup_id);
                        }
                        else
                        {
                            document.hierarchy_context_node = Guid();
                            EditorGUI::close_popup(gui, popup_id);
                        }
                        break;
                    }
                    for(const GUI::RoutedInputEvent& routed :
                        gui->get_routed_input_events(hit.element.id))
                    {
                        const GUI::InputEvent& event = routed.event;
                        if(event.type == GUI::InputEventType::pointer_down &&
                            event.button == GUI::PointerButton::left)
                        {
                            document.selected_node = hit.node;
                            document.inspector_revision = 0;
                            drag = HierarchyDragState();
                            if(hit.node != document.snapshot->root)
                            {
                                drag.source = hit.node;
                                drag.source_element = hit.element.id;
                                drag.press_position = event.position;
                                drag.pressed = true;
                                pressed_this_frame = true;
                            }
                        }
                    }
                }
                if(!drag.pressed) return false;
                // Preserve excursions outside the click threshold even if the pointer returns
                // to its starting position within the same input batch.
                bool tracking_motion = !pressed_this_frame;
                for(const GUI::RoutedInputEvent& routed :
                    gui->get_routed_input_events(drag.source_element))
                {
                    const GUI::InputEvent& event = routed.event;
                    if(event.type == GUI::InputEventType::pointer_down &&
                        event.button == GUI::PointerButton::left) tracking_motion = true;
                    if(tracking_motion && (event.type == GUI::InputEventType::pointer_move ||
                        event.type == GUI::InputEventType::pointer_up))
                    {
                        f32 delta_x = event.position.x - drag.press_position.x;
                        f32 delta_y = event.position.y - drag.press_position.y;
                        if(delta_x * delta_x + delta_y * delta_y >= 16.0f) drag.dragging = true;
                    }
                    if(event.type == GUI::InputEventType::pointer_up &&
                        event.button == GUI::PointerButton::left) tracking_motion = false;
                }
                bool released = false;
                Float2U pointer_position = gui->get_pointer_position();
                for(const GUI::InputEvent& event : gui->get_input_events())
                {
                    if(event.type == GUI::InputEventType::blur)
                    {
                        drag = HierarchyDragState();
                        return true;
                    }
                    if((event.type == GUI::InputEventType::key_down && event.key == KeyCode::esc) ||
                        event.type == GUI::InputEventType::navigation_back)
                    {
                        drag.cancelled = true;
                        drag.drop_mode = HierarchyDropMode::none;
                    }
                    if(event.type == GUI::InputEventType::pointer_up &&
                        event.button == GUI::PointerButton::left)
                    {
                        released = true;
                        pointer_position = event.position;
                    }
                }
                f32 delta_x = pointer_position.x - drag.press_position.x;
                f32 delta_y = pointer_position.y - drag.press_position.y;
                if(!drag.dragging && delta_x * delta_x + delta_y * delta_y >= 16.0f)
                    drag.dragging = true;
                if(drag.dragging && !drag.cancelled)
                    update_hierarchy_drop(document, handles, pointer_position);
                if(gui->is_pointer_button_down(GUI::PointerButton::left)) return false;
                if(drag.dragging || drag.cancelled)
                {
                    if(released && !drag.cancelled) apply_hierarchy_drop(document);
                    drag = HierarchyDragState();
                    // A drag released over its palette button must not also insert via click.
                    return true;
                }
                else
                {
                    drag = HierarchyDragState();
                }
                return false;
            }

            void EditorApp::process_interactions(const UIHandles& handles)
            {
                if(directory_dialog_active || !deferred_directory_closes.empty()) return;
                if(process_explorer_interactions(handles)) return;
                for(usize i = 0; i < documents.size(); ++i)
                {
                    if(documents[i].panel_open) continue;
                    selected_document = (i32)i;
                    apply_inspector_changes(documents[i]);
                    request_close(documents[i], false);
                    return;
                }

                if(EditorGUI::is_item_clicked(gui, handles.create_document)) create_document();
                if(EditorGUI::is_item_clicked(gui, handles.open_document)) open_document();

                DocumentView* document = find_document(handles.document_id);
                if(!document) document = active_document();
                for(DocumentView& view : documents)
                {
                    if(&view != document) view.hierarchy_drag = HierarchyDragState();
                }
                if(document)
                {
                    if(process_visual_effect_actions(*document, handles)) return;
                    apply_inspector_changes(*document);
                    for(const PropertyActionHit& hit : handles.browse_assets)
                    {
                        if(!EditorGUI::is_item_clicked(gui, hit.element) ||
                            hit.property_index >= document->property_editors.size()) continue;
                        PropertyEditor& property =
                            document->property_editors[hit.property_index];
                        asset_picker_document = document->id;
                        asset_picker_property = hit.property_index;
                        asset_picker_revision = document->revision;
                        RectF rect = item_screen_rect(gui, hit.element);
                        asset_picker_position = Float2U(rect.offset_x - 280.0f, rect.offset_y + rect.height);
                        EditorGUI::open_popup(gui, gui->make_id("inspector.asset_picker"));
                        return;
                    }
                    for(const auto& choice : handles.asset_choices)
                    {
                        if(!EditorGUI::is_item_clicked(gui, choice.element)) continue;
                        if(asset_picker_document == document->id && asset_picker_revision == document->revision &&
                            asset_picker_property < document->property_editors.size())
                        {
                            document->property_editors[asset_picker_property].text = guid_string(choice.asset);
                            apply_inspector_changes(*document);
                        }
                        EditorGUI::close_popup(gui, gui->make_id("inspector.asset_picker"));
                        return;
                    }
                    if(process_hierarchy_interactions(*document, handles)) return;
                    for(const NodeHit& hit : handles.nodes)
                    {
                        if(EditorGUI::is_item_clicked(gui, hit.element))
                        {
                            document->selected_node = hit.node;
                            document->inspector_revision = 0;
                            break;
                        }
                    }

                    if(EditorGUI::is_item_clicked(gui, handles.save_document)) save(*document, false);
                    if(EditorGUI::is_item_clicked(gui, handles.save_as_document)) save(*document, true);
                    if(EditorGUI::is_item_clicked(gui, handles.cook_document)) cook(*document);
                    if(EditorGUI::is_item_clicked(gui, handles.undo)) undo_document(*document);
                    if(EditorGUI::is_item_clicked(gui, handles.redo)) redo_document(*document);
                    if(EditorGUI::is_item_clicked(gui, handles.close_document))
                    {
                        request_close(*document, false);
                        return;
                    }

                    if(EditorGUI::is_item_clicked(gui, handles.set_root_node) &&
                        document->snapshot && document->hierarchy_context_node != Guid() &&
                        document->hierarchy_context_node != document->snapshot->root)
                    {
                        Guid node = document->hierarchy_context_node;
                        EditorGUI::close_popup(gui,
                            hierarchy_context_popup_id(gui, document->id));
                        document->hierarchy_context_node = Guid();
                        Variant command(VariantType::object);
                        command["kind"] = "set_root";
                        command["node"] = guid_string(node).c_str();
                        Variant params = editing_params(*document);
                        params["commands"] = Variant(VariantType::array);
                        params["commands"].push_back(move(command));
                        params["label"] = "Set root node";
                        Variant result;
                        if(invoke(GameGUIEditor::APPLY_COMMANDS_URL, params, result))
                        {
                            document->selected_node = node;
                            refresh_snapshot(*document);
                        }
                        return;
                    }

                    if(EditorGUI::is_item_clicked(gui, handles.delete_node) &&
                        document->snapshot && document->hierarchy_context_node != Guid() &&
                        document->hierarchy_context_node != document->snapshot->root)
                    {
                        Guid node = document->hierarchy_context_node;
                        Guid parent;
                        usize sibling_index = 0;
                        find_parent_info(*document->snapshot, node, parent, sibling_index);
                        EditorGUI::close_popup(gui,
                            hierarchy_context_popup_id(gui, document->id));
                        document->hierarchy_context_node = Guid();
                        Variant command(VariantType::object);
                        command["kind"] = "remove_node";
                        command["node"] = guid_string(node).c_str();
                        Variant params = editing_params(*document);
                        params["commands"] = Variant(VariantType::array);
                        params["commands"].push_back(move(command));
                        params["label"] = "Delete node";
                        Variant result;
                        if(invoke(GameGUIEditor::APPLY_COMMANDS_URL, params, result))
                        {
                            document->selected_node = parent;
                            refresh_snapshot(*document);
                        }
                        return;
                    }

                    for(const TypeHit& hit : handles.types)
                    {
                        if(!EditorGUI::is_item_clicked(gui, hit.element)) continue;
                        add_node(*document, hit.type, document->selected_node);
                        break;
                    }

                }

            }
        }
    }
}
