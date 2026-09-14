/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Main.cpp
* @author JXMaster
* @date 2026/9/10
*/
#include "../../../Programs/GameGUIEditor/Source/EditorApp.hpp"
#include <Luna/Font/Font.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Random.hpp>
#include <Luna/Runtime/File.hpp>

using namespace Luna;
using namespace Luna::GameGUIEditor;
using namespace Luna::GameGUIEditor::Internal;

namespace
{
    GUI::InputEvent pointer(GUI::InputEventType type, const Float2U& position)
    {
        GUI::InputEvent event;
        event.type = type;
        event.position = position;
        event.button = GUI::PointerButton::left;
        return event;
    }

    struct HierarchyTest
    {
        EditorApp app;
        UIHandles handles;
        Path fixture;

        HierarchyTest()
        {
            app.gui = GUI::new_context();
            lupanic_if_failed(app.gui->register_font("default", Font::get_default_font()));
            EditorGUI::register_style_schemas(app.gui);
            EditorGUI::set_default_style(app.gui, EditorGUI::DefaultStyleDesc());
            auto service = new_service();
            lupanic_if_failed(service);
            app.service = move(service.get());
            const c8* current = get_current_dir();
            fixture = current;
            release_current_dir(current);
            c8 name[GUID_STRING_LENGTH + 1] = {};
            lupanic_if_failed(encode_guid(random_guid(), name, GUID_STRING_LENGTH));
            fixture.push_back(name);
            lupanic_if_failed(create_dir(fixture.encode().c_str()));
            luassert_always(app.open_working_directory(fixture));
            Variant types;
            luassert_always(app.invoke(GET_NODE_TYPES_URL, Variant(VariantType::object), types));
            for(const Variant& value : types.values())
            {
                NodeTypeView type;
                luassert_always(decode_guid_string(value["type"], type.type));
                type.name = value["name"].c_str();
                type.display_name = value["display_name"].c_str();
                type.category = value["category"].c_str();
                app.node_types.push_back(move(type));
            }
            luassert_always(app.create_document());
            EditorGUI::DockSpaceLayoutDesc layout;
            layout.root_node = 0;
            layout.nodes.resize(3);
            layout.nodes[0].split = true;
            layout.nodes[0].split_axis = EditorGUI::DockSplitAxis::y;
            layout.nodes[0].split_ratio = 0.25f;
            layout.nodes[0].child0 = 1;
            layout.nodes[0].child1 = 2;
            layout.nodes[1].tabs.push_back(app.gui->make_id("panel.palette"));
            layout.nodes[2].tabs.push_back(app.gui->make_id("panel.hierarchy"));
            EditorGUI::set_dockspace_layout(app.gui, app.gui->make_id("editor.dock_space"), layout);
            frame();
        }

        DocumentView& document() { return *app.active_document(); }

        ~HierarchyTest()
        {
            app.documents.clear();
            app.service.reset();
            lupanic_if_failed(delete_file(fixture.encode().c_str()));
        }

        void frame(Span<const GUI::InputEvent> events = {})
        {
            app.rebuild_inspector(document());
            GUI::FrameDesc desc;
            desc.logical_size = Float2U(640.0f, 640.0f);
            desc.render_size = UInt2U(1280, 1280);
            app.gui->begin_frame(desc);
            app.gui->add_input_events(events);
            handles = UIHandles();
            handles.document_id = document().id;
            app.gui->push_layer(app.gui->make_id("test.layer"));
            GUI::ElementHandle root = EditorGUI::begin_v_layout(app.gui,
                app.gui->make_id("test.root"), "Test", fill_layout());
            EditorGUI::begin_dock_space(app.gui, app.gui->make_id("editor.dock_space"),
                "Test DockSpace", fill_layout());
            app.build_palette_panel(handles);
            app.build_hierarchy_panel(handles);
            EditorGUI::end_dock_space(app.gui);
            EditorGUI::end_v_layout(app.gui, root);
            app.gui->pop_layer();
            lupanic_if_failed(EditorGUI::layout_tree(app.gui, root, RectF(0, 0, 640, 640)));
            app.gui->route_input();
            if(EditorGUI::resolve_interactions(app.gui).relayout_requested)
                lupanic_if_failed(EditorGUI::layout_tree(app.gui, root, RectF(0, 0, 640, 640)));
            lupanic_if_failed(app.gui->generate_draw_commands());
            app.process_interactions(handles);
            luassert_always(app.error_message.empty());
        }

        Float2U palette(const Guid& type)
        {
            for(const TypeHit& hit : handles.types)
            {
                if(hit.type != type) continue;
                RectF rect = item_screen_rect(app.gui, hit.element);
                return Float2U(rect.offset_x + rect.width * 0.5f, rect.offset_y + rect.height * 0.5f);
            }
            lupanic();
            return Float2U(0.0f);
        }

        Float2U node(const Guid& id, f32 vertical_position = 0.5f)
        {
            for(const NodeHit& hit : handles.nodes)
            {
                if(hit.node != id) continue;
                RectF rect = item_screen_rect(app.gui, hit.element);
                return Float2U(rect.offset_x + rect.width * 0.5f,
                    rect.offset_y + rect.height * vertical_position);
            }
            lupanic();
            return Float2U(0.0f);
        }

        void click(const Float2U& at)
        {
            frame({pointer(GUI::InputEventType::pointer_down, at)});
            frame({pointer(GUI::InputEventType::pointer_up, at)});
            frame();
        }

        void drag(const Float2U& from, const Float2U& to, HierarchyDropMode expected)
        {
            frame({pointer(GUI::InputEventType::pointer_down, from)});
            frame({pointer(GUI::InputEventType::pointer_move, to)});
            luassert_always(document().hierarchy_drag.dragging);
            luassert_always(document().hierarchy_drag.drop_mode == expected);
            frame({pointer(GUI::InputEventType::pointer_up, to)});
            frame();
        }

        const Vector<AuthoringChildLink>& children(const Guid& parent)
        {
            const AuthoringNodeRecord* record = find_authoring_node(*document().snapshot, parent);
            luassert_always(record);
            return record->children;
        }
    };

    void palette_drag_test()
    {
        HierarchyTest test;
        Guid root = test.document().snapshot->root;
        test.click(test.palette(GameGUI::get_flex_node_type()));
        Guid container = test.document().selected_node;
        luassert_always(container != root && test.children(root).size() == 1);
        test.drag(test.palette(GameGUI::get_text_node_type()), test.node(container), HierarchyDropMode::child);
        Guid text = test.document().selected_node;
        luassert_always(test.children(container).size() == 1 && test.children(container)[0].child == text);
        // The formerly empty parent is expanded, so the created child is visible.
        test.node(text);
        test.drag(test.palette(GameGUI::get_button_node_type()), test.node(text, 0.05f), HierarchyDropMode::before);
        Guid button = test.document().selected_node;
        luassert_always(test.children(container).size() == 2);
        luassert_always(test.children(container)[0].child == button);
        luassert_always(test.children(container)[1].child == text);
        test.drag(test.palette(GameGUI::get_canvas_node_type()), test.node(text, 0.95f), HierarchyDropMode::after);
        Guid canvas = test.document().selected_node;
        luassert_always(test.children(container).size() == 3 && test.children(container)[2].child == canvas);
        test.app.undo_document(test.document());
        test.frame();
        luassert_always(test.children(container).size() == 2);
        test.app.redo_document(test.document());
        test.frame();
        luassert_always(test.children(container).size() == 3 && test.children(container)[2].child == canvas);

        // Existing-node movement shares the same hit zones, with move-only index adjustment.
        test.drag(test.node(button), test.node(canvas, 0.95f), HierarchyDropMode::after);
        luassert_always(test.children(container)[2].child == button);
        test.drag(test.node(button), test.node(root), HierarchyDropMode::child);
        luassert_always(test.children(root).size() == 2 && test.children(root)[1].child == button);
        test.drag(test.node(container), test.node(canvas), HierarchyDropMode::none);
        luassert_always(test.children(root)[0].child == container);

        usize count = test.document().snapshot->nodes.size();
        Float2U source = test.palette(GameGUI::get_text_node_type());
        test.drag(source, Float2U(630.0f, 620.0f), HierarchyDropMode::none);
        luassert_always(test.document().snapshot->nodes.size() == count);
        test.frame({pointer(GUI::InputEventType::pointer_down, source)});
        test.frame({pointer(GUI::InputEventType::pointer_move, test.node(root))});
        test.frame({pointer(GUI::InputEventType::pointer_up, source)});
        test.frame();
        luassert_always(test.document().snapshot->nodes.size() == count);

        // Fast drags can cross several positions in a single frame, including back to the button.
        test.frame({pointer(GUI::InputEventType::pointer_down, source),
            pointer(GUI::InputEventType::pointer_move, test.node(root)),
            pointer(GUI::InputEventType::pointer_up, source)});
        test.frame();
        luassert_always(test.document().snapshot->nodes.size() == count);
        test.frame({pointer(GUI::InputEventType::pointer_down, source),
            pointer(GUI::InputEventType::pointer_up, test.node(root)),
            pointer(GUI::InputEventType::pointer_move, source)});
        test.frame();
        ++count;
        luassert_always(test.document().snapshot->nodes.size() == count);
        luassert_always(test.children(root).back().child == test.document().selected_node);

        GUI::InputEvent blur;
        blur.type = GUI::InputEventType::blur;
        test.frame({pointer(GUI::InputEventType::pointer_down, source)});
        test.frame({pointer(GUI::InputEventType::pointer_move, test.node(root)), blur});
        test.frame();
        luassert_always(!test.document().hierarchy_drag.pressed);
        luassert_always(test.document().snapshot->nodes.size() == count);
        GUI::InputEvent focus;
        focus.type = GUI::InputEventType::focus;
        test.frame({focus});

        GUI::InputEvent escape;
        escape.type = GUI::InputEventType::key_down;
        escape.key = KeyCode::esc;
        test.frame({pointer(GUI::InputEventType::pointer_down, source)});
        test.frame({pointer(GUI::InputEventType::pointer_move, test.node(root)), escape});
        test.frame({pointer(GUI::InputEventType::pointer_up, test.node(root))});
        test.frame();
        luassert_always(test.document().snapshot->nodes.size() == count);

        // Palette drags are scoped to their document; switching documents cancels the old drag.
        test.frame({pointer(GUI::InputEventType::pointer_down, source)});
        test.frame({pointer(GUI::InputEventType::pointer_move, test.node(root))});
        luassert_always(test.app.create_document());
        test.frame();
        test.frame({pointer(GUI::InputEventType::pointer_up, test.node(test.document().snapshot->root))});
        luassert_always(test.document().snapshot->nodes.size() == 1);
        luassert_always(!test.app.documents[0].hierarchy_drag.pressed);
        luassert_always(test.app.documents[0].snapshot->nodes.size() == count);
    }
}

void explorer_test();

int main()
{
    lupanic_if_failed(Luna::init());
    lupanic_if_failed(add_modules({GameGUI::module_game_gui(), EditorGUI::module_editor_gui(),
        Frontend::module_frontend()}));
    lupanic_if_failed(init_modules());
    palette_drag_test();
    explorer_test();
    Luna::close();
    return 0;
}
