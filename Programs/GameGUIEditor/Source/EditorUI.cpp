/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorUI.cpp
* @author JXMaster
* @date 2026/8/28
*/
#include "EditorApp.hpp"
#include "InspectorLayout.hpp"
#include <cstring>

namespace Luna
{
    namespace GameGUIEditor
    {
        namespace Internal
        {
            static EditorGUI::IconName node_type_icon(const NodeTypeView& type)
            {
                if(!strcmp(type.name.c_str(), "Flex")) return EditorGUI::IconName::rows;
                if(!strcmp(type.name.c_str(), "Canvas")) return EditorGUI::IconName::frame_corners;
                if(!strcmp(type.name.c_str(), "Panel")) return EditorGUI::IconName::squares_four;
                if(!strcmp(type.name.c_str(), "Text")) return EditorGUI::IconName::cursor_text;
                if(!strcmp(type.name.c_str(), "Button")) return EditorGUI::IconName::cursor_click;
                if(!strcmp(type.name.c_str(), "AssetInstance")) return EditorGUI::IconName::package;
                if(!strcmp(type.category.c_str(), "Layout")) return EditorGUI::IconName::grid_four;
                if(!strcmp(type.category.c_str(), "Visual")) return EditorGUI::IconName::eye;
                if(!strcmp(type.category.c_str(), "Input")) return EditorGUI::IconName::hand_tap;
                if(!strcmp(type.category.c_str(), "Composition")) return EditorGUI::IconName::stack;
                return EditorGUI::IconName::plus;
            }

            static bool parent_flex_stretches_size(const DocumentView& document,
                const PropertyEditor& property)
            {
                if(!document.snapshot || property.target != PropertyTarget::node ||
                    property.desc.editor != EditingPropertyEditor::size || property.size_mode == 0)
                    return false;
                bool width = property.desc.id == Name("width");
                bool height = property.desc.id == Name("height");
                if(!width && !height) return false;
                Guid parent_id;
                usize sibling_index = 0;
                if(!find_parent_info(*document.snapshot, document.selected_node,
                    parent_id, sibling_index)) return false;
                const AuthoringNodeRecord* parent = find_authoring_node(*document.snapshot,
                    parent_id);
                return parent && flex_stretches_child_size(*parent, property.desc.id);
            }

            void EditorApp::build_hierarchy_node(DocumentView& document, const Guid& node_id,
                const Guid& parent, usize sibling_index, u32 depth, UIHandles& handles)
            {
                const AuthoringNodeRecord* node = find_authoring_node(*document.snapshot, node_id);
                if(!node) return;
                String label;
                const c8* name = node->name.empty() ? "Unnamed Node" : node->name.c_str();
                strprintf(label, "%s", name);
                EditorGUI::TreeNodeFlag flags = EditorGUI::TreeNodeFlag::open_on_arrow;
                if(node->children.empty()) flags |= EditorGUI::TreeNodeFlag::leaf;
                if(document.selected_node == node_id) flags |= EditorGUI::TreeNodeFlag::selected;
                GUI::ElementHandle item;
                bool open = EditorGUI::tree_node(gui,
                    guid_gui_id(gui->make_id("hierarchy.nodes"), node_id), label.c_str(),
                    flags, depth, fill_width(26.0f), EditorGUI::DisclosureDesc(), &item);
                GUI::Interactable interactable;
                interactable.pointer_hit_behavior = GUI::PointerHitBehavior::target;
                set_flags(interactable.flags, GUI::InteractableFlag::hoverable);
                set_flags(interactable.flags, GUI::InteractableFlag::activatable);
                set_flags(interactable.flags, GUI::InteractableFlag::focusable);
                gui->set_interactable(item, interactable);
                handles.nodes.push_back(NodeHit{node_id, parent, sibling_index, depth, item});
                if(open)
                {
                    for(usize i = 0; i < node->children.size(); ++i)
                        build_hierarchy_node(document, node->children[i].child, node_id, i,
                            depth + 1, handles);
                }
            }

            void EditorApp::build_hierarchy_panel(UIHandles& handles)
            {
                if(!EditorGUI::begin_dock_panel(gui, gui->make_id("panel.hierarchy"),
                    "Hierarchy")) return;
                GUI::ElementHandle root = EditorGUI::begin_v_layout(gui,
                    gui->make_id("hierarchy.root"), "Hierarchy Root", fill_layout());
                EditorGUI::text(gui, gui->make_id("hierarchy.title"), "Node Tree",
                    fill_width(30.0f));
                EditorGUI::ScrollViewDesc scroll_desc;
                scroll_desc.horizontal = false;
                EditorGUI::begin_scroll_view(gui, gui->make_id("hierarchy.scroll"),
                    "Hierarchy Scroll", fill_layout(), scroll_desc);
                DocumentView* document = active_document();
                if(document && document->snapshot)
                    build_hierarchy_node(*document, document->snapshot->root, Guid(), 0, 0,
                        handles);
                EditorGUI::end_scroll_view(gui);
                GUI::FlexLayoutDesc root_layout;
                root_layout.main_axis_gap = 4.0f;
                if(document)
                {
                    GUI::DrawConfig hierarchy_draw;
                    hierarchy_draw.name = Name("game_gui_editor.hierarchy.drop");
                    hierarchy_draw.callback = draw_hierarchy_drop;
                    hierarchy_draw.userdata = &document->hierarchy_drag;
                    hierarchy_draw.phases = GUI::DrawPhaseFlag::after_children;
                    gui->set_draw_config(root, hierarchy_draw);
                }
                EditorGUI::end_v_layout(gui, root, root_layout);
                if(document)
                {
                    GUI::id_t popup_id = hierarchy_context_popup_id(gui, document->id);
                    EditorGUI::PopupDesc popup_desc;
                    popup_desc.position = document->hierarchy_context_position;
                    popup_desc.layout = fixed_layout(180.0f, 74.0f);
                    GUI::ElementHandle popup;
                    if(EditorGUI::begin_popup(gui, popup_id, popup_desc, &popup))
                    {
                        handles.set_root_node = EditorGUI::menu_item(gui,
                            GUI::make_scoped_id(popup_id, "set_root"), "Set as Root");
                        handles.delete_node = EditorGUI::menu_item(gui,
                            GUI::make_scoped_id(popup_id, "delete"), "Delete Node");
                        lupanic_if_failed(EditorGUI::end_popup(gui, popup,
                            RectF(0.0f, 0.0f, 180.0f, 74.0f)));
                    }
                    else if(!EditorGUI::is_popup_open(gui, popup_id))
                    {
                        document->hierarchy_context_node = Guid();
                    }
                }
                EditorGUI::end_dock_panel(gui);
            }

            void EditorApp::build_palette_panel(UIHandles& handles)
            {
                if(!EditorGUI::begin_dock_panel(gui, gui->make_id("panel.palette"),
                    "Node Palette")) return;
                EditorGUI::ScrollViewDesc scroll_desc;
                scroll_desc.horizontal = true;
                scroll_desc.vertical = false;
                EditorGUI::begin_scroll_view(gui, gui->make_id("palette.scroll"),
                    "Palette Scroll", fill_layout(), scroll_desc);
                GUI::LayoutConfig row_config;
                row_config.width.kind = GUI::SizeKind::fit;
                row_config.height.kind = GUI::SizeKind::fixed;
                row_config.height.value = 36.0f;
                row_config.flex_shrink = 0.0f;
                GUI::ElementHandle row = EditorGUI::begin_h_layout(gui,
                    gui->make_id("palette.row"), "Palette Row", row_config);
                GUI::id_t scope = gui->make_id("palette.types");
                for(const NodeTypeView& type : node_types)
                {
                    GUI::id_t item_id = guid_gui_id(scope, type.type);
                    GUI::LayoutConfig button_layout = fixed_layout(36.0f, 36.0f);
                    button_layout.padding = Float4U(0.01f);
                    GUI::ElementHandle item = EditorGUI::begin_button(gui, item_id,
                        type.display_name.c_str(), button_layout);
                    EditorGUI::IconDesc icon_desc;
                    icon_desc.size = 20.0f;
                    EditorGUI::icon(gui, GUI::make_scoped_id(item_id, "icon"),
                        node_type_icon(type), fixed_layout(20.0f, 20.0f), icon_desc);
                    EditorGUI::end_button(gui);
                    String tooltip;
                    strprintf(tooltip, "%s / %s", type.category.c_str(), type.display_name.c_str());
                    EditorGUI::set_item_tooltip(gui,
                        guid_gui_id(gui->make_id("palette.tooltips"), type.type), item,
                        tooltip.c_str());
                    handles.types.push_back(TypeHit{type.type, item});
                }
                GUI::FlexLayoutDesc row_flex;
                row_flex.axis = GUI::LayoutAxis::x;
                row_flex.cross_alignment = GUI::FlexAlignment::center;
                row_flex.main_axis_gap = 8.0f;
                EditorGUI::end_h_layout(gui, row, row_flex);
                EditorGUI::end_scroll_view(gui);
                EditorGUI::end_dock_panel(gui);
            }

            void EditorApp::build_document_panels(UIHandles& handles)
            {
                for(usize i = 0; i < documents.size(); ++i)
                {
                    DocumentView& document = documents[i];
                    String label = document.title;
                    if(document.dirty) label.append(" *");
                    if(!EditorGUI::begin_dock_panel(gui, document_panel_id(gui, document.id),
                        label.c_str(), &document.panel_open)) continue;
                    selected_document = (i32)i;
                    handles.document_id = document.id;
                    GUI::id_t content_scope = GUI::make_scoped_id(gui->make_id("documents.content"),
                        document.id);
                    GUI::ElementHandle root = EditorGUI::begin_v_layout(gui, content_scope,
                        "Document Preview", fill_layout());
                    RV preview_result = ensure_preview(document);
                    if(failed(preview_result)) error_message = explain(preview_result.errcode());
                    handles.preview_host = EditorGUI::begin_v_layout(gui,
                        GUI::make_scoped_id(content_scope, "preview.host"),
                        "Interactive GameGUI Preview", fill_layout());
                    GUI::Interactable preview_interactable;
                    preview_interactable.pointer_hit_behavior = GUI::PointerHitBehavior::target;
                    set_flags(preview_interactable.flags, GUI::InteractableFlag::hoverable);
                    set_flags(preview_interactable.flags, GUI::InteractableFlag::activatable);
                    gui->set_interactable(handles.preview_host, preview_interactable);
                    EditorGUI::ImageDesc image;
                    image.flags = EditorGUI::ImageFlag::flip_y;
                    EditorGUI::image(gui, GUI::make_scoped_id(content_scope, "preview.image"),
                        document.preview.target, fill_layout(), image);
                    GUI::FlexLayoutDesc preview_layout;
                    preview_layout.clip_children = true;
                    EditorGUI::end_v_layout(gui, handles.preview_host, preview_layout);
                    EditorGUI::end_v_layout(gui, root);
                    EditorGUI::end_dock_panel(gui);
                }
            }

#if !defined(LUNA_PLATFORM_MACOS)
            void EditorApp::build_main_menu_bar(UIHandles& handles)
            {
                DocumentView* document = active_document();
                GUI::ElementHandle menu_bar = EditorGUI::begin_menu_bar(gui,
                    gui->make_id("main_menu_bar"), "Main Menu Bar", fill_width(34.0f));
                if(EditorGUI::begin_menu(gui, gui->make_id("main_menu.file"), "File"))
                {
                    handles.create_document = EditorGUI::menu_item(gui,
                        gui->make_id("main_menu.file.new"), "New");
                    EditorGUI::menu_separator(gui, gui->make_id("main_menu.file.new_separator"));

                    handles.open_document = EditorGUI::menu_item(gui,
                        gui->make_id("main_menu.file.open"), "Open...");
                    EditorGUI::menu_separator(gui, gui->make_id("main_menu.file.save_separator"));

                    EditorGUI::MenuItemDesc document_desc;
                    document_desc.enabled = document != nullptr;
                    handles.save_document = EditorGUI::menu_item(gui,
                        gui->make_id("main_menu.file.save"), "Save", false, document_desc);

                    EditorGUI::MenuItemDesc save_as_desc;
                    save_as_desc.enabled = document != nullptr;
                    handles.save_as_document = EditorGUI::menu_item(gui,
                        gui->make_id("main_menu.file.save_as"), "Save As...", false, save_as_desc);
                    EditorGUI::MenuItemDesc cook_desc;
                    cook_desc.enabled = document && !document->asset_path.empty();
                    handles.cook_document = EditorGUI::menu_item(gui,
                        gui->make_id("main_menu.file.cook"), "Cook", false, cook_desc);
                    EditorGUI::menu_separator(gui, gui->make_id("main_menu.file.close_separator"));
                    EditorGUI::MenuItemDesc close_desc;
                    close_desc.enabled = document != nullptr;
                    handles.close_document = EditorGUI::menu_item(gui,
                        gui->make_id("main_menu.file.close"), "Close", false, close_desc);
                    lupanic_if_failed(EditorGUI::end_menu(gui,
                        RectF(0.0f, 0.0f, 230.0f, 210.0f)));
                }

                if(EditorGUI::begin_menu(gui, gui->make_id("main_menu.edit"), "Edit"))
                {
                    EditorGUI::MenuItemDesc undo_desc;
                    undo_desc.enabled = document && document->can_undo;
                    handles.undo = EditorGUI::menu_item(gui,
                        gui->make_id("main_menu.edit.undo"), "Undo", false, undo_desc);
                    EditorGUI::MenuItemDesc redo_desc;
                    redo_desc.enabled = document && document->can_redo;
                    handles.redo = EditorGUI::menu_item(gui,
                        gui->make_id("main_menu.edit.redo"), "Redo", false, redo_desc);
                    lupanic_if_failed(EditorGUI::end_menu(gui,
                        RectF(0.0f, 0.0f, 230.0f, 70.0f)));
                }
                EditorGUI::end_menu_bar(gui, menu_bar);
            }
#endif

            void EditorApp::build_inspector_panel(UIHandles& handles)
            {
                if(!EditorGUI::begin_dock_panel(gui, gui->make_id("panel.inspector"),
                    "Inspector")) return;
                GUI::ElementHandle root = EditorGUI::begin_v_layout(gui,
                    gui->make_id("inspector.root"), "Inspector Root", fill_layout());
                DocumentView* document = active_document();
                if(!document || !document->snapshot)
                {
                    EditorGUI::text(gui, gui->make_id("inspector.empty"),
                        "No document is selected.", fill_width(30.0f));
                    EditorGUI::end_v_layout(gui, root);
                    EditorGUI::end_dock_panel(gui);
                    return;
                }
                if(document->inspector_revision != document->revision ||
                    document->inspector_node != document->selected_node)
                {
                    rebuild_inspector(*document);
                }
                const AuthoringNodeRecord* node = find_authoring_node(*document->snapshot,
                    document->selected_node);
                if(node)
                {
                    String type_name = "Unsupported type";
                    for(const NodeTypeView& type : node_types)
                    {
                        if(type.type == node->type)
                        {
                            type_name = type.display_name;
                            break;
                        }
                    }
                    EditorGUI::text(gui, gui->make_id("inspector.type"), type_name.c_str(),
                        fill_width(30.0f));
                    GUI::ElementHandle name_row = EditorGUI::begin_h_layout(gui,
                        gui->make_id("inspector.node_name.row"), "Name", fill_width(30.0f));
                    EditorGUI::text(gui, gui->make_id("inspector.node_name.label"), "Name",
                        fixed_layout(112.0f, 28.0f));
                    EditorGUI::input_text(gui, gui->make_id("inspector.node_name"),
                        document->node_name, fill_width(28.0f));
                    EditorGUI::end_h_layout(gui, name_row);
                    EditorGUI::ScrollViewDesc scroll_desc;
                    scroll_desc.horizontal = false;
                    EditorGUI::begin_scroll_view(gui, gui->make_id("inspector.properties.scroll"),
                        "Property Inspector", fill_layout(), scroll_desc);

                    auto draw_section = [&](EditingPropertySection section, const c8* id,
                        const c8* label, i32 raw_filter)
                    {
                        bool has_properties = false;
                        for(const PropertyEditor& property : document->property_editors)
                        {
                            if(property.desc.section == section &&
                                (raw_filter < 0 || property.raw == (raw_filter != 0)))
                            {
                                has_properties = true;
                                break;
                            }
                        }
                        if(!has_properties && raw_filter != 0) return;
                        GUI::id_t section_id = gui->make_id(id);
                        if(!EditorGUI::collapsing_header(gui, section_id, label,
                            fill_width(30.0f))) return;
                        if(!has_properties)
                        {
                            String empty_message;
                            strprintf(empty_message, "No %s fields.", label);
                            EditorGUI::text(gui, GUI::make_scoped_id(section_id, "empty"),
                                empty_message.c_str(), fill_width(24.0f));
                            return;
                        }
                        GUI::id_t node_scope = guid_gui_id(section_id,
                            document->selected_node);
                        for(usize property_index = 0;
                            property_index < document->property_editors.size(); ++property_index)
                        {
                            PropertyEditor& property =
                                document->property_editors[property_index];
                            if(property.desc.section != section ||
                                (raw_filter >= 0 && property.raw != (raw_filter != 0))) continue;
                            GUI::id_t target_scope = GUI::make_scoped_id(node_scope,
                                property.target == PropertyTarget::node ? "node" : "attachment");
                            GUI::id_t property_id = GUI::make_scoped_id(target_scope,
                                property.desc.id.c_str());
                            bool stretched_size = parent_flex_stretches_size(*document, property);
                            bool stacked = property.desc.editor == EditingPropertyEditor::float2 ||
                                property.desc.editor == EditingPropertyEditor::float4 ||
                                property.desc.editor == EditingPropertyEditor::color ||
                                property.desc.editor == EditingPropertyEditor::visual_effects ||
                                property.desc.editor == EditingPropertyEditor::size ||
                                property.desc.editor == EditingPropertyEditor::asset ||
                                property.desc.editor == EditingPropertyEditor::json;
                            f32 row_height = property.desc.editor ==
                                EditingPropertyEditor::visual_effects ?
                                visual_effects_editor_height(property) :
                                (property.desc.editor == EditingPropertyEditor::float4 ?
                                142.0f : (stretched_size ? 86.0f : 58.0f));
                            GUI::ElementHandle row = stacked ?
                                EditorGUI::begin_v_layout(gui, property_id,
                                    property.desc.display_name.c_str(), fill_width(row_height)) :
                                EditorGUI::begin_h_layout(gui, property_id,
                                    property.desc.display_name.c_str(), fill_width(32.0f));
                            EditorGUI::text(gui, GUI::make_scoped_id(property_id, "label"),
                                property.desc.display_name.c_str(), stacked ?
                                fill_width(24.0f) : fixed_layout(112.0f, 28.0f));
                            GUI::ElementHandle control;
                            f32 minimum = property.desc.bounded ?
                                (f32)property.desc.minimum : 0.0f;
                            f32 maximum = property.desc.bounded ?
                                (f32)property.desc.maximum : 0.0f;
                            EditorGUI::DragDesc drag_desc;
                            drag_desc.speed = (f32)property.desc.step;
                            switch(property.desc.editor)
                            {
                            case EditingPropertyEditor::boolean:
                                control = EditorGUI::checkbox(gui,
                                    GUI::make_scoped_id(property_id, "value"), "",
                                    &property.boolean, fill_width(28.0f));
                                break;
                            case EditingPropertyEditor::number:
                                control = EditorGUI::drag_float(gui,
                                    GUI::make_scoped_id(property_id, "value"), &property.number,
                                    minimum, maximum, fill_width(28.0f), drag_desc);
                                break;
                            case EditingPropertyEditor::string:
                            case EditingPropertyEditor::name:
                            case EditingPropertyEditor::json:
                                control = EditorGUI::input_text(gui,
                                    GUI::make_scoped_id(property_id, "value"), property.text,
                                    fill_width(28.0f));
                                break;
                            case EditingPropertyEditor::enumeration:
                            {
                                Vector<const c8*> items;
                                items.reserve(property.desc.enumeration_items.size());
                                for(const EditingEnumItemDesc& item :
                                    property.desc.enumeration_items)
                                    items.push_back(item.display_name.c_str());
                                control = EditorGUI::combo(gui,
                                    GUI::make_scoped_id(property_id, "value"),
                                    property.desc.display_name.c_str(), &property.selected_item,
                                    Span<const c8*>(items.data(), items.size()), fill_width(28.0f));
                                break;
                            }
                            case EditingPropertyEditor::float2:
                                control = EditorGUI::drag_float2(gui,
                                    GUI::make_scoped_id(property_id, "value"), property.vector,
                                    minimum, maximum, fill_width(28.0f), drag_desc);
                                break;
                            case EditingPropertyEditor::float4:
                            {
                                const c8* side_ids[] = {"left", "top", "right", "bottom"};
                                const c8* side_labels[] = {"Left", "Top", "Right", "Bottom"};
                                for(usize side = 0; side < 4; ++side)
                                {
                                    GUI::id_t side_id = GUI::make_scoped_id(property_id,
                                        side_ids[side]);
                                    GUI::ElementHandle side_row = EditorGUI::begin_h_layout(gui,
                                        side_id, side_labels[side], fill_width(28.0f));
                                    GUI::LayoutConfig label_layout = fixed_layout(64.0f, 28.0f);
                                    label_layout.flex_shrink = 0.0f;
                                    EditorGUI::text(gui, GUI::make_scoped_id(side_id, "label"),
                                        side_labels[side], label_layout);
                                    GUI::ElementHandle side_control = EditorGUI::drag_float(gui,
                                        GUI::make_scoped_id(side_id, "value"),
                                        property.vector + side, minimum, maximum,
                                        fill_width(28.0f), drag_desc);
                                    if(!control.id) control = side_control;
                                    EditorGUI::end_h_layout(gui, side_row);
                                }
                                break;
                            }
                            case EditingPropertyEditor::color:
                                control = EditorGUI::color_edit4(gui,
                                    GUI::make_scoped_id(property_id, "value"),
                                    property.desc.display_name.c_str(), property.vector,
                                    fill_width(28.0f));
                                break;
                            case EditingPropertyEditor::visual_effects:
                                control = build_visual_effects_editor(property, property_index,
                                    property_id, handles);
                                break;
                            case EditingPropertyEditor::size:
                            {
                                GUI::ElementHandle size_row = EditorGUI::begin_h_layout(gui,
                                    GUI::make_scoped_id(property_id, "value"), "Size",
                                    fill_width(28.0f));
                                const c8* modes[] = {"Auto", "Fixed", "Percent"};
                                GUI::LayoutConfig mode_layout = fixed_layout(112.0f, 28.0f);
                                mode_layout.flex_shrink = 0.0f;
                                control = EditorGUI::combo(gui,
                                    GUI::make_scoped_id(property_id, "mode"), "Size Mode",
                                    &property.size_mode, Span<const c8*>(modes, 3),
                                    mode_layout);
                                if(property.size_mode != 0)
                                {
                                    EditorGUI::drag_float(gui,
                                        GUI::make_scoped_id(property_id, "size"), &property.number,
                                        minimum, maximum, fill_width(28.0f), drag_desc);
                                    if(property.size_mode == 2)
                                    {
                                        EditorGUI::text(gui,
                                            GUI::make_scoped_id(property_id, "percent_suffix"), "%",
                                            fixed_layout(14.0f, 28.0f));
                                    }
                                }
                                GUI::FlexLayoutDesc flex;
                                flex.main_axis_gap = 4.0f;
                                EditorGUI::end_h_layout(gui, size_row, flex);
                                if(stretched_size)
                                {
                                    GUI::ElementHandle warning_row = EditorGUI::begin_h_layout(gui,
                                        GUI::make_scoped_id(property_id, "stretch_warning"),
                                        "Stretch Warning", fill_width(24.0f));
                                    EditorGUI::IconDesc icon_desc;
                                    icon_desc.tint = Float4U(0.957f, 0.678f, 0.184f, 1.0f);
                                    icon_desc.size = 15.0f;
                                    EditorGUI::icon(gui,
                                        GUI::make_scoped_id(property_id, "stretch_warning.icon"),
                                        EditorGUI::IconName::warning, fixed_layout(18.0f, 22.0f),
                                        icon_desc);
                                    String warning;
                                    strprintf(warning, "Parent Flex Stretch overrides %s.",
                                        property.desc.display_name.c_str());
                                    EditorGUI::TextDesc text_desc;
                                    text_desc.typography = EditorGUI::TypographyRole::caption;
                                    text_desc.color = icon_desc.tint;
                                    EditorGUI::text(gui,
                                        GUI::make_scoped_id(property_id, "stretch_warning.text"),
                                        warning.c_str(), fill_width(22.0f), text_desc);
                                    GUI::FlexLayoutDesc warning_flex;
                                    warning_flex.axis = GUI::LayoutAxis::x;
                                    warning_flex.cross_alignment = GUI::FlexAlignment::center;
                                    warning_flex.main_axis_gap = 4.0f;
                                    EditorGUI::end_h_layout(gui, warning_row, warning_flex);
                                }
                                break;
                            }
                            case EditingPropertyEditor::asset:
                            {
                                GUI::ElementHandle asset_row = EditorGUI::begin_h_layout(gui,
                                    GUI::make_scoped_id(property_id, "value"), "Asset",
                                    fill_width(28.0f));
                                control = EditorGUI::input_text(gui,
                                    GUI::make_scoped_id(property_id, "asset_guid"), property.text,
                                    fill_width(28.0f));
                                PropertyActionHit hit;
                                hit.property_index = property_index;
                                hit.element = EditorGUI::text_button(gui,
                                    GUI::make_scoped_id(property_id, "browse"), "...",
                                    fixed_layout(32.0f, 28.0f));
                                handles.browse_assets.push_back(hit);
                                GUI::FlexLayoutDesc flex;
                                flex.main_axis_gap = 4.0f;
                                EditorGUI::end_h_layout(gui, asset_row, flex);
                                break;
                            }
                            }
                            if(!property.desc.description.empty() && control.id)
                            {
                                EditorGUI::set_item_tooltip(gui,
                                    GUI::make_scoped_id(property_id, "tooltip"), control,
                                    property.desc.description.c_str());
                            }
                            if(stacked) EditorGUI::end_v_layout(gui, row);
                            else EditorGUI::end_h_layout(gui, row);
                        }
                    };

                    draw_section(EditingPropertySection::layout,
                        "inspector.section.layout", "Layout", 0);
                    draw_section(EditingPropertySection::style,
                        "inspector.section.style", "Style", 0);
                    draw_section(EditingPropertySection::property,
                        "inspector.section.properties", "Properties", 0);
                    draw_section(EditingPropertySection::property,
                        "inspector.section.raw", "Raw / Unknown Properties", 1);
                    EditorGUI::end_scroll_view(gui);
                }
                EditorGUI::end_v_layout(gui, root);
                EditorGUI::end_dock_panel(gui);
            }

            void EditorApp::build_diagnostics_panel()
            {
                if(!EditorGUI::begin_dock_panel(gui, gui->make_id("panel.diagnostics"),
                    "Diagnostics")) return;
                GUI::ElementHandle root = EditorGUI::begin_v_layout(gui,
                    gui->make_id("diagnostics.root"), "Diagnostics Root", fill_layout());
                if(!error_message.empty())
                {
                    String message = "Service: ";
                    message.append(error_message);
                    EditorGUI::text(gui, gui->make_id("diagnostics.service"), message.c_str(),
                        fill_width(26.0f));
                }
                DocumentView* document = active_document();
                if(document && document->diagnostics.type() == VariantType::array &&
                    !document->diagnostics.empty())
                {
                    usize index = 0;
                    for(const Variant& diagnostic : document->diagnostics.values())
                    {
                        String message;
                        strprintf(message, "[%s] %s", diagnostic["severity"].c_str("error"),
                            diagnostic["message"].c_str());
                        EditorGUI::text(gui, GUI::make_scoped_id(gui->make_id("diagnostics.items"),
                            (u64)++index), message.c_str(), fill_width(24.0f));
                    }
                }
                else if(error_message.empty())
                {
                    EditorGUI::text(gui, gui->make_id("diagnostics.clean"),
                        "No diagnostics for the active document.", fill_width(26.0f));
                }
                EditorGUI::end_v_layout(gui, root);
                EditorGUI::end_dock_panel(gui);
            }

            GUI::ElementHandle EditorApp::build_editor(UIHandles& handles)
            {
                GUI::ElementHandle root = EditorGUI::begin_v_layout(gui,
                    gui->make_id("editor.root"), "GameGUI Editor Root", fill_layout());
#if !defined(LUNA_PLATFORM_MACOS)
                build_main_menu_bar(handles);
#endif
                EditorGUI::begin_dock_space(gui,
                    gui->make_id("editor.dock_space"), "GameGUI Editor DockSpace", fill_layout());
                build_document_panels(handles);
                build_hierarchy_panel(handles);
                build_palette_panel(handles);
                build_inspector_panel(handles);
                build_diagnostics_panel();
                EditorGUI::end_dock_space(gui);
                EditorGUI::end_v_layout(gui, root);
                return root;
            }
        }
    }
}
