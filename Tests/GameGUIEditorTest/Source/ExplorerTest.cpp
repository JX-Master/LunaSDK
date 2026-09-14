/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file ExplorerTest.cpp
* @author JXMaster
* @date 2026/9/11
*/
#include "../../../Programs/GameGUIEditor/Source/EditorApp.hpp"
#include <Luna/Font/Font.hpp>
#include <Luna/Runtime/File.hpp>
#include <Luna/Runtime/Random.hpp>

using namespace Luna;
using namespace Luna::GameGUIEditor;
using namespace Luna::GameGUIEditor::Internal;

void explorer_test()
{
    EditorApp app;
    app.gui = GUI::new_context();
    lupanic_if_failed(app.gui->register_font("default", Font::get_default_font()));
    EditorGUI::register_style_schemas(app.gui);
    EditorGUI::set_default_style(app.gui, EditorGUI::DefaultStyleDesc());
    auto created = new_service();
    lupanic_if_failed(created);
    app.service = move(created.get());
    const c8* current = get_current_dir();
    Path fixture(current);
    release_current_dir(current);
    c8 unique[GUID_STRING_LENGTH + 1] = {};
    lupanic_if_failed(encode_guid(random_guid(), unique, GUID_STRING_LENGTH));
    fixture.push_back(unique);
    lupanic_if_failed(create_dir(fixture.encode().c_str()));
    luassert_always(app.open_working_directory(fixture));
    Path folder = fixture;
    folder.push_back("Folder");
    lupanic_if_failed(create_dir(folder.encode().c_str()));
    luassert_always(app.create_document());
    Path menu(app.working_directories[0]["vfs_path"].c_str());
    menu.push_back("Menu");
    Variant params = editing_params(*app.active_document()), metadata;
    params["path"] = menu.encode().c_str();
    luassert_always(app.invoke(SAVE_AS_URL, params, metadata));
    app.update_metadata(*app.active_document(), metadata);
    Guid menu_guid = app.active_document()->asset_guid;
    u64 document_id = app.active_document()->id;
    luassert_always(app.invoke(CLOSE_DOCUMENT_URL, editing_params(*app.active_document()), metadata));
    app.remove_document_view(document_id);
    Path foreign(app.working_directories[0]["vfs_path"].c_str());
    foreign.push_back("NotGameGUI");
    auto foreign_asset = Asset::new_asset(foreign, "Test.Explorer.Foreign", true);
    lupanic_if_failed(foreign_asset);
    Path cooked(app.working_directories[0]["vfs_path"].c_str());
    cooked.push_back("CookedOnly");
    auto cooked_asset = Asset::new_asset(cooked, GameGUI::get_asset_type(), true);
    lupanic_if_failed(cooked_asset);
    params = Variant(VariantType::object);
    params["working_directory_id"] = app.selected_directory;
    luassert_always(app.invoke(REFRESH_DIRECTORY_URL, params, metadata));
    luassert_always(app.refresh_working_directories());

    EditorGUI::DockSpaceLayoutDesc layout;
    layout.root_node = 0;
    layout.nodes.resize(1);
    layout.nodes[0].tabs.push_back(app.gui->make_id("panel.explorer"));
    EditorGUI::set_dockspace_layout(app.gui, app.gui->make_id("editor.dock_space"), layout);
    UIHandles handles;
    auto frame = [&](Span<const GUI::InputEvent> input = {})
    {
        if(app.active_document()) app.rebuild_inspector(*app.active_document());
        GUI::FrameDesc desc;
        desc.logical_size = Float2U(500.0f, 600.0f);
        desc.render_size = UInt2U(1000, 1200);
        desc.delta_time = 1.0f / 60.0f;
        app.gui->begin_frame(desc);
        app.gui->add_input_events(input);
        app.gui->push_layer(app.gui->make_id("explorer.test.layer"));
        handles = UIHandles();
        GUI::ElementHandle root = EditorGUI::begin_v_layout(app.gui,
            app.gui->make_id("explorer.test.root"), "Explorer Test", fill_layout());
        EditorGUI::begin_dock_space(app.gui, app.gui->make_id("editor.dock_space"), "Explorer", fill_layout());
        app.build_explorer_panel(handles);
        EditorGUI::end_dock_space(app.gui);
        app.build_asset_picker(handles);
        EditorGUI::end_v_layout(app.gui, root);
        app.gui->pop_layer();
        lupanic_if_failed(EditorGUI::layout_tree(app.gui, root, RectF(0, 0, 500, 600)));
        app.gui->route_input();
        if(EditorGUI::resolve_interactions(app.gui).relayout_requested)
            lupanic_if_failed(EditorGUI::layout_tree(app.gui, root, RectF(0, 0, 500, 600)));
        lupanic_if_failed(app.gui->generate_draw_commands());
        app.process_interactions(handles);
        luassert_always(app.error_message.empty());
    };
    frame();
    // Root, empty folder and two GameGUI assets; no foreign asset or physical data-unit files.
    luassert_always(handles.explorer.size() == 4);
    Float2U menu_position, root_position;
    for(const auto& hit : handles.explorer)
    {
        RectF rect = item_screen_rect(app.gui, hit.element);
        Float2U position(rect.offset_x + rect.width * 0.6f, rect.offset_y + rect.height * 0.5f);
        if(hit.asset == menu_guid) menu_position = position;
        if(hit.asset == Guid() && hit.path.empty()) root_position = position;
        luassert_always(hit.path != Path("NotGameGUI"));
    }
    auto event = [](GUI::InputEventType type, const Float2U& at, GUI::PointerButton button = GUI::PointerButton::left)
    {
        GUI::InputEvent result;
        result.type = type;
        result.position = at;
        result.button = button;
        return result;
    };
    frame({event(GUI::InputEventType::pointer_down, menu_position)});
    frame({event(GUI::InputEventType::pointer_up, menu_position)});
    luassert_always(app.documents.empty());
    frame({event(GUI::InputEventType::pointer_down, menu_position)});
    frame({event(GUI::InputEventType::pointer_up, menu_position)});
    luassert_always(app.documents.size() == 1 && app.active_document()->asset_guid == menu_guid);
    // The Inspector's asset picker uses the same catalog and filters by the property's type.
    auto authoring_type = get_authoring_node_type(GameGUI::get_asset_instance_node_type());
    lupanic_if_failed(authoring_type);
    NodeTypeView type;
    type.type = authoring_type.get().type;
    type.property_schema = authoring_type.get().property_schema;
    app.node_types.push_back(move(type));
    app.add_node(*app.active_document(), GameGUI::get_asset_instance_node_type(), app.active_document()->snapshot->root);
    app.rebuild_inspector(*app.active_document());
    app.asset_picker_document = app.active_document()->id;
    app.asset_picker_revision = app.active_document()->revision;
    for(usize i = 0; i < app.active_document()->property_editors.size(); ++i)
        if(app.active_document()->property_editors[i].desc.id == Name("asset")) app.asset_picker_property = i;
    app.asset_picker_position = Float2U(50.0f, 200.0f);
    EditorGUI::open_popup(app.gui, app.gui->make_id("inspector.asset_picker"));
    frame();
    luassert_always(handles.asset_choices.size() == 2);
    Float2U choice_position;
    for(const auto& choice : handles.asset_choices)
    {
        if(choice.asset != menu_guid) continue;
        RectF rect = item_screen_rect(app.gui, choice.element);
        choice_position = Float2U(rect.offset_x + rect.width * 0.5f, rect.offset_y + rect.height * 0.5f);
    }
    frame({event(GUI::InputEventType::pointer_down, choice_position)});
    frame({event(GUI::InputEventType::pointer_up, choice_position)});
    auto selected = find_authoring_node(*app.active_document()->snapshot, app.active_document()->selected_node);
    luassert_always(selected && selected->properties["asset"].str() == Name(guid_string(menu_guid)));
    app.undo_document(*app.active_document());
    app.undo_document(*app.active_document());
    luassert_always(!app.active_document()->dirty);
    frame();
    frame({event(GUI::InputEventType::pointer_down, root_position, GUI::PointerButton::right)});
    frame({event(GUI::InputEventType::pointer_up, root_position, GUI::PointerButton::right)});
    frame();
    luassert_always(EditorGUI::is_popup_open(app.gui, app.gui->make_id("explorer.context")));
    luassert_always(handles.unload_directory.id && handles.refresh_directory.id);
    // Close a clean directory through the actual context menu. Removal is deferred past the frame.
    RectF unload_rect = item_screen_rect(app.gui, handles.unload_directory);
    Float2U unload_position(unload_rect.offset_x + unload_rect.width * 0.5f,
        unload_rect.offset_y + unload_rect.height * 0.5f);
    frame({event(GUI::InputEventType::pointer_down, unload_position)});
    frame({event(GUI::InputEventType::pointer_up, unload_position)});
    luassert_always(app.deferred_directory_closes.size() == 1 && app.documents.size() == 1);
    app.process_deferred_directory_closes();
    luassert_always(app.working_directories.empty() && app.documents.empty());
    frame();
    luassert_always(handles.open_directory_button.id);
    app.service.reset();
    for(const c8* name : {"Menu.json", "Menu.meta", "NotGameGUI.meta", "CookedOnly.meta", "Folder"})
    {
        Path path = fixture;
        path.push_back(name);
        lupanic_if_failed(delete_file(path.encode().c_str()));
    }
    lupanic_if_failed(delete_file(fixture.encode().c_str()));
}
