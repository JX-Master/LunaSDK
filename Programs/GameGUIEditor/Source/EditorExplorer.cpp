/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorExplorer.cpp
* @author JXMaster
* @date 2026/9/11
*/
#include "EditorApp.hpp"
#include <Luna/Runtime/Algorithm.hpp>
#include <cstring>

namespace Luna::GameGUIEditor::Internal
{
    void EditorApp::build_explorer_folder(const Variant& directory, const Path& folder,
        u32 depth, UIHandles& handles)
    {
        u64 id = directory["working_directory_id"].unum();
        GUI::id_t scope = GUI::make_scoped_id(gui->make_id("explorer.directory"), id);
        Vector<Path> children;
        for(const Variant& value : directory["folders"].values())
        {
            Path path(value.c_str());
            Path parent = path;
            parent.pop_back();
            if(parent == folder) children.push_back(path);
        }
        sort(children.begin(), children.end(), [](const Path& a, const Path& b)
            { return strcmp(a.encode().c_str(), b.encode().c_str()) < 0; });
        for(const Path& child : children)
        {
            auto flags = EditorGUI::TreeNodeFlag::open_on_arrow;
            if(selected_directory == id && selected_folder == child) flags |= EditorGUI::TreeNodeFlag::selected;
            GUI::ElementHandle element;
            bool expanded = EditorGUI::tree_node(gui, GUI::make_scoped_id(scope, child.encode().c_str()),
                child.back().c_str(), flags, depth, fill_width(26.0f), EditorGUI::DisclosureDesc(), &element);
            handles.explorer.push_back({id, child, Guid(), element});
            if(expanded) build_explorer_folder(directory, child, depth + 1, handles);
        }
        Vector<const Variant*> assets;
        for(const Variant& asset : directory["assets"].values())
        {
            if(asset["type"].str() != GameGUI::get_asset_type()) continue;
            Path parent(asset["path"].c_str());
            parent.pop_back();
            if(parent == folder) assets.push_back(&asset);
        }
        sort(assets.begin(), assets.end(), [](const Variant* a, const Variant* b)
            { return strcmp((*a)["path"].c_str(), (*b)["path"].c_str()) < 0; });
        for(const Variant* value : assets)
        {
            const Variant& asset = *value;
            Guid guid;
            if(!decode_guid_string(asset["asset_guid"], guid)) continue;
            Path path(asset["path"].c_str());
            String label = path.back().c_str();
            auto flags = EditorGUI::TreeNodeFlag::leaf;
            for(const DocumentView& document : documents)
            {
                if(document.asset_guid != guid) continue;
                if(document.dirty) label.append(" *");
                if(active_document() && active_document()->id == document.id) flags |= EditorGUI::TreeNodeFlag::selected;
            }
            if(!asset["editable"].boolean()) label.append(" (no Authoring)");
            GUI::ElementHandle element;
            EditorGUI::tree_node(gui, guid_gui_id(scope, guid), label.c_str(), flags, depth,
                fill_width(26.0f), EditorGUI::DisclosureDesc(), &element);
            // Disclosure leaves do not toggle and are non-interactive by default. Explorer
            // still needs their selection, double-click and hover events.
            GUI::Interactable interactable;
            interactable.pointer_hit_behavior = GUI::PointerHitBehavior::target;
            set_flags(interactable.flags, GUI::InteractableFlag::hoverable);
            set_flags(interactable.flags, GUI::InteractableFlag::activatable);
            set_flags(interactable.flags, GUI::InteractableFlag::focusable);
            gui->set_interactable(element, interactable);
            handles.explorer.push_back({id, path, guid, element});
            EditorGUI::set_item_tooltip(gui, GUI::make_scoped_id(element.id, "tooltip"), element,
                asset["editable"].boolean() ? path.encode().c_str() : "This asset has no editable Authoring data unit.");
        }
    }

    void EditorApp::build_explorer_panel(UIHandles& handles)
    {
        if(!EditorGUI::begin_dock_panel(gui, gui->make_id("panel.explorer"), "Explorer")) return;
        if(working_directories.empty())
        {
            EditorGUI::text(gui, gui->make_id("explorer.empty"), "Open a working directory to create or edit assets.", fill_width(52.0f));
            handles.open_directory_button = EditorGUI::text_button(gui, gui->make_id("explorer.open"), "Open Directory...", fill_width(30.0f));
        }
        EditorGUI::ScrollViewDesc scroll;
        scroll.horizontal = false;
        EditorGUI::begin_scroll_view(gui, gui->make_id("explorer.scroll"), "Asset Explorer", fill_layout(), scroll);
        for(const Variant& directory : working_directories.values())
        {
            u64 id = directory["working_directory_id"].unum();
            Path path(directory["native_path"].c_str());
            String label = path.empty() ? directory["native_path"].c_str() : path.back().c_str();
            if(label.empty()) label = directory["native_path"].c_str();
            if(directory["closing"].boolean()) label.append(" (closing)");
            auto flags = EditorGUI::TreeNodeFlag::open_on_arrow;
            if(selected_directory == id && selected_folder.empty()) flags |= EditorGUI::TreeNodeFlag::selected;
            GUI::ElementHandle element;
            bool expanded = EditorGUI::tree_node(gui, GUI::make_scoped_id(gui->make_id("explorer.root"), id),
                label.c_str(), flags, 0, fill_width(28.0f), EditorGUI::DisclosureDesc(), &element);
            handles.explorer.push_back({id, Path(), Guid(), element});
            EditorGUI::set_item_tooltip(gui, GUI::make_scoped_id(element.id, "tooltip"), element, directory["native_path"].c_str());
            if(expanded) build_explorer_folder(directory, Path(), 1, handles);
        }
        EditorGUI::end_scroll_view(gui);
        GUI::id_t popup_id = gui->make_id("explorer.context");
        EditorGUI::PopupDesc desc;
        desc.position = explorer_context_position;
        desc.layout = fixed_layout(190.0f, 74.0f);
        GUI::ElementHandle popup;
        if(EditorGUI::begin_popup(gui, popup_id, desc, &popup))
        {
            handles.refresh_directory = EditorGUI::menu_item(gui, GUI::make_scoped_id(popup_id, "refresh"), "Refresh Directory");
            handles.unload_directory = EditorGUI::menu_item(gui, GUI::make_scoped_id(popup_id, "unload"), "Unload Directory");
            lupanic_if_failed(EditorGUI::end_popup(gui, popup, RectF(0, 0, 190, 74)));
        }
        EditorGUI::end_dock_panel(gui);
    }

    bool EditorApp::process_explorer_interactions(const UIHandles& handles)
    {
        if(EditorGUI::is_item_clicked(gui, handles.open_directory) ||
            EditorGUI::is_item_clicked(gui, handles.open_directory_button))
        {
            open_working_directory();
            return true;
        }
        if(EditorGUI::is_item_clicked(gui, handles.unload_directory))
        {
            EditorGUI::close_popup(gui, gui->make_id("explorer.context"));
            Variant plan;
            if(confirm_directory_close(explorer_context_directory, plan)) deferred_directory_closes.push_back(move(plan));
            return true;
        }
        if(EditorGUI::is_item_clicked(gui, handles.refresh_directory))
        {
            EditorGUI::close_popup(gui, gui->make_id("explorer.context"));
            Variant params(VariantType::object), result;
            params["working_directory_id"] = explorer_context_directory;
            if(!invoke(REFRESH_DIRECTORY_URL, params, result)) show_file_error("Refresh Directory Failed");
            else refresh_working_directories();
            return true;
        }
        for(const auto& hit : handles.explorer)
        {
            if(EditorGUI::is_item_clicked(gui, hit.element))
            {
                selected_directory = hit.directory_id;
                selected_folder = hit.path;
                if(hit.asset != Guid()) selected_folder.pop_back();
            }
            if(hit.asset != Guid() && EditorGUI::is_item_double_clicked(gui, hit.element))
            {
                Variant params(VariantType::object);
                params["asset_guid"] = guid_string(hit.asset).c_str();
                open_document_asset(params);
                return true;
            }
            if(hit.asset == Guid() && hit.path.empty())
            {
                for(const auto& routed : gui->get_routed_input_events(hit.element.id))
                {
                    const auto& event = routed.event;
                    if(event.type != GUI::InputEventType::pointer_down || event.button != GUI::PointerButton::right) continue;
                    explorer_context_directory = hit.directory_id;
                    explorer_context_position = event.position;
                    EditorGUI::open_popup(gui, gui->make_id("explorer.context"));
                    return true;
                }
            }
        }
        return false;
    }

    void EditorApp::build_asset_picker(UIHandles& handles)
    {
        auto document = find_document(asset_picker_document);
        GUI::id_t popup_id = gui->make_id("inspector.asset_picker");
        if(!document || document->revision != asset_picker_revision ||
            asset_picker_property >= document->property_editors.size())
        {
            EditorGUI::close_popup(gui, popup_id);
            return;
        }
        EditorGUI::PopupDesc desc;
        desc.position = asset_picker_position;
        desc.layout = fixed_layout(360.0f, 320.0f);
        GUI::ElementHandle popup;
        if(!EditorGUI::begin_popup(gui, popup_id, desc, &popup)) return;
        EditorGUI::text(gui, GUI::make_scoped_id(popup_id, "title"), "Assets in Open Working Directories", fill_width(26.0f));
        EditorGUI::ScrollViewDesc scroll;
        scroll.horizontal = false;
        EditorGUI::begin_scroll_view(gui, GUI::make_scoped_id(popup_id, "scroll"), "Select Asset", fill_layout(), scroll);
        const auto& property = document->property_editors[asset_picker_property];
        for(const Variant& directory : working_directories.values())
        {
            if(directory["closing"].boolean()) continue;
            for(const Variant& asset : directory["assets"].values())
            {
                if(!property.desc.asset_type.empty() && asset["type"].str() != property.desc.asset_type) continue;
                Guid guid;
                if(!decode_guid_string(asset["asset_guid"], guid)) continue;
                Path root(directory["native_path"].c_str());
                String label;
                strprintf(label, "%s / %s", root.empty() ? "/" : root.back().c_str(), asset["path"].c_str());
                GUI::ElementHandle item = EditorGUI::text_button(gui, guid_gui_id(popup_id, guid), label.c_str(), fill_width(28.0f));
                handles.asset_choices.push_back({directory["working_directory_id"].unum(), Path(asset["path"].c_str()), guid, item});
            }
        }
        EditorGUI::end_scroll_view(gui);
        lupanic_if_failed(EditorGUI::end_popup(gui, popup, RectF(0, 0, 360, 320)));
    }
}
