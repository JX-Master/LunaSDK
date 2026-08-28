/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorDocuments.cpp
* @author JXMaster
* @date 2026/8/28
*/
#include "EditorApp.hpp"
#include <Luna/VFS/VFS.hpp>
#include <Luna/Window/FileDialog.hpp>
#include <Luna/Window/MessageBox.hpp>

namespace Luna
{
    namespace GameGUIEditor
    {
        namespace Internal
        {
            DocumentView* EditorApp::find_document(u64 id)
            {
                for(DocumentView& document : documents)
                {
                    if(document.id == id) return &document;
                }
                return nullptr;
            }

            DocumentView* EditorApp::active_document()
            {
                if(documents.empty()) return nullptr;
                selected_document = clamp(selected_document, 0, (i32)documents.size() - 1);
                return &documents[(usize)selected_document];
            }

            void EditorApp::place_document_panel(u64 document_id, u64 target_document_id)
            {
                GUI::id_t dock_space = gui->make_id("editor.dock_space");
                GUI::id_t panel = document_panel_id(gui, document_id);
                bool placed = false;
                if(target_document_id && target_document_id != document_id)
                {
                    placed = EditorGUI::dock_panel(gui, dock_space, panel,
                        document_panel_id(gui, target_document_id));
                }
                if(!placed)
                {
                    placed = EditorGUI::dock_panel(gui, dock_space, panel,
                        gui->make_id("panel.diagnostics"), EditorGUI::DockPanelPlacement::up, 0.78f);
                }
                if(!placed)
                {
                    error_message = "The document panel could not be added to the dock space.";
                    return;
                }
                EditorGUI::activate_dock_panel(gui, dock_space, panel);
            }

            bool EditorApp::invoke(const c8* url, const Variant& params, Variant& result)
            {
                auto response = service->frontend()->invoke(url, params);
                if(!response.valid())
                {
                    error_message = explain(response.errcode());
                    return false;
                }
                result = move(response.get());
                error_message.clear();
                return true;
            }

            bool EditorApp::update_metadata(DocumentView& document, const Variant& metadata)
            {
                if(metadata.type() != VariantType::object) return false;
                document.id = metadata["document_id"].unum();
                document.revision = metadata["revision"].unum();
                document.history_state = metadata["history_state"].unum();
                document.title = metadata["title"].c_str();
                document.asset_path = metadata["asset_path"].c_str();
                decode_guid_string(metadata["asset_guid"], document.asset_guid);
                document.dirty = metadata["dirty"].boolean();
                document.can_undo = metadata["can_undo"].boolean();
                document.can_redo = metadata["can_redo"].boolean();
                document.diagnostics = metadata["diagnostics"];
                return true;
            }

            bool EditorApp::refresh_snapshot(DocumentView& document)
            {
                Variant params(VariantType::object);
                params["document_id"] = document.id;
                Variant snapshot;
                if(!invoke(GameGUIEditor::GET_SNAPSHOT_URL, params, snapshot)) return false;
                update_metadata(document, snapshot);
                auto decoded = GameGUI::decode_document(snapshot["document"]);
                if(!decoded.valid())
                {
                    error_message = explain(decoded.errcode());
                    return false;
                }
                document.snapshot = decoded.get();
                if(!GameGUI::find_node(*document.snapshot, document.selected_node))
                    document.selected_node = document.snapshot->root;
                document.inspector_revision = 0;
                document.preview.revision = 0;
                return true;
            }

            bool EditorApp::create_document()
            {
                DocumentView* target = active_document();
                u64 target_id = target ? target->id : 0;
                Variant metadata;
                if(!invoke(GameGUIEditor::CREATE_DOCUMENT_URL, Variant(VariantType::object), metadata))
                    return false;
                DocumentView document;
                update_metadata(document, metadata);
                documents.push_back(move(document));
                selected_document = (i32)documents.size() - 1;
                if(!refresh_snapshot(documents.back())) return false;
                if(dock_layout_initialized) place_document_panel(documents.back().id, target_id);
                return true;
            }

            bool EditorApp::native_path_to_asset_path(Path native_path, String& asset_path)
            {
                native_path.normalize();
                if(!native_path.is_subpath_of(workspace_root))
                {
                    error_message = "GameGUI assets must be stored inside the current workspace.";
                    return false;
                }
                if(!native_path.extension().empty())
                {
                    if(native_path.extension() != "json")
                    {
                        error_message = "GameGUI document files must use the .json extension.";
                        return false;
                    }
                    native_path.remove_extension();
                }
                Path relative_path;
                relative_path.assign_relative(workspace_root, native_path);
                Path vfs_path("/");
                vfs_path.append(relative_path);
                asset_path = vfs_path.encode();
                return true;
            }

            bool EditorApp::open_document()
            {
                Window::FileDialogFilter filter;
                filter.name = "GameGUI Document";
                const c8* extension = "json";
                filter.extensions = {&extension, 1};
                auto selected_files = Window::open_file_dialog("Open GameGUI Document",
                    {&filter, 1}, workspace_root);
                if(!selected_files.valid())
                {
                    if(selected_files.errcode() != E_INTERRUPTED)
                        error_message = explain(selected_files.errcode());
                    return false;
                }
                if(selected_files.get().empty()) return false;
                String asset_path;
                if(!native_path_to_asset_path(selected_files.get()[0], asset_path)) return false;
                Variant params(VariantType::object);
                params["path"] = asset_path.c_str();
                Variant metadata;
                if(!invoke(GameGUIEditor::OPEN_DOCUMENT_URL, params, metadata)) return false;
                u64 id = metadata["document_id"].unum();
                for(usize i = 0; i < documents.size(); ++i)
                {
                    if(documents[i].id == id)
                    {
                        selected_document = (i32)i;
                        if(!refresh_snapshot(documents[i])) return false;
                        if(dock_layout_initialized)
                        {
                            EditorGUI::activate_dock_panel(gui, gui->make_id("editor.dock_space"),
                                document_panel_id(gui, documents[i].id));
                        }
                        return true;
                    }
                }
                DocumentView* target = active_document();
                u64 target_id = target ? target->id : 0;
                DocumentView document;
                update_metadata(document, metadata);
                documents.push_back(move(document));
                selected_document = (i32)documents.size() - 1;
                if(!refresh_snapshot(documents.back())) return false;
                if(dock_layout_initialized) place_document_panel(documents.back().id, target_id);
                return true;
            }

            void EditorApp::rebuild_inspector(DocumentView& document)
            {
                if(!document.snapshot) return;
                const GameGUI::NodeRecord* node = GameGUI::find_node(*document.snapshot,
                    document.selected_node);
                if(!node) return;
                document.node_name = node->name.c_str();
                document.property_editors.clear();
                for(const auto& item : node->properties.key_values())
                {
                    PropertyEditor editor;
                    editor.key = item.first;
                    editor.original = item.second;
                    editor.boolean = item.second.boolean();
                    editor.text = property_text(item.second);
                    document.property_editors.push_back(move(editor));
                }
                document.inspector_revision = document.revision;
                document.inspector_node = document.selected_node;
            }
            bool EditorApp::confirm_exit()
            {
                if(!has_dirty_documents()) return true;
                constexpr usize DISCARD_BUTTON_INDEX = 0;
                constexpr usize CANCEL_BUTTON_INDEX = 1;
                const c8* buttons[] = {"Discard Changes", "Cancel"};
                auto response = Window::message_box(
                    "There are unsaved changes. Discard them and quit?", "Unsaved Changes",
                    Span<const c8*>(buttons, 2), Window::MessageBoxIcon::warning,
                    DISCARD_BUTTON_INDEX, CANCEL_BUTTON_INDEX);
                if(!response.valid())
                {
                    error_message = explain(response.errcode());
                    return false;
                }
                return response.get() == DISCARD_BUTTON_INDEX;
            }

            void EditorApp::undo_document(DocumentView& document)
            {
                if(!document.can_undo) return;
                Variant result;
                if(invoke(GameGUIEditor::UNDO_URL, editing_params(document), result))
                    refresh_snapshot(document);
            }

            void EditorApp::redo_document(DocumentView& document)
            {
                if(!document.can_redo) return;
                Variant result;
                if(invoke(GameGUIEditor::REDO_URL, editing_params(document), result))
                    refresh_snapshot(document);
            }

            void EditorApp::save(DocumentView& document, bool save_as)
            {
                Variant params = editing_params(document);
                const c8* url = GameGUIEditor::SAVE_URL;
                if(save_as || document.asset_path.empty())
                {
                    Window::FileDialogFilter filter;
                    filter.name = "GameGUI Document";
                    const c8* extension = "json";
                    filter.extensions = {&extension, 1};
                    Path initial_path = workspace_root;
                    if(document.asset_path.empty()) initial_path.push_back(Name("Untitled.json"));
                    else
                    {
                        Path current_path(document.asset_path.c_str());
                        current_path.append_extension("json");
                        initial_path.append(current_path);
                    }
                    auto selected_path = Window::save_file_dialog("Save GameGUI Document As",
                        {&filter, 1}, initial_path);
                    if(!selected_path.valid())
                    {
                        if(selected_path.errcode() != E_INTERRUPTED)
                            error_message = explain(selected_path.errcode());
                        return;
                    }
                    String asset_path;
                    if(!native_path_to_asset_path(selected_path.get(), asset_path)) return;
                    params["path"] = asset_path.c_str();
                    url = GameGUIEditor::SAVE_AS_URL;
                }
                Variant metadata;
                if(invoke(url, params, metadata)) refresh_snapshot(document);
            }

            void EditorApp::remove_document_view(u64 id)
            {
                for(usize i = 0; i < documents.size(); ++i)
                {
                    if(documents[i].id == id)
                    {
                        documents.erase(documents.begin() + i);
                        if(documents.empty()) selected_document = 0;
                        else
                        {
                            usize next = min(i, documents.size() - 1);
                            selected_document = (i32)next;
                            EditorGUI::activate_dock_panel(gui, gui->make_id("editor.dock_space"),
                                document_panel_id(gui, documents[next].id));
                        }
                        break;
                    }
                }
            }

            void EditorApp::request_close(DocumentView& document, bool discard)
            {
                if(document.dirty && !discard)
                {
                    constexpr usize SAVE_BUTTON_INDEX = 0;
                    constexpr usize DISCARD_BUTTON_INDEX = 1;
                    constexpr usize CANCEL_BUTTON_INDEX = 2;
                    const c8* buttons[] = {"Save", "Discard", "Cancel"};
                    String message;
                    strprintf(message, "Save changes to \"%s\" before closing?", document.title.c_str());
                    auto response = Window::message_box(message.c_str(), "Unsaved Changes",
                        Span<const c8*>(buttons, 3), Window::MessageBoxIcon::warning,
                        SAVE_BUTTON_INDEX, CANCEL_BUTTON_INDEX);
                    if(!response.valid())
                    {
                        document.panel_open = true;
                        error_message = explain(response.errcode());
                        return;
                    }
                    if(response.get() == CANCEL_BUTTON_INDEX)
                    {
                        document.panel_open = true;
                        return;
                    }
                    if(response.get() == SAVE_BUTTON_INDEX)
                    {
                        save(document, document.asset_path.empty());
                        if(document.dirty)
                        {
                            document.panel_open = true;
                            return;
                        }
                    }
                    else discard = true;
                }
                Variant params = editing_params(document);
                params["discard"] = discard;
                Variant result;
                if(invoke(GameGUIEditor::CLOSE_DOCUMENT_URL, params, result))
                {
                    u64 id = document.id;
                    // The editor draw list generated earlier in this frame may still refer to
                    // resources owned by this document, notably its preview texture. Keep the
                    // view alive until Renderer has consumed the draw list.
                    bool already_deferred = false;
                    for(u64 deferred_id : deferred_document_removals)
                    {
                        if(deferred_id == id)
                        {
                            already_deferred = true;
                            break;
                        }
                    }
                    if(!already_deferred) deferred_document_removals.push_back(id);
                }
                else document.panel_open = true;
            }
        }
    }
}
