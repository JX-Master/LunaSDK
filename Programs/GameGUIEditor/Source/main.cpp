/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file main.cpp
* @author JXMaster
* @date 2026/8/26
*/
#include "../Service/GameGUIEditorService.hpp"
#include <Luna/Asset/Asset.hpp>
#include <Luna/EditorGUI/EditorGUI.hpp>
#include <Luna/Font/Font.hpp>
#include <Luna/GameGUI/GameGUI.hpp>
#include <Luna/GUIWindow/GUIWindow.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/RHI/SwapChain.hpp>
#include <Luna/RHIUtility/BlitContext.hpp>
#include <Luna/RHIUtility/RHIUtility.hpp>
#include <Luna/Runtime/Guid.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/VariantUtils/JSON.hpp>
#include <Luna/VFS/VFS.hpp>
#include <Luna/VG/VG.hpp>
#include <Luna/Window/AppMain.hpp>
#include <Luna/Window/Event.hpp>
#include <Luna/Window/FileDialog.hpp>
#include <Luna/Window/MessageBox.hpp>
#include <Luna/Window/Window.hpp>
#if defined(LUNA_PLATFORM_MACOS)
#include <Luna/Window/ApplicationMenu.hpp>
#endif
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace Luna;

namespace
{
    constexpr c8 APP_NAME[] = "Luna GameGUI Editor";
    constexpr u32 PREVIEW_WIDTH = 640;
    constexpr u32 PREVIEW_HEIGHT = 480;

#if defined(LUNA_PLATFORM_MACOS)
    constexpr Window::application_menu_item_id_t MENU_ITEM_NEW = 1;
    constexpr Window::application_menu_item_id_t MENU_ITEM_OPEN = 2;
    constexpr Window::application_menu_item_id_t MENU_ITEM_SAVE = 3;
    constexpr Window::application_menu_item_id_t MENU_ITEM_SAVE_AS = 4;
    constexpr Window::application_menu_item_id_t MENU_ITEM_CLOSE = 5;
    constexpr Window::application_menu_item_id_t MENU_ITEM_UNDO = 6;
    constexpr Window::application_menu_item_id_t MENU_ITEM_REDO = 7;

    Window::ApplicationMenuItemDesc menu_command(const c8* title,
        Window::application_menu_item_id_t id, KeyCode shortcut_key = KeyCode::unknown,
        Window::KeyModifierFlag shortcut_modifiers = Window::KeyModifierFlag::none)
    {
        Window::ApplicationMenuItemDesc desc;
        desc.title = title;
        desc.id = id;
        desc.shortcut_key = shortcut_key;
        desc.shortcut_modifiers = shortcut_modifiers;
        return desc;
    }

    Window::ApplicationMenuItemDesc standard_menu_command(Window::ApplicationMenuItemRole role,
        KeyCode shortcut_key = KeyCode::unknown,
        Window::KeyModifierFlag shortcut_modifiers = Window::KeyModifierFlag::none)
    {
        Window::ApplicationMenuItemDesc desc;
        desc.role = role;
        desc.shortcut_key = shortcut_key;
        desc.shortcut_modifiers = shortcut_modifiers;
        return desc;
    }

    Window::ApplicationMenuItemDesc menu_separator()
    {
        Window::ApplicationMenuItemDesc desc;
        desc.type = Window::ApplicationMenuItemType::separator;
        return desc;
    }

    Window::ApplicationMenuItemDesc menu_submenu(const c8* title,
        Span<const Window::ApplicationMenuItemDesc> children,
        Window::ApplicationMenuItemRole role = Window::ApplicationMenuItemRole::none)
    {
        Window::ApplicationMenuItemDesc desc;
        desc.type = Window::ApplicationMenuItemType::submenu;
        desc.role = role;
        desc.title = title;
        desc.children = children;
        return desc;
    }

    Window::ApplicationMenuItemState menu_item_state(bool enabled)
    {
        Window::ApplicationMenuItemState state;
        state.enabled = enabled;
        return state;
    }
#endif

    GUI::LayoutConfig fill_layout()
    {
        GUI::LayoutConfig result;
        result.width.kind = GUI::SizeKind::percent;
        result.width.value = 1.0f;
        result.height.kind = GUI::SizeKind::percent;
        result.height.value = 1.0f;
        result.flex_grow = 1.0f;
        return result;
    }

    GUI::LayoutConfig fill_width(f32 height)
    {
        GUI::LayoutConfig result;
        result.width.kind = GUI::SizeKind::percent;
        result.width.value = 1.0f;
        result.height.kind = GUI::SizeKind::fixed;
        result.height.value = height;
        return result;
    }

    GUI::LayoutConfig grow_width(f32 height)
    {
        GUI::LayoutConfig result;
        result.width.kind = GUI::SizeKind::fit;
        result.height.kind = GUI::SizeKind::fixed;
        result.height.value = height;
        result.flex_grow = 1.0f;
        return result;
    }

    GUI::LayoutConfig fixed_layout(f32 width, f32 height)
    {
        GUI::LayoutConfig result;
        result.width.kind = GUI::SizeKind::fixed;
        result.width.value = width;
        result.height.kind = GUI::SizeKind::fixed;
        result.height.value = height;
        return result;
    }

    String guid_string(const Guid& guid)
    {
        c8 buffer[GUID_STRING_LENGTH];
        RV result = encode_guid(guid, buffer, sizeof(buffer));
        luassert(succeeded(result));
        return String(buffer, sizeof(buffer));
    }

    bool decode_guid_string(const Variant& value, Guid& result)
    {
        return value.type() == VariantType::string &&
            succeeded(decode_guid(value.c_str(), value.str().size(), result));
    }

    GUI::id_t guid_gui_id(GUI::id_t scope, const Guid& guid)
    {
        GUI::id_t result = memhash64(&guid, sizeof(guid), scope);
        return result ? result : GUI::DEFAULT_DATA_SCOPE;
    }

    struct PropertyEditor
    {
        Name key;
        Variant original;
        String text;
        bool boolean = false;
    };

    struct PreviewState
    {
        Ref<GUI::IContext> context;
        Ref<GameGUI::IInstance> instance;
        Ref<RHI::ITexture> target;
        u64 revision = 0;
    };

    struct DocumentView
    {
        u64 id = 0;
        u64 revision = 0;
        u64 history_state = 0;
        String title;
        String asset_path;
        Guid asset_guid;
        bool dirty = false;
        bool can_undo = false;
        bool can_redo = false;
        Ref<GameGUI::Document> snapshot;
        Variant diagnostics;
        Guid selected_node;
        u64 inspector_revision = 0;
        Guid inspector_node;
        String node_name;
        Vector<PropertyEditor> property_editors;
        String new_property_name;
        String new_property_value = "null";
        PreviewState preview;
    };

    struct NodeTypeView
    {
        Guid type;
        String name;
        String display_name;
        String category;
        Variant schema;
    };

    EditorGUI::IconName node_type_icon(const NodeTypeView& type)
    {
        const c8* kind = type.schema.type() == VariantType::object ?
            type.schema["kind"].c_str("") : "";
        if(!strcmp(kind, "flex")) return EditorGUI::IconName::rows;
        if(!strcmp(kind, "canvas")) return EditorGUI::IconName::frame_corners;
        if(!strcmp(kind, "panel")) return EditorGUI::IconName::squares_four;
        if(!strcmp(kind, "text")) return EditorGUI::IconName::cursor_text;
        if(!strcmp(kind, "button")) return EditorGUI::IconName::cursor_click;
        if(!strcmp(kind, "asset_instance")) return EditorGUI::IconName::package;
        if(!strcmp(type.category.c_str(), "Layout")) return EditorGUI::IconName::grid_four;
        if(!strcmp(type.category.c_str(), "Visual")) return EditorGUI::IconName::eye;
        if(!strcmp(type.category.c_str(), "Input")) return EditorGUI::IconName::hand_tap;
        if(!strcmp(type.category.c_str(), "Composition")) return EditorGUI::IconName::stack;
        return EditorGUI::IconName::plus;
    }

    struct NodeHit
    {
        Guid node;
        GUI::ElementHandle element;
    };

    struct TypeHit
    {
        Guid type;
        GUI::ElementHandle element;
    };

    struct UIHandles
    {
        u64 document_id = 0;
        GUI::ElementHandle create_document;
        GUI::ElementHandle open_document;
        GUI::ElementHandle save_document;
        GUI::ElementHandle save_as_document;
        GUI::ElementHandle close_document;
        GUI::ElementHandle undo;
        GUI::ElementHandle redo;
        GUI::ElementHandle delete_node;
        GUI::ElementHandle add_property;
        GUI::ElementHandle preview_host;
        GUI::ElementHandle close_save;
        GUI::ElementHandle close_discard;
        GUI::ElementHandle close_cancel;
        Vector<NodeHit> nodes;
        Vector<TypeHit> types;
    };

    struct PreviewInput
    {
        u64 document_id = 0;
        Vector<GUI::InputEvent> events;
    };

    struct EditorApp
    {
        Ref<Window::IWindow> window;
        Ref<RHI::ISwapChain> swap_chain;
        Ref<RHI::ICommandBuffer> cmdbuf;
        Ref<RHI::ITexture> gui_target;
        Ref<RHIUtility::IBlitContext> blit;
        Ref<GUI::IContext> gui;
        Ref<GUI::IRenderer> renderer;
        UniquePtr<GameGUIEditor::Service> service;
        GUIWindow::GUIWindowInputAdapter input_adapter;
        Vector<DocumentView> documents;
        Vector<u64> deferred_document_removals;
        Vector<NodeTypeView> node_types;
        i32 selected_document = 0;
        u64 pending_close_document = 0;
        bool discard_smoke = false;
        String workspace_path;
        Path workspace_root;
        String error_message;
        u32 queue = U32_MAX;
        u32 width = 0;
        u32 height = 0;
        i32 max_frames = -1;
#if defined(LUNA_PLATFORM_MACOS)
        bool application_menu_has_document = false;
        bool application_menu_can_undo = false;
        bool application_menu_can_redo = false;
#endif

        RV init();
        RV run();
        RV resize_target(const UInt2U& size);
        RV initialize_dock_layout();
        bool invoke(const c8* url, const Variant& params, Variant& result);
        bool native_path_to_asset_path(Path native_path, String& asset_path);
        bool create_document();
        bool open_document();
        bool refresh_snapshot(DocumentView& document);
        bool update_metadata(DocumentView& document, const Variant& metadata);
        DocumentView* find_document(u64 id);
        DocumentView* active_document();
        void rebuild_inspector(DocumentView& document);
        RV ensure_preview(DocumentView& document);
        RV build_preview(DocumentView& document, Span<const GUI::InputEvent> input);
        RV render_frame(DocumentView* preview_document);
        GUI::ElementHandle build_editor(UIHandles& handles);
#if !defined(LUNA_PLATFORM_MACOS)
        void build_main_menu_bar(UIHandles& handles);
#endif
        void build_hierarchy_panel(UIHandles& handles);
        void build_palette_panel(UIHandles& handles);
        void build_document_panel(UIHandles& handles);
        void build_inspector_panel(UIHandles& handles);
        void build_diagnostics_panel();
        void build_hierarchy_node(DocumentView& document, const Guid& node, u32 depth,
            UIHandles& handles);
        void process_interactions(const UIHandles& handles);
        void apply_inspector_changes(DocumentView& document);
        void remove_document_view(u64 id);
        bool has_dirty_documents() const;
        bool confirm_exit();
        void request_close(DocumentView& document, bool discard);
        void save(DocumentView& document, bool save_as);
        void undo_document(DocumentView& document);
        void redo_document(DocumentView& document);
#if defined(LUNA_PLATFORM_MACOS)
        RV install_application_menu();
        RV update_application_menu_state();
        void handle_application_menu_item(Window::application_menu_item_id_t id);
#endif
    };

    String property_text(const Variant& value)
    {
        switch(value.type())
        {
        case VariantType::string: return String(value.c_str());
        case VariantType::number:
        {
            String result;
            if(value.number_type() == VariantNumberType::number_i64)
                strprintf(result, "%lld", (long long)value.inum());
            else if(value.number_type() == VariantNumberType::number_u64)
                strprintf(result, "%llu", (unsigned long long)value.unum());
            else strprintf(result, "%.9g", value.fnum());
            return result;
        }
        default:
        {
            auto encoded = VariantUtils::write_json(value,
                VariantUtils::JSONWriteOptions::strict());
            return encoded.valid() ? encoded.get() : String("null");
        }
        }
    }

    R<Variant> property_value(const PropertyEditor& editor)
    {
        switch(editor.original.type())
        {
        case VariantType::string: return Variant(editor.text.c_str());
        case VariantType::boolean: return Variant(editor.boolean);
        case VariantType::number:
            if(editor.original.number_type() == VariantNumberType::number_i64)
            {
                c8* end = nullptr;
                i64 value = (i64)strtoll(editor.text.c_str(), &end, 10);
                if(!end || *end) return set_error(E_BAD_DATA, "Invalid signed integer property value.");
                return Variant(value);
            }
            if(editor.original.number_type() == VariantNumberType::number_u64)
            {
                c8* end = nullptr;
                u64 value = (u64)strtoull(editor.text.c_str(), &end, 10);
                if(!end || *end) return set_error(E_BAD_DATA, "Invalid unsigned integer property value.");
                return Variant(value);
            }
            else
            {
                c8* end = nullptr;
                f64 value = strtod(editor.text.c_str(), &end);
                if(!end || *end) return set_error(E_BAD_DATA, "Invalid floating-point property value.");
                return Variant(value);
            }
        default:
            return VariantUtils::read_json(editor.text.c_str(), editor.text.size(),
                VariantUtils::JSONReadOptions::strict());
        }
    }

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
        Variant metadata;
        if(!invoke(GameGUIEditor::CREATE_DOCUMENT_URL, Variant(VariantType::object), metadata))
            return false;
        DocumentView document;
        update_metadata(document, metadata);
        documents.push_back(move(document));
        selected_document = (i32)documents.size() - 1;
        return refresh_snapshot(documents.back());
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
                return refresh_snapshot(documents[i]);
            }
        }
        DocumentView document;
        update_metadata(document, metadata);
        documents.push_back(move(document));
        selected_document = (i32)documents.size() - 1;
        return refresh_snapshot(documents.back());
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
}

namespace
{
    Variant editing_params(const DocumentView& document)
    {
        Variant params(VariantType::object);
        params["document_id"] = document.id;
        params["expected_revision"] = document.revision;
        return params;
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

#if defined(LUNA_PLATFORM_MACOS)
    RV EditorApp::install_application_menu()
    {
        Window::ApplicationMenuItemDesc app_items[] =
        {
            standard_menu_command(Window::ApplicationMenuItemRole::about),
            menu_separator(),
            menu_submenu(nullptr, {}, Window::ApplicationMenuItemRole::services),
            menu_separator(),
            standard_menu_command(Window::ApplicationMenuItemRole::hide,
                KeyCode::h, Window::KeyModifierFlag::system),
            standard_menu_command(Window::ApplicationMenuItemRole::hide_others,
                KeyCode::h, Window::KeyModifierFlag::system | Window::KeyModifierFlag::alt),
            standard_menu_command(Window::ApplicationMenuItemRole::show_all),
            menu_separator(),
            standard_menu_command(Window::ApplicationMenuItemRole::quit,
                KeyCode::q, Window::KeyModifierFlag::system),
        };

        Window::ApplicationMenuItemDesc file_items[] =
        {
            menu_command("New", MENU_ITEM_NEW, KeyCode::n, Window::KeyModifierFlag::system),
            menu_command("Open...", MENU_ITEM_OPEN, KeyCode::o, Window::KeyModifierFlag::system),
            menu_separator(),
            menu_command("Save", MENU_ITEM_SAVE, KeyCode::s, Window::KeyModifierFlag::system),
            menu_command("Save As...", MENU_ITEM_SAVE_AS, KeyCode::s,
                Window::KeyModifierFlag::system | Window::KeyModifierFlag::shift),
            menu_separator(),
            menu_command("Close", MENU_ITEM_CLOSE, KeyCode::w, Window::KeyModifierFlag::system),
        };
        DocumentView* document = active_document();
        bool has_document = document != nullptr;
        file_items[3].state = menu_item_state(has_document);
        file_items[4].state = menu_item_state(has_document);
        file_items[6].state = menu_item_state(has_document);

        Window::ApplicationMenuItemDesc edit_items[] =
        {
            menu_command("Undo", MENU_ITEM_UNDO, KeyCode::z, Window::KeyModifierFlag::system),
            menu_command("Redo", MENU_ITEM_REDO, KeyCode::z,
                Window::KeyModifierFlag::system | Window::KeyModifierFlag::shift),
        };
        edit_items[0].state = menu_item_state(document && document->can_undo);
        edit_items[1].state = menu_item_state(document && document->can_redo);

        Window::ApplicationMenuItemDesc main_items[] =
        {
            menu_submenu(APP_NAME, Span<const Window::ApplicationMenuItemDesc>(app_items, 9)),
            menu_submenu("File", Span<const Window::ApplicationMenuItemDesc>(file_items, 7)),
            menu_submenu("Edit", Span<const Window::ApplicationMenuItemDesc>(edit_items, 2)),
            menu_submenu(nullptr, {}, Window::ApplicationMenuItemRole::window_menu),
            menu_submenu(nullptr, {}, Window::ApplicationMenuItemRole::help_menu),
        };
        Window::ApplicationMenuDesc desc;
        desc.items = Span<const Window::ApplicationMenuItemDesc>(main_items, 5);
        lutry
        {
            luexp(Window::set_application_menu(desc));
            application_menu_has_document = has_document;
            application_menu_can_undo = document && document->can_undo;
            application_menu_can_redo = document && document->can_redo;
        }
        lucatchret;
        return ok;
    }

    RV EditorApp::update_application_menu_state()
    {
        DocumentView* document = active_document();
        bool has_document = document != nullptr;
        bool can_undo = document && document->can_undo;
        bool can_redo = document && document->can_redo;
        lutry
        {
            if(has_document != application_menu_has_document)
            {
                Window::ApplicationMenuItemState state = menu_item_state(has_document);
                luexp(Window::set_application_menu_item_state(MENU_ITEM_SAVE, state));
                luexp(Window::set_application_menu_item_state(MENU_ITEM_SAVE_AS, state));
                luexp(Window::set_application_menu_item_state(MENU_ITEM_CLOSE, state));
                application_menu_has_document = has_document;
            }
            if(can_undo != application_menu_can_undo)
            {
                luexp(Window::set_application_menu_item_state(MENU_ITEM_UNDO,
                    menu_item_state(can_undo)));
                application_menu_can_undo = can_undo;
            }
            if(can_redo != application_menu_can_redo)
            {
                luexp(Window::set_application_menu_item_state(MENU_ITEM_REDO,
                    menu_item_state(can_redo)));
                application_menu_can_redo = can_redo;
            }
        }
        lucatchret;
        return ok;
    }

    void EditorApp::handle_application_menu_item(Window::application_menu_item_id_t id)
    {
        switch(id)
        {
        case MENU_ITEM_NEW:
            create_document();
            break;
        case MENU_ITEM_OPEN:
            open_document();
            break;
        case MENU_ITEM_SAVE:
        {
            DocumentView* document = active_document();
            if(document) save(*document, false);
            break;
        }
        case MENU_ITEM_SAVE_AS:
        {
            DocumentView* document = active_document();
            if(document) save(*document, true);
            break;
        }
        case MENU_ITEM_CLOSE:
        {
            DocumentView* document = active_document();
            if(document) request_close(*document, false);
            break;
        }
        case MENU_ITEM_UNDO:
        {
            DocumentView* document = active_document();
            if(document) undo_document(*document);
            break;
        }
        case MENU_ITEM_REDO:
        {
            DocumentView* document = active_document();
            if(document) redo_document(*document);
            break;
        }
        default:
            break;
        }
    }
#endif

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
                if(selected_document >= (i32)documents.size())
                    selected_document = max((i32)documents.size() - 1, 0);
                break;
            }
        }
    }

    void EditorApp::request_close(DocumentView& document, bool discard)
    {
        Variant params = editing_params(document);
        params["discard"] = discard;
        Variant result;
        if(invoke(GameGUIEditor::CLOSE_DOCUMENT_URL, params, result))
        {
            u64 id = document.id;
            pending_close_document = 0;
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
        else if(document.dirty && !discard)
        {
            pending_close_document = document.id;
        }
    }

    void EditorApp::apply_inspector_changes(DocumentView& document)
    {
        if(!document.snapshot) return;
        const GameGUI::NodeRecord* node = GameGUI::find_node(*document.snapshot,
            document.selected_node);
        if(!node) return;
        Variant commands(VariantType::array);
        if(Name(document.node_name.c_str()) != node->name)
        {
            Variant command(VariantType::object);
            command["kind"] = "set_name";
            command["node"] = guid_string(node->id).c_str();
            command["name"] = document.node_name.c_str();
            commands.push_back(move(command));
        }
        for(const PropertyEditor& editor : document.property_editors)
        {
            auto value = property_value(editor);
            if(!value.valid())
            {
                error_message = explain(value.errcode());
                continue;
            }
            if(value.get() == editor.original) continue;
            Variant command(VariantType::object);
            command["kind"] = "set_property";
            command["node"] = guid_string(node->id).c_str();
            command["property"] = editor.key;
            command["value"] = value.get();
            commands.push_back(move(command));
        }
        if(commands.empty()) return;
        Variant params = editing_params(document);
        params["commands"] = move(commands);
        params["label"] = "Edit node properties";
        String coalesce = guid_string(node->id);
        coalesce.append(".inspector");
        params["coalesce_key"] = coalesce.c_str();
        Variant metadata;
        if(invoke(GameGUIEditor::APPLY_COMMANDS_URL, params, metadata))
            refresh_snapshot(document);
    }

    void EditorApp::process_interactions(const UIHandles& handles)
    {
        if(EditorGUI::is_item_clicked(gui, handles.create_document)) create_document();
        if(EditorGUI::is_item_clicked(gui, handles.open_document)) open_document();

        DocumentView* document = find_document(handles.document_id);
        if(!document) document = active_document();
        if(document)
        {
            apply_inspector_changes(*document);
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
            if(EditorGUI::is_item_clicked(gui, handles.undo)) undo_document(*document);
            if(EditorGUI::is_item_clicked(gui, handles.redo)) redo_document(*document);
            if(EditorGUI::is_item_clicked(gui, handles.close_document))
                request_close(*document, false);

            if(EditorGUI::is_item_clicked(gui, handles.delete_node) &&
                document->snapshot && document->selected_node != document->snapshot->root)
            {
                Variant command(VariantType::object);
                command["kind"] = "remove_node";
                command["node"] = guid_string(document->selected_node).c_str();
                Variant params = editing_params(*document);
                params["commands"] = Variant(VariantType::array);
                params["commands"].push_back(move(command));
                params["label"] = "Delete node";
                Variant result;
                if(invoke(GameGUIEditor::APPLY_COMMANDS_URL, params, result))
                    refresh_snapshot(*document);
            }

            for(const TypeHit& hit : handles.types)
            {
                if(!EditorGUI::is_item_clicked(gui, hit.element)) continue;
                Variant command(VariantType::object);
                command["kind"] = "insert_node";
                command["parent"] = guid_string(document->selected_node).c_str();
                command["type"] = guid_string(hit.type).c_str();
                Variant params = editing_params(*document);
                params["commands"] = Variant(VariantType::array);
                params["commands"].push_back(move(command));
                params["label"] = "Add node";
                Variant result;
                if(invoke(GameGUIEditor::APPLY_COMMANDS_URL, params, result))
                {
                    if(!result["created_nodes"].empty())
                        decode_guid_string(result["created_nodes"][0], document->selected_node);
                    refresh_snapshot(*document);
                }
                break;
            }

            if(EditorGUI::is_item_clicked(gui, handles.add_property) &&
                !document->new_property_name.empty())
            {
                auto property = VariantUtils::read_json(document->new_property_value.c_str(),
                    document->new_property_value.size(), VariantUtils::JSONReadOptions::strict());
                if(!property.valid()) error_message = explain(property.errcode());
                else
                {
                    Variant command(VariantType::object);
                    command["kind"] = "set_property";
                    command["node"] = guid_string(document->selected_node).c_str();
                    command["property"] = document->new_property_name.c_str();
                    command["value"] = property.get();
                    Variant params = editing_params(*document);
                    params["commands"] = Variant(VariantType::array);
                    params["commands"].push_back(move(command));
                    params["label"] = "Add property";
                    Variant result;
                    if(invoke(GameGUIEditor::APPLY_COMMANDS_URL, params, result))
                    {
                        document->new_property_name.clear();
                        document->new_property_value = "null";
                        refresh_snapshot(*document);
                    }
                }
            }
        }

        if(pending_close_document)
        {
            DocumentView* closing = find_document(pending_close_document);
            if(!closing) pending_close_document = 0;
            else if(EditorGUI::is_item_clicked(gui, handles.close_save))
            {
                save(*closing, closing->asset_path.empty());
                if(!closing->dirty) request_close(*closing, false);
            }
            else if(EditorGUI::is_item_clicked(gui, handles.close_discard))
                request_close(*closing, true);
            else if(EditorGUI::is_item_clicked(gui, handles.close_cancel))
                pending_close_document = 0;
        }
    }

    RV EditorApp::run()
    {
        lutry
        {
            i32 frame_index = 0;
            while(true)
            {
                Window::poll_events();
#if defined(LUNA_PLATFORM_MACOS)
                if(Window::is_application_quit_requested()) break;
                luexp(update_application_menu_state());
#endif
                if(window->is_closed()) break;
                if(window->is_minimized())
                {
                    sleep(100);
                    continue;
                }
                UInt2U framebuffer_size = window->get_framebuffer_size();
                if(framebuffer_size.x && framebuffer_size.y &&
                    (framebuffer_size.x != width || framebuffer_size.y != height))
                {
                    luexp(swap_chain->reset({framebuffer_size.x, framebuffer_size.y, 2,
                        RHI::Format::unknown, true}));
                    luexp(resize_target(framebuffer_size));
                    width = framebuffer_size.x;
                    height = framebuffer_size.y;
                }
                UInt2U logical_size = window->get_size();
                GUI::FrameDesc frame;
                frame.logical_size = Float2U((f32)logical_size.x, (f32)logical_size.y);
                frame.render_size = framebuffer_size;
                frame.delta_time = 1.0f / 60.0f;
                gui->begin_frame(frame);
                GUIWindow::update_input(&input_adapter);
                gui->push_layer(gui->make_id("editor.root.layer"));
                UIHandles handles;
                GUI::ElementHandle root = build_editor(handles);
                gui->pop_layer();
                luexp(EditorGUI::layout_tree(gui, root,
                    RectF(0.0f, 0.0f, frame.logical_size.x, frame.logical_size.y)));
                gui->route_input();
                EditorGUI::ResolveResult resolved = EditorGUI::resolve_interactions(gui);
                if(resolved.relayout_requested)
                {
                    luexp(EditorGUI::layout_tree(gui, root,
                        RectF(0.0f, 0.0f, frame.logical_size.x, frame.logical_size.y)));
                }

                PreviewInput preview_input;
                preview_input.document_id = handles.document_id;
                RectF preview_rect = EditorGUI::get_item_rect(gui, handles.preview_host);
                if(handles.preview_host.id && preview_rect.width > 0.0f && preview_rect.height > 0.0f)
                {
                    for(const GUI::RoutedInputEvent& routed :
                        gui->get_routed_input_events(handles.preview_host.id))
                    {
                        GUI::InputEvent event = routed.event;
                        if(routed.has_element_position)
                        {
                            event.position.x = routed.element_position.x / preview_rect.width *
                                (f32)PREVIEW_WIDTH;
                            event.position.y = routed.element_position.y / preview_rect.height *
                                (f32)PREVIEW_HEIGHT;
                        }
                        preview_input.events.push_back(move(event));
                    }
                }
                luexp(GUIWindow::update_text_input(&input_adapter));
                luexp(gui->generate_draw_commands());

                process_interactions(handles);
                if(discard_smoke && frame_index == 0)
                {
                    DocumentView* document = active_document();
                    if(document) request_close(*document, true);
                }
                DocumentView* preview_document = find_document(preview_input.document_id);
                if(!preview_document) preview_document = active_document();
                if(preview_document)
                {
                    Span<const GUI::InputEvent> events;
                    if(preview_document->id == preview_input.document_id)
                        events = Span<const GUI::InputEvent>(preview_input.events.data(),
                            preview_input.events.size());
                    luexp(build_preview(*preview_document, events));
                }
                luexp(render_frame(preview_document));
                for(u64 id : deferred_document_removals) remove_document_view(id);
                deferred_document_removals.clear();
                ++frame_index;
                if(max_frames >= 0 && frame_index >= max_frames) break;
            }
#if defined(LUNA_PLATFORM_MACOS)
            auto _ = Window::reset_application_menu();
#endif
            GUIWindow::uninstall_window_event_handler(&input_adapter);
            Window::set_event_handler(nullptr, nullptr);
            service.reset();
            documents.clear();
            luexp(VFS::unmount("/"));
        }
        lucatchret;
        return ok;
    }
}

int luna_main(int argc, const char* argv[])
{
    i32 max_frames = -1;
    bool discard_smoke = false;
    const c8* workspace_path = nullptr;
    for(int i = 1; i < argc; ++i)
    {
        i32 parsed = 0;
        if(sscanf(argv[i], "--frames=%d", &parsed) == 1) max_frames = parsed;
        else if(!strcmp(argv[i], "--discard-smoke")) discard_smoke = true;
        else if(!strncmp(argv[i], "--workspace=", 12)) workspace_path = argv[i] + 12;
    }
    lupanic_if_failed(Luna::init());
    lupanic_if_failed(add_modules({
        module_window(),
        module_rhi(),
        module_rhi_utility(),
        module_font(),
        module_vg(),
        GUI::module_gui(),
        EditorGUI::module_editor_gui(),
        GUIWindow::module_gui_window(),
        GameGUI::module_game_gui(),
        Frontend::module_frontend()
    }));
    Window::StartupParams startup_params;
    startup_params.name = APP_NAME;
    Window::set_startup_params(startup_params);
    lupanic_if_failed(init_modules());
    {
        EditorApp app;
        app.max_frames = max_frames;
        app.discard_smoke = discard_smoke;
        if(workspace_path) app.workspace_path = workspace_path;
        lupanic_if_failed(app.init());
        lupanic_if_failed(app.run());
    }
    Luna::close();
    return 0;
}

namespace
{
    void EditorApp::build_hierarchy_node(DocumentView& document, const Guid& node_id,
        u32 depth, UIHandles& handles)
    {
        const GameGUI::NodeRecord* node = GameGUI::find_node(*document.snapshot, node_id);
        if(!node) return;
        String label;
        const c8* name = node->name.empty() ? "Unnamed Node" : node->name.c_str();
        strprintf(label, "%s", name);
        GUI::LayoutConfig layout = fill_width(26.0f);
        layout.padding.x = 8.0f + (f32)depth * 16.0f;
        GUI::ElementHandle item = EditorGUI::selectable(gui,
            guid_gui_id(gui->make_id("hierarchy.nodes"), node_id), label.c_str(),
            document.selected_node == node_id, layout);
        handles.nodes.push_back(NodeHit{node_id, item});
        for(const GameGUI::ChildLink& child : node->children)
            build_hierarchy_node(document, child.child, depth + 1, handles);
    }

    void EditorApp::build_hierarchy_panel(UIHandles& handles)
    {
        if(!EditorGUI::begin_dock_panel(gui, gui->make_id("panel.hierarchy"),
            "Hierarchy")) return;
        GUI::ElementHandle root = EditorGUI::begin_v_layout(gui,
            gui->make_id("hierarchy.root"), "Hierarchy Root", fill_layout());
        GUI::ElementHandle toolbar = EditorGUI::begin_h_layout(gui,
            gui->make_id("hierarchy.toolbar"), "Hierarchy Toolbar", fill_width(34.0f));
        EditorGUI::text(gui, gui->make_id("hierarchy.title"), "Node Tree",
            grow_width(30.0f));
        handles.delete_node = EditorGUI::text_button(gui,
            gui->make_id("hierarchy.delete"), "Delete", fixed_layout(72.0f, 28.0f));
        GUI::FlexLayoutDesc toolbar_layout;
        toolbar_layout.axis = GUI::LayoutAxis::x;
        toolbar_layout.cross_alignment = GUI::FlexAlignment::center;
        toolbar_layout.main_axis_gap = 6.0f;
        EditorGUI::end_h_layout(gui, toolbar, toolbar_layout);
        EditorGUI::ScrollViewDesc scroll_desc;
        scroll_desc.horizontal = false;
        EditorGUI::begin_scroll_view(gui, gui->make_id("hierarchy.scroll"),
            "Hierarchy Scroll", fill_layout(), scroll_desc);
        DocumentView* document = active_document();
        if(document && document->snapshot)
            build_hierarchy_node(*document, document->snapshot->root, 0, handles);
        EditorGUI::end_scroll_view(gui);
        GUI::FlexLayoutDesc root_layout;
        root_layout.main_axis_gap = 4.0f;
        EditorGUI::end_v_layout(gui, root, root_layout);
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

    void EditorApp::build_document_panel(UIHandles& handles)
    {
        if(!EditorGUI::begin_dock_panel(gui, gui->make_id("panel.document"),
            "Documents")) return;
        GUI::ElementHandle root = EditorGUI::begin_v_layout(gui,
            gui->make_id("documents.root"), "Documents Root", fill_layout());

        if(pending_close_document)
        {
            GUI::ElementHandle row = EditorGUI::begin_h_layout(gui,
                gui->make_id("close.prompt"), "Close Prompt", fill_width(38.0f));
            EditorGUI::text(gui, gui->make_id("close.prompt.text"),
                "This document has unsaved changes.", grow_width(30.0f));
            handles.close_save = EditorGUI::text_button(gui,
                gui->make_id("close.save"), "Save", fixed_layout(60.0f, 30.0f));
            handles.close_discard = EditorGUI::text_button(gui,
                gui->make_id("close.discard"), "Discard", fixed_layout(72.0f, 30.0f));
            handles.close_cancel = EditorGUI::text_button(gui,
                gui->make_id("close.cancel"), "Cancel", fixed_layout(64.0f, 30.0f));
            GUI::FlexLayoutDesc row_layout;
            row_layout.axis = GUI::LayoutAxis::x;
            row_layout.cross_alignment = GUI::FlexAlignment::center;
            row_layout.main_axis_gap = 6.0f;
            EditorGUI::end_h_layout(gui, row, row_layout);
        }

        EditorGUI::TabBarDesc tab_desc;
        tab_desc.fitting_mode = EditorGUI::TabBarFittingMode::shrink;
        EditorGUI::begin_tab_bar(gui, gui->make_id("documents.tabs"), &selected_document,
            fill_layout(), tab_desc);
        for(usize i = 0; i < documents.size(); ++i)
        {
            DocumentView& document = documents[i];
            String label = document.title;
            if(document.dirty) label.append(" *");
            if(EditorGUI::begin_tab_item(gui,
                GUI::make_scoped_id(gui->make_id("documents.tabs.items"), document.id),
                label.c_str()))
            {
                handles.document_id = document.id;
                RV preview_result = ensure_preview(document);
                if(failed(preview_result)) error_message = explain(preview_result.errcode());
                handles.preview_host = EditorGUI::begin_button(gui,
                    GUI::make_scoped_id(gui->make_id("documents.preview.host"), document.id),
                    "Interactive GameGUI Preview", fill_layout());
                EditorGUI::ImageDesc image;
                image.flags = EditorGUI::ImageFlag::flip_y;
                EditorGUI::image(gui,
                    GUI::make_scoped_id(gui->make_id("documents.preview.image"), document.id),
                    document.preview.target, fill_layout(), image);
                EditorGUI::end_button(gui);
                EditorGUI::end_tab_item(gui);
            }
        }
        EditorGUI::end_tab_bar(gui);
        GUI::FlexLayoutDesc root_layout;
        root_layout.main_axis_gap = 4.0f;
        EditorGUI::end_v_layout(gui, root, root_layout);
        EditorGUI::end_dock_panel(gui);
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
            EditorGUI::menu_separator(gui, gui->make_id("main_menu.file.close_separator"));
            EditorGUI::MenuItemDesc close_desc;
            close_desc.enabled = document != nullptr;
            handles.close_document = EditorGUI::menu_item(gui,
                gui->make_id("main_menu.file.close"), "Close", false, close_desc);
            lupanic_if_failed(EditorGUI::end_menu(gui,
                RectF(0.0f, 0.0f, 230.0f, 180.0f)));
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
        const GameGUI::NodeRecord* node = GameGUI::find_node(*document->snapshot,
            document->selected_node);
        if(node)
        {
            String type_name = "Unsupported type";
            Variant schema;
            for(const NodeTypeView& type : node_types)
            {
                if(type.type == node->type)
                {
                    type_name = type.display_name;
                    schema = type.schema;
                    break;
                }
            }
            EditorGUI::text(gui, gui->make_id("inspector.type"), type_name.c_str(),
                fill_width(30.0f));
            String schema_text;
            strprintf(schema_text, "Schema: %s", schema["kind"].c_str("unknown"));
            EditorGUI::text(gui, gui->make_id("inspector.schema"), schema_text.c_str(),
                fill_width(26.0f));
            EditorGUI::input_text(gui, gui->make_id("inspector.node_name"),
                document->node_name, fill_width(30.0f));
            EditorGUI::ScrollViewDesc scroll_desc;
            scroll_desc.horizontal = false;
            EditorGUI::begin_scroll_view(gui, gui->make_id("inspector.properties.scroll"),
                "Property Inspector", fill_layout(), scroll_desc);
            GUI::id_t property_scope = guid_gui_id(gui->make_id("inspector.properties"),
                document->selected_node);
            for(PropertyEditor& property : document->property_editors)
            {
                GUI::id_t property_id = GUI::make_scoped_id(property_scope,
                    property.key.c_str());
                GUI::ElementHandle row = EditorGUI::begin_v_layout(gui, property_id,
                    property.key.c_str(), fill_width(56.0f));
                EditorGUI::text(gui, GUI::make_scoped_id(property_id, "label"),
                    property.key.c_str(), fill_width(22.0f));
                if(property.original.type() == VariantType::boolean)
                {
                    EditorGUI::checkbox(gui, GUI::make_scoped_id(property_id, "value"),
                        "Enabled", &property.boolean, fill_width(28.0f));
                }
                else
                {
                    EditorGUI::input_text(gui, GUI::make_scoped_id(property_id, "value"),
                        property.text, fill_width(28.0f));
                }
                EditorGUI::end_v_layout(gui, row);
            }
            GUI::ElementHandle add_row = EditorGUI::begin_v_layout(gui,
                gui->make_id("inspector.add_property.row"), "Add Property", fill_width(86.0f));
            EditorGUI::input_text(gui, gui->make_id("inspector.add_property.name"),
                document->new_property_name, fill_width(26.0f));
            EditorGUI::input_text(gui, gui->make_id("inspector.add_property.value"),
                document->new_property_value, fill_width(26.0f));
            handles.add_property = EditorGUI::text_button(gui,
                gui->make_id("inspector.add_property.button"), "Add JSON Property",
                fill_width(28.0f));
            EditorGUI::end_v_layout(gui, add_row);
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
        build_hierarchy_panel(handles);
        build_palette_panel(handles);
        build_document_panel(handles);
        build_inspector_panel(handles);
        build_diagnostics_panel();
        EditorGUI::end_dock_space(gui);
        EditorGUI::end_v_layout(gui, root);
        return root;
    }
}

namespace
{
    RV EditorApp::resize_target(const UInt2U& size)
    {
        lutry
        {
            luset(gui_target, RHI::get_main_device()->new_texture(
                RHI::MemoryType::local, RHI::TextureDesc::tex2d(
                    swap_chain->get_desc().format,
                    RHI::TextureUsageFlag::color_attachment |
                        RHI::TextureUsageFlag::read_texture,
                    size.x, size.y, 1, 1)));
        }
        lucatchret;
        return ok;
    }

    RV EditorApp::initialize_dock_layout()
    {
        EditorGUI::DockSpaceLayoutDesc layout;
        layout.nodes.resize(9);
        layout.root_node = 0;
        layout.nodes[0].split = true;
        layout.nodes[0].split_axis = EditorGUI::DockSplitAxis::y;
        layout.nodes[0].split_ratio = 0.10f;
        layout.nodes[0].child0 = 1;
        layout.nodes[0].child1 = 2;
        layout.nodes[1].tabs.push_back(gui->make_id("panel.palette"));

        layout.nodes[2].split = true;
        layout.nodes[2].split_axis = EditorGUI::DockSplitAxis::x;
        layout.nodes[2].split_ratio = 0.22f;
        layout.nodes[2].child0 = 3;
        layout.nodes[2].child1 = 4;
        layout.nodes[3].tabs.push_back(gui->make_id("panel.hierarchy"));
        layout.nodes[4].split = true;
        layout.nodes[4].split_axis = EditorGUI::DockSplitAxis::x;
        layout.nodes[4].split_ratio = 0.76f;
        layout.nodes[4].child0 = 5;
        layout.nodes[4].child1 = 8;
        layout.nodes[5].split = true;
        layout.nodes[5].split_axis = EditorGUI::DockSplitAxis::y;
        layout.nodes[5].split_ratio = 0.78f;
        layout.nodes[5].child0 = 6;
        layout.nodes[5].child1 = 7;
        layout.nodes[6].tabs.push_back(gui->make_id("panel.document"));
        layout.nodes[7].tabs.push_back(gui->make_id("panel.diagnostics"));
        layout.nodes[8].tabs.push_back(gui->make_id("panel.inspector"));
        EditorGUI::set_dockspace_layout(gui, gui->make_id("editor.dock_space"), layout);
        return ok;
    }

    bool EditorApp::has_dirty_documents() const
    {
        for(const DocumentView& document : documents)
        {
            if(document.dirty) return true;
        }
        return false;
    }

    RV EditorApp::init()
    {
        lutry
        {
            Path current_dir;
            if(workspace_path.empty())
            {
                const c8* process_path = get_process_path();
                current_dir = process_path;
                release_process_path(process_path);
                current_dir.pop_back();
            }
            else current_dir = workspace_path.c_str();
            luexp(set_current_dir(current_dir.encode().c_str()));
            const c8* resolved_current_dir = get_current_dir();
            workspace_root = resolved_current_dir;
            release_current_dir(resolved_current_dir);
            luexp(VFS::mount(VFS::get_platform_filesystem_driver(),
                workspace_root.encode(PathSeparator::system_preferred).c_str(), "/"));
            luexp(Asset::load_assets_meta("/", true));

            luset(window, Window::new_window(APP_NAME,
                Window::DEFAULT_POS, Window::DEFAULT_POS, 1440, 960));
            luexp(window->set_foreground());
            RHI::IDevice* device = RHI::get_main_device();
            for(u32 i = 0; i < device->get_num_command_queues(); ++i)
            {
                if(device->get_command_queue_desc(i).type == RHI::CommandQueueType::graphics)
                {
                    queue = i;
                    break;
                }
            }
            if(queue == U32_MAX) luthrow(set_error(E_NOT_SUPPORTED,
                "No graphics command queue is available."));
            UInt2U size = window->get_framebuffer_size();
            luset(swap_chain, device->new_swap_chain(queue, window,
                RHI::SwapChainDesc({size.x, size.y, 2, RHI::Format::bgra8_unorm,
                    true, RHI::ColorSpace::srgb})));
            luset(cmdbuf, device->new_command_buffer(queue));
            luset(blit, RHIUtility::new_blit_context(device, swap_chain->get_desc().format));
            luset(renderer, GUI::new_renderer(device));
            gui = GUI::new_context();
            luexp(gui->register_font("default", Font::get_default_font()));
            EditorGUI::register_style_schemas(gui);
            EditorGUI::DefaultStyleDesc style;
            style.input_mode = EditorGUI::InputMode::pointer;
            style.color_theme = EditorGUI::ColorTheme::dark;
            EditorGUI::set_default_style(gui, style);
            luexp(resize_target(size));
            width = size.x;
            height = size.y;
            luexp(initialize_dock_layout());

            lulet(created_service, GameGUIEditor::new_service());
            service = move(created_service);
            Variant types;
            if(!invoke(GameGUIEditor::GET_NODE_TYPES_URL,
                Variant(VariantType::object), types))
            {
                luthrow(E_FAILURE);
            }
            for(const Variant& value : types.values())
            {
                NodeTypeView type;
                if(!decode_guid_string(value["type"], type.type)) continue;
                type.name = value["name"].c_str();
                type.display_name = value["display_name"].c_str();
                type.category = value["category"].c_str();
                type.schema = value["property_schema"];
                node_types.push_back(move(type));
            }
            if(!create_document()) luthrow(E_FAILURE);

            Window::set_event_handler([](object_t event, void* userdata)
            {
                EditorApp* app = (EditorApp*)userdata;
#if defined(LUNA_PLATFORM_MACOS)
                if(auto menu_item = cast_object<Window::ApplicationMenuItemInvokedEvent>(event))
                {
                    app->handle_application_menu_item(menu_item->item_id);
                }
                if(auto quit = cast_object<Window::ApplicationRequestQuitEvent>(event))
                {
                    quit->do_quit = app->confirm_exit();
                }
#endif
                if(auto close = cast_object<Window::WindowRequestCloseEvent>(event))
                {
                    if(close->window == app->window) close->do_close = app->confirm_exit();
                }
            }, this);
            input_adapter.window = window;
            input_adapter.gui = gui;
            GUIWindow::install_window_event_handler(&input_adapter);
#if defined(LUNA_PLATFORM_MACOS)
            luexp(install_application_menu());
#endif
        }
        lucatchret;
        return ok;
    }

    RV EditorApp::ensure_preview(DocumentView& document)
    {
        lutry
        {
            if(!document.preview.context)
            {
                document.preview.context = GUI::new_context();
                luexp(document.preview.context->register_font("default",
                    Font::get_default_font()));
            }
            if(!document.preview.target)
            {
                luset(document.preview.target, RHI::get_main_device()->new_texture(
                    RHI::MemoryType::local, RHI::TextureDesc::tex2d(
                        RHI::Format::rgba8_unorm,
                        RHI::TextureUsageFlag::color_attachment |
                            RHI::TextureUsageFlag::read_texture,
                        PREVIEW_WIDTH, PREVIEW_HEIGHT, 1, 1)));
            }
            if(!document.preview.instance || document.preview.revision != document.revision)
            {
                GameGUI::InstanceDesc desc;
                desc.document = document.snapshot;
                desc.instance_scope = GUI::make_scoped_id(GUI::DEFAULT_DATA_SCOPE,
                    document.id);
                if(document.asset_guid != Guid())
                    desc.source_asset = Asset::get_asset(document.asset_guid);
                document.preview.instance = GameGUI::new_instance(desc);
                RV prepared = document.preview.instance->prepare();
                if(failed(prepared)) error_message = explain(prepared.errcode());
                document.preview.revision = document.revision;
            }
        }
        lucatchret;
        return ok;
    }

    RV EditorApp::build_preview(DocumentView& document,
        Span<const GUI::InputEvent> input)
    {
        lutry
        {
            luexp(ensure_preview(document));
            GUI::FrameDesc frame;
            frame.logical_size = Float2U((f32)PREVIEW_WIDTH, (f32)PREVIEW_HEIGHT);
            frame.render_size = UInt2U(PREVIEW_WIDTH, PREVIEW_HEIGHT);
            frame.delta_time = 1.0f / 60.0f;
            document.preview.context->begin_frame(frame);
            document.preview.context->add_input_events(input);
            document.preview.context->push_layer(
                document.preview.context->make_id("preview.root.layer"));
            lulet(root, document.preview.instance->build(document.preview.context));
            document.preview.context->pop_layer();
            if(root.id)
            {
                luexp(document.preview.context->apply_layout(root,
                    RectF(0.0f, 0.0f, (f32)PREVIEW_WIDTH, (f32)PREVIEW_HEIGHT)));
            }
            document.preview.context->route_input();
            luexp(document.preview.instance->resolve_interactions(
                document.preview.context));
            if(root.id && document.preview.instance->relayout_requested())
            {
                luexp(document.preview.context->apply_layout(root,
                    RectF(0.0f, 0.0f, (f32)PREVIEW_WIDTH, (f32)PREVIEW_HEIGHT)));
            }
            luexp(document.preview.context->generate_draw_commands());
        }
        lucatchret;
        return ok;
    }

    RV EditorApp::render_frame(DocumentView* preview_document)
    {
        lutry
        {
            if(preview_document && preview_document->preview.target &&
                preview_document->preview.context)
            {
                GUI::RenderTargetDesc preview_target(preview_document->preview.target);
                preview_target.color_load_op = RHI::LoadOp::clear;
                preview_target.color_clear_value = Float4U(0.055f, 0.065f, 0.08f, 1.0f);
                preview_target.color_final_state = RHI::TextureStateFlag::shader_read_ps;
                luexp(renderer->render(preview_document->preview.context, cmdbuf,
                    preview_target));
            }
            GUI::RenderTargetDesc editor_target(gui_target);
            editor_target.color_load_op = RHI::LoadOp::clear;
            editor_target.color_clear_value = Float4U(0.06f, 0.07f, 0.09f, 1.0f);
            editor_target.color_final_state = RHI::TextureStateFlag::shader_read_ps;
            luexp(renderer->render(gui, cmdbuf, editor_target));
            lulet(back_buffer, swap_chain->get_current_back_buffer());
            blit->reset();
            RHI::SamplerDesc sampler(RHI::Filter::linear, RHI::Filter::linear,
                RHI::Filter::nearest, RHI::TextureAddressMode::clamp,
                RHI::TextureAddressMode::clamp, RHI::TextureAddressMode::clamp);
            blit->blit(back_buffer, RHI::SubresourceIndex(0, 0),
                RHI::TextureViewDesc::tex2d(gui_target), sampler,
                Float2U(0.0f), Float2U((f32)width, 0.0f),
                Float2U(0.0f, (f32)height), Float2U((f32)width, (f32)height));
            luexp(blit->commit(cmdbuf, false));
            cmdbuf->resource_barrier({}, {
                {back_buffer, RHI::TEXTURE_BARRIER_ALL_SUBRESOURCES,
                    RHI::TextureStateFlag::automatic,
                    RHI::TextureStateFlag::present,
                    RHI::ResourceBarrierFlag::none}
            });
            luexp(cmdbuf->submit({}, {}, true));
            cmdbuf->wait();
            luexp(cmdbuf->reset());
            luexp(swap_chain->present());
        }
        lucatchret;
        return ok;
    }
}
