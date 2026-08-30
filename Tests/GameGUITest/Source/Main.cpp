/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Main.cpp
* @author JXMaster
* @date 2026/8/25
*/
#include <Luna/GameGUI/GameGUI.hpp>
#include <Luna/Runtime/Assert.hpp>
#include <Luna/Runtime/File.hpp>
#include <Luna/Runtime/Guid.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Random.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/VFS/VFS.hpp>

using namespace Luna;
using namespace Luna::GameGUI;

#define lutest luassert_always

namespace
{
    usize g_state_build_count = 0;
    constexpr const c8* g_document_asset_path = "/GameGUITest/DocumentAsset";

    NodeRecord make_node(const Guid& type, const c8* name = "")
    {
        NodeRecord node;
        node.id = random_guid();
        node.type = type;
        node.name = name;
        node.properties = Variant(VariantType::object);
        return node;
    }

    Ref<Document> make_single_node_document(const NodeRecord& node)
    {
        Ref<Document> document = new_object<Document>();
        document->root = node.id;
        document->nodes.push_back(node);
        return document;
    }

    void delete_asset_source(const Path& asset_path)
    {
        Path document_path = asset_path;
        document_path.append_extension("cooked");
        lupanic_if_failed(VFS::delete_file(document_path));
    }

    void document_asset_test()
    {
        NodeRecord root = make_node(get_flex_node_type(), "Root");
        root.properties["gap"] = 12.0;
        root.properties["nested"] = Variant(VariantType::object);
        root.properties["nested"]["value"] = (u64)42;
        Ref<Document> source = make_single_node_document(root);

        auto asset_result = Asset::new_asset(g_document_asset_path, get_asset_type(), false);
        lutest(asset_result.valid());
        Asset::asset_t asset = asset_result.get();
        lupanic_if_failed(Asset::set_asset_data_unit_object(asset, Name(), source.object()));
        lupanic_if_failed(Asset::save_asset_data_unit(asset, Name()));
        lupanic_if_failed(Asset::set_asset_data_unit_object(asset, Name(), nullptr));
        lupanic_if_failed(Asset::load_asset_data_unit(asset, Name()));
        auto loaded_result = Asset::get_asset_data_unit_object<Document>(asset, Name());
        lutest(loaded_result.valid());
        Ref<Document> loaded = loaded_result.get();
        lutest(loaded);
        lutest(loaded->nodes.size() == 1);
        lutest(loaded->nodes[0].type == root.type);
        lutest(loaded->nodes[0].properties == root.properties);

        loaded->nodes[0].properties["unsaved"] = true;
        lupanic_if_failed(Asset::load_asset_data_unit(asset, Name(), true));
        loaded_result = Asset::get_asset_data_unit_object<Document>(asset, Name());
        lutest(loaded_result.valid());
        loaded = loaded_result.get();
        lutest(loaded && !loaded->nodes[0].properties.contains("unsaved"));

        delete_asset_source(g_document_asset_path);
    }

    void topology_validation_test()
    {
        NodeRecord root = make_node(get_flex_node_type());
        NodeRecord orphan = make_node(get_text_node_type());
        Ref<Document> document = new_object<Document>();
        document->root = root.id;
        document->nodes.push_back(root);
        document->nodes.push_back(orphan);
        Vector<Diagnostic> diagnostics;
        lutest(failed(validate_document(*document, &diagnostics)));
        lutest(!diagnostics.empty());

        ChildLink link;
        link.child = orphan.id;
        document->nodes[0].children.push_back(link);
        lutest(succeeded(validate_document(*document)));
        document->nodes[1].children.push_back(ChildLink{root.id});
        lutest(failed(validate_document(*document)));
    }

    R<Any> prepare_test_node(const NodeRecord& node, object_t userdata)
    {
        Any data;
        data.emplace<u64>(123);
        return data;
    }

    Variant create_test_state(object_t userdata)
    {
        Variant state(VariantType::object);
        state["build_count"] = (u64)40;
        return state;
    }

    R<GUI::ElementHandle> build_test_node(BuildContext& context, const NodeRecord& node, object_t userdata)
    {
        lutest(context.prepared_data().as<u64>() && *context.prepared_data().as<u64>() == 123);
        Variant& state = context.state();
        state["build_count"] = state["build_count"].unum() + 1;
        g_state_build_count = (usize)state["build_count"].unum();
        GUI::ElementHandle element = context.gui()->begin_element(context.make_id("element"));
        context.gui()->end_element();
        return element;
    }

    RV resolve_test_node(ResolveContext& context, const NodeRecord& node, object_t userdata)
    {
        context.request_relayout();
        return ok;
    }

    void registry_and_state_test()
    {
        Guid type = random_guid();
        NodeTypeDesc desc;
        desc.type = type;
        desc.name = "GameGUITest.StatefulNode";
        desc.prepare = prepare_test_node;
        desc.create_state = create_test_state;
        desc.build = build_test_node;
        desc.resolve = resolve_test_node;
        lupanic_if_failed(register_node_type(desc));
        lutest(failed(register_node_type(desc)));

        NodeRecord node = make_node(type);
        Ref<Document> document = make_single_node_document(node);
        InstanceDesc instance_desc;
        instance_desc.document = document;
        instance_desc.instance_scope = 0x12345678;
        Ref<IInstance> instance = new_instance(instance_desc);
        lupanic_if_failed(instance->prepare());

        Ref<GUI::IContext> gui = GUI::new_context();
        GUI::FrameDesc frame;
        frame.logical_size = Float2U(100.0f, 100.0f);
        gui->begin_frame(frame);
        gui->push_layer(1);
        auto first = instance->build(gui);
        gui->pop_layer();
        lutest(first.valid());
        lutest(g_state_build_count == 41);
        GUI::id_t stable_id = first.get().id;
        lutest(stable_id == instance->make_stable_id(node.id, "element"));

        gui->begin_frame(frame);
        gui->push_layer(1);
        auto second = instance->build(gui);
        gui->pop_layer();
        lutest(second.valid());
        lutest(second.get().id == stable_id);
        lutest(second.get().generation != first.get().generation);
        lutest(g_state_build_count == 42);
        lutest(failed(instance->build(gui)));
        gui->route_input();
        lupanic_if_failed(instance->resolve_interactions(gui));
        lutest(instance->relayout_requested());
        lupanic_if_failed(unregister_node_type(type));
    }

    void button_action_test()
    {
        NodeRecord button = make_node(get_button_node_type(), "ActionButton");
        button.properties["width"] = 100.0;
        button.properties["height"] = 50.0;
        button.properties["text"] = "Run";
        button.properties["action"] = "run";
        button.properties["action_payload"] = (u64)99;
        Ref<Document> document = make_single_node_document(button);
        InstanceDesc desc;
        desc.document = document;
        Ref<IInstance> instance = new_instance(desc);
        lupanic_if_failed(instance->prepare());

        Ref<GUI::IContext> gui = GUI::new_context();
        GUI::FrameDesc frame;
        frame.logical_size = Float2U(200.0f, 100.0f);
        gui->begin_frame(frame);
        GUI::InputEvent enter;
        enter.type = GUI::InputEventType::pointer_enter;
        enter.position = Float2U(10.0f, 10.0f);
        gui->add_input_event(enter);
        GUI::InputEvent down = enter;
        down.type = GUI::InputEventType::pointer_down;
        gui->add_input_event(down);
        GUI::InputEvent up = enter;
        up.type = GUI::InputEventType::pointer_up;
        gui->add_input_event(up);
        gui->push_layer(1);
        auto root = instance->build(gui);
        gui->pop_layer();
        lutest(root.valid());
        lupanic_if_failed(gui->apply_layout(root.get(), RectF(0.0f, 0.0f, 100.0f, 50.0f)));
        gui->route_input();
        lupanic_if_failed(instance->resolve_interactions(gui));
        Span<const Action> actions = instance->get_actions();
        lutest(actions.size() == 1);
        lutest(actions[0].name == Name("run"));
        lutest(actions[0].node == button.id);
        lutest(actions[0].source_id != 0);
        lutest(actions[0].payload.unum() == 99);
    }

    Variant make_float2(f64 x, f64 y)
    {
        Variant value(VariantType::array);
        value.push_back(x);
        value.push_back(y);
        return value;
    }

    Variant make_float4(f64 x, f64 y, f64 z, f64 w)
    {
        Variant value(VariantType::array);
        value.push_back(x);
        value.push_back(y);
        value.push_back(z);
        value.push_back(w);
        return value;
    }

    void canvas_layout_test()
    {
        NodeRecord canvas = make_node(get_canvas_node_type(), "CanvasRoot");
        NodeRecord panel = make_node(get_panel_node_type(), "PlacedPanel");
        panel.properties["width"] = 40.0;
        panel.properties["height"] = 30.0;
        ChildLink link;
        link.child = panel.id;
        link.attachment = Variant(VariantType::object);
        link.attachment["anchor_min"] = make_float2(0.0, 0.0);
        link.attachment["anchor_max"] = make_float2(0.0, 0.0);
        link.attachment["offset"] = make_float4(10.0, 20.0, 0.0, 0.0);
        canvas.children.push_back(link);
        Ref<Document> document = new_object<Document>();
        document->root = canvas.id;
        document->nodes.push_back(canvas);
        document->nodes.push_back(panel);

        InstanceDesc desc;
        desc.document = document;
        Ref<IInstance> instance = new_instance(desc);
        lupanic_if_failed(instance->prepare());
        Ref<GUI::IContext> gui = GUI::new_context();
        GUI::FrameDesc frame;
        frame.logical_size = Float2U(200.0f, 100.0f);
        gui->begin_frame(frame);
        gui->push_layer(1);
        auto root = instance->build(gui);
        gui->pop_layer();
        lutest(root.valid());
        lupanic_if_failed(gui->apply_layout(root.get(), RectF(0.0f, 0.0f, 200.0f, 100.0f)));
        const GUI::Element* placed = gui->find_element(instance->make_stable_id(panel.id, "element"));
        lutest(placed);
        lutest(placed->layout_result.rect.offset_x == 10.0f);
        lutest(placed->layout_result.rect.offset_y == 20.0f);
        lutest(placed->layout_result.rect.width == 40.0f);
        lutest(placed->layout_result.rect.height == 30.0f);
    }

    Ref<Document> make_nested_text_document()
    {
        NodeRecord text = make_node(get_text_node_type(), "NestedText");
        text.properties["height"] = 20.0;
        text.properties["text"] = "Nested";
        return make_single_node_document(text);
    }

    Asset::asset_t make_dynamic_document_asset(const Ref<Document>& document)
    {
        auto asset = Asset::new_asset(Path(), get_asset_type(), false);
        lutest(asset.valid());
        lupanic_if_failed(Asset::set_asset_data_unit_object(asset.get(), Name(), document.object()));
        return asset.get();
    }

    NodeRecord make_asset_instance(Asset::asset_t asset)
    {
        NodeRecord node = make_node(get_asset_instance_node_type(), "NestedAsset");
        c8 buffer[GUID_STRING_LENGTH];
        lupanic_if_failed(encode_guid(Asset::get_asset_guid(asset), buffer, sizeof(buffer)));
        String asset_guid(buffer, sizeof(buffer));
        node.properties["asset"] = asset_guid.c_str();
        node.properties["height"] = 30.0;
        return node;
    }

    void nested_asset_test()
    {
        Asset::asset_t unloaded_asset = Asset::get_asset(random_guid());
        Ref<Document> unloaded_parent = make_single_node_document(make_asset_instance(unloaded_asset));
        InstanceDesc unloaded_desc;
        unloaded_desc.document = unloaded_parent;
        Ref<IInstance> unloaded_instance = new_instance(unloaded_desc);
        lutest(failed(unloaded_instance->prepare()));

        Ref<Document> nested_document = make_nested_text_document();
        Asset::asset_t nested_asset = make_dynamic_document_asset(nested_document);

        NodeRecord root = make_node(get_flex_node_type(), "RepeatedMounts");
        NodeRecord first = make_asset_instance(nested_asset);
        NodeRecord second = make_asset_instance(nested_asset);
        root.children.push_back(ChildLink{first.id});
        root.children.push_back(ChildLink{second.id});
        Ref<Document> document = new_object<Document>();
        document->root = root.id;
        document->nodes.push_back(root);
        document->nodes.push_back(first);
        document->nodes.push_back(second);
        lupanic_if_failed(validate_document(*document));

        Asset::asset_t parent_asset = make_dynamic_document_asset(document);
        Vector<Asset::asset_t> references;
        Asset::get_asset_data_unit_referred_assets(parent_asset, Name(), references);
        lutest(references.size() == 1 && references[0] == nested_asset);

        InstanceDesc desc;
        desc.document = document;
        desc.source_asset = parent_asset;
        desc.instance_scope = 77;
        Ref<IInstance> instance = new_instance(desc);
        lupanic_if_failed(instance->prepare());
        Ref<GUI::IContext> gui = GUI::new_context();
        GUI::FrameDesc frame;
        frame.logical_size = Float2U(200.0f, 100.0f);
        gui->begin_frame(frame);
        gui->push_layer(1);
        auto generated_root = instance->build(gui);
        gui->pop_layer();
        lutest(generated_root.valid());
        lutest(gui->get_elements().size() == 5);
        Span<const GeneratedNodeInfo> generated = instance->get_generated_nodes();
        lutest(generated.size() == 5);
        usize nested_text_count = 0;
        GUI::id_t first_nested_source = 0;
        for (const GeneratedNodeInfo& info : generated)
        {
            if (info.node == nested_document->root)
            {
                ++nested_text_count;
                if (!first_nested_source)
                    first_nested_source = info.source_id;
                else
                    lutest(first_nested_source != info.source_id);
            }
            lutest(info.source_id != 0 && info.root_element_id != 0);
        }
        lutest(nested_text_count == 2);
        Span<const GUI::Element> elements = gui->get_elements();
        for (usize i = 0; i < elements.size(); ++i)
        {
            for (usize j = i + 1; j < elements.size(); ++j)
                lutest(elements[i].id != elements[j].id);
        }
        lupanic_if_failed(gui->apply_layout(generated_root.get(), RectF(0.0f, 0.0f, 200.0f, 100.0f)));
    }

    void nested_cycle_test()
    {
        Ref<Document> document_a = new_object<Document>();
        Ref<Document> document_b = new_object<Document>();
        Asset::asset_t asset_a = make_dynamic_document_asset(document_a);
        Asset::asset_t asset_b = make_dynamic_document_asset(document_b);
        NodeRecord node_a = make_asset_instance(asset_b);
        NodeRecord node_b = make_asset_instance(asset_a);
        document_a->root = node_a.id;
        document_a->nodes.push_back(node_a);
        document_b->root = node_b.id;
        document_b->nodes.push_back(node_b);

        InstanceDesc desc;
        desc.document = document_a;
        desc.source_asset = asset_a;
        Ref<IInstance> instance = new_instance(desc);
        RV result = instance->prepare();
        lutest(failed(result));
        lutest(!instance->get_diagnostics().empty());
        Span<const Diagnostic> diagnostics = instance->get_diagnostics();
        lutest(diagnostics[diagnostics.size() - 1].asset_mount_chain.size() == 3);
    }
}

int main()
{
    lupanic_if_failed(Luna::init());
    lupanic_if_failed(add_modules({module_game_gui()}));
    lupanic_if_failed(init_modules());
    const c8* current_dir = get_current_dir();
    RV mount_result = VFS::mount(VFS::get_platform_filesystem_driver(), current_dir, "/GameGUITest");
    release_current_dir(current_dir);
    lupanic_if_failed(mount_result);
    document_asset_test();
    topology_validation_test();
    registry_and_state_test();
    button_action_test();
    canvas_layout_test();
    nested_asset_test();
    nested_cycle_test();
    lupanic_if_failed(VFS::unmount("/GameGUITest"));
    Luna::close();
    return 0;
}
