/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Main.cpp
* @author JXMaster
* @date 2026/8/26
*/
#include "../../../Programs/GameGUIEditor/Service/GameGUIEditorService.hpp"
#include <Luna/GameGUI/GameGUI.hpp>
#include <Luna/Runtime/Assert.hpp>
#include <Luna/Runtime/Guid.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Random.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/VFS/VFS.hpp>
#include <cstring>

using namespace Luna;

#define lutest luassert_always

namespace
{
    constexpr const c8* MOUNT_PATH = "/GameGUIEditorServiceTest";
    constexpr const c8* SAVE_PATH = "/GameGUIEditorServiceTest/SavedDocument";
    constexpr const c8* SOURCE_PATH = "/GameGUIEditorServiceTest/UnknownSource";

    String guid_string(const Guid& guid)
    {
        c8 buffer[GUID_STRING_LENGTH];
        lupanic_if_failed(encode_guid(guid, buffer, sizeof(buffer)));
        return String(buffer, sizeof(buffer));
    }

    Variant document_params(const Variant& metadata)
    {
        Variant params(VariantType::object);
        params["document_id"] = metadata["document_id"];
        params["expected_revision"] = metadata["revision"];
        return params;
    }

    Variant invoke(Frontend::IFrontend* frontend, const c8* url,
        const Variant& params = Variant(VariantType::object))
    {
        auto result = frontend->invoke(url, params);
        lutest(result.valid());
        return move(result.get());
    }

    Variant command_batch(const Variant& metadata, Variant&& command,
        const c8* label, const c8* coalesce_key = "")
    {
        Variant params = document_params(metadata);
        params["label"] = label;
        if(coalesce_key[0]) params["coalesce_key"] = coalesce_key;
        params["commands"] = Variant(VariantType::array);
        params["commands"].push_back(move(command));
        return params;
    }

    Variant set_property_command(const Guid& node, const c8* property,
        const Variant& value)
    {
        Variant command(VariantType::object);
        command["kind"] = "set_property";
        command["node"] = guid_string(node).c_str();
        command["property"] = property;
        command["value"] = value;
        return command;
    }

    void remove_file_if_present(const c8* path)
    {
        RV result = VFS::delete_file(path);
        lutest(succeeded(result) || result.errcode() == E_NOT_FOUND);
    }

    void service_history_test(GameGUIEditor::Service& service)
    {
        Frontend::IFrontend* frontend = service.frontend();
        Variant first = invoke(frontend, GameGUIEditor::CREATE_DOCUMENT_URL);
        Variant second = invoke(frontend, GameGUIEditor::CREATE_DOCUMENT_URL);
        lutest(first["document_id"].unum() != second["document_id"].unum());
        lutest(first["dirty"].boolean() && second["dirty"].boolean());

        Variant listed = invoke(frontend, GameGUIEditor::LIST_DOCUMENTS_URL);
        lutest(listed.type() == VariantType::array && listed.size() == 2);
        Variant schemas = invoke(frontend, GameGUIEditor::GET_NODE_TYPES_URL);
        lutest(schemas.type() == VariantType::array && schemas.size() >= 6);

        Variant snapshot = invoke(frontend, GameGUIEditor::GET_SNAPSHOT_URL,
            document_params(first));
        auto document_result = GameGUI::decode_document(snapshot["document"]);
        lutest(document_result.valid());
        Ref<GameGUI::Document> document = document_result.get();
        Guid root = document->root;

        Variant insert(VariantType::object);
        insert["kind"] = "insert_node";
        insert["parent"] = guid_string(root).c_str();
        insert["type"] = guid_string(GameGUI::get_text_node_type()).c_str();
        insert["name"] = "Child Text";
        Variant edit = invoke(frontend, GameGUIEditor::APPLY_COMMANDS_URL,
            command_batch(first, move(insert), "Insert text"));
        lutest(edit["revision"].unum() > first["revision"].unum());
        lutest(edit["created_nodes"].size() == 1);
        Guid child;
        lupanic_if_failed(decode_guid(edit["created_nodes"][0].c_str(),
            edit["created_nodes"][0].str().size(), child));

        Variant insert_parent(VariantType::object);
        insert_parent["kind"] = "insert_node";
        insert_parent["parent"] = guid_string(root).c_str();
        insert_parent["type"] = guid_string(GameGUI::get_flex_node_type()).c_str();
        insert_parent["name"] = "Container";
        Variant parent_inserted = invoke(frontend, GameGUIEditor::APPLY_COMMANDS_URL,
            command_batch(edit, move(insert_parent), "Insert container"));
        Guid container;
        lupanic_if_failed(decode_guid(parent_inserted["created_nodes"][0].c_str(),
            parent_inserted["created_nodes"][0].str().size(), container));

        Variant insert_sibling(VariantType::object);
        insert_sibling["kind"] = "insert_node";
        insert_sibling["parent"] = guid_string(root).c_str();
        insert_sibling["type"] = guid_string(GameGUI::get_text_node_type()).c_str();
        insert_sibling["name"] = "Sibling Text";
        Variant sibling_inserted = invoke(frontend, GameGUIEditor::APPLY_COMMANDS_URL,
            command_batch(parent_inserted, move(insert_sibling), "Insert sibling"));
        Guid sibling;
        lupanic_if_failed(decode_guid(sibling_inserted["created_nodes"][0].c_str(),
            sibling_inserted["created_nodes"][0].str().size(), sibling));

        Variant reorder(VariantType::object);
        reorder["kind"] = "move_node";
        reorder["node"] = guid_string(child).c_str();
        reorder["parent"] = guid_string(root).c_str();
        reorder["index"] = (u64)2;
        Variant reordered = invoke(frontend, GameGUIEditor::APPLY_COMMANDS_URL,
            command_batch(sibling_inserted, move(reorder), "Reorder child"));
        snapshot = invoke(frontend, GameGUIEditor::GET_SNAPSHOT_URL,
            document_params(reordered));
        document_result = GameGUI::decode_document(snapshot["document"]);
        lutest(document_result.valid());
        const GameGUI::NodeRecord* reordered_root = GameGUI::find_node(
            *document_result.get(), root);
        lutest(reordered_root && reordered_root->children.size() == 3);
        lutest(reordered_root->children[0].child == container);
        lutest(reordered_root->children[1].child == sibling);
        lutest(reordered_root->children[2].child == child);

        Variant reparent(VariantType::object);
        reparent["kind"] = "move_node";
        reparent["node"] = guid_string(child).c_str();
        reparent["parent"] = guid_string(container).c_str();
        reparent["index"] = (u64)0;
        edit = invoke(frontend, GameGUIEditor::APPLY_COMMANDS_URL,
            command_batch(reordered, move(reparent), "Reparent child"));
        snapshot = invoke(frontend, GameGUIEditor::GET_SNAPSHOT_URL,
            document_params(edit));
        document_result = GameGUI::decode_document(snapshot["document"]);
        lutest(document_result.valid());
        const GameGUI::NodeRecord* reparented_root = GameGUI::find_node(
            *document_result.get(), root);
        const GameGUI::NodeRecord* reparented_container = GameGUI::find_node(
            *document_result.get(), container);
        lutest(reparented_root && reparented_root->children.size() == 2);
        lutest(reparented_container && reparented_container->children.size() == 1);
        lutest(reparented_container->children[0].child == child);

        Variant set_root(VariantType::object);
        set_root["kind"] = "set_root";
        set_root["node"] = guid_string(child).c_str();
        Variant root_changed = invoke(frontend, GameGUIEditor::APPLY_COMMANDS_URL,
            command_batch(edit, move(set_root), "Set root node"));
        snapshot = invoke(frontend, GameGUIEditor::GET_SNAPSHOT_URL,
            document_params(root_changed));
        document_result = GameGUI::decode_document(snapshot["document"]);
        lutest(document_result.valid());
        lutest(document_result.get()->root == child);
        const GameGUI::NodeRecord* promoted_root = GameGUI::find_node(
            *document_result.get(), child);
        const GameGUI::NodeRecord* demoted_root = GameGUI::find_node(
            *document_result.get(), root);
        const GameGUI::NodeRecord* detached_container = GameGUI::find_node(
            *document_result.get(), container);
        lutest(promoted_root && promoted_root->children.size() == 1);
        lutest(promoted_root->children[0].child == root);
        lutest(demoted_root && demoted_root->children.size() == 2);
        lutest(demoted_root->children[0].child == container);
        lutest(demoted_root->children[1].child == sibling);
        lutest(detached_container && detached_container->children.empty());

        Variant root_undone = invoke(frontend, GameGUIEditor::UNDO_URL,
            document_params(root_changed));
        snapshot = invoke(frontend, GameGUIEditor::GET_SNAPSHOT_URL,
            document_params(root_undone));
        document_result = GameGUI::decode_document(snapshot["document"]);
        lutest(document_result.valid());
        lutest(document_result.get()->root == root);
        const GameGUI::NodeRecord* restored_container = GameGUI::find_node(
            *document_result.get(), container);
        lutest(restored_container && restored_container->children.size() == 1);
        lutest(restored_container->children[0].child == child);

        edit = invoke(frontend, GameGUIEditor::REDO_URL,
            document_params(root_undone));
        snapshot = invoke(frontend, GameGUIEditor::GET_SNAPSHOT_URL,
            document_params(edit));
        document_result = GameGUI::decode_document(snapshot["document"]);
        lutest(document_result.valid());
        lutest(document_result.get()->root == child);

        Variant stale_command = set_property_command(child, "text", "stale");
        auto stale = frontend->invoke(GameGUIEditor::APPLY_COMMANDS_URL,
            command_batch(first, move(stale_command), "Stale edit"));
        lutest(!stale.valid());

        Variant property = set_property_command(child, "text", "First");
        Variant first_text = invoke(frontend, GameGUIEditor::APPLY_COMMANDS_URL,
            command_batch(edit, move(property), "Edit text", "text-edit"));
        property = set_property_command(child, "text", "Second");
        Variant second_text = invoke(frontend, GameGUIEditor::APPLY_COMMANDS_URL,
            command_batch(first_text, move(property), "Edit text", "text-edit"));
        lutest(second_text["history_state"] == first_text["history_state"]);

        Variant undone = invoke(frontend, GameGUIEditor::UNDO_URL,
            document_params(second_text));
        snapshot = invoke(frontend, GameGUIEditor::GET_SNAPSHOT_URL,
            document_params(undone));
        document_result = GameGUI::decode_document(snapshot["document"]);
        lutest(document_result.valid());
        lutest(GameGUI::find_node(*document_result.get(), child)->properties["text"].str() ==
            Name("Text"));
        Variant redone = invoke(frontend, GameGUIEditor::REDO_URL,
            document_params(undone));
        snapshot = invoke(frontend, GameGUIEditor::GET_SNAPSHOT_URL,
            document_params(redone));
        document_result = GameGUI::decode_document(snapshot["document"]);
        lutest(!strcmp(GameGUI::find_node(*document_result.get(), child)->properties["text"].c_str(),
            "Second"));

        Variant branch_base = invoke(frontend, GameGUIEditor::UNDO_URL,
            document_params(redone));
        property = set_property_command(child, "text", "Branch");
        Variant branch = invoke(frontend, GameGUIEditor::APPLY_COMMANDS_URL,
            command_batch(branch_base, move(property), "Branch edit"));
        auto cannot_redo = frontend->invoke(GameGUIEditor::REDO_URL,
            document_params(branch));
        lutest(!cannot_redo.valid());

        auto refused_close = frontend->invoke(GameGUIEditor::CLOSE_DOCUMENT_URL,
            document_params(second));
        lutest(!refused_close.valid());
        Variant close_params = document_params(second);
        close_params["discard"] = true;
        invoke(frontend, GameGUIEditor::CLOSE_DOCUMENT_URL, close_params);
    }

    void asset_boundary_test(GameGUIEditor::Service& service)
    {
        GameGUI::NodeRecord unknown;
        unknown.id = random_guid();
        unknown.type = random_guid();
        unknown.type_version = 19;
        unknown.name = "Unsupported";
        unknown.properties = Variant(VariantType::object);
        unknown.properties["opaque"] = "preserve";
        Ref<GameGUI::Document> source = new_object<GameGUI::Document>();
        source->root = unknown.id;
        source->nodes.push_back(unknown);
        auto source_asset_result = Asset::new_asset(SOURCE_PATH,
            GameGUI::get_asset_type(), false);
        lutest(source_asset_result.valid());
        Asset::asset_t source_asset = source_asset_result.get();
        lupanic_if_failed(Asset::set_asset_data(source_asset, source.object()));
        lupanic_if_failed(Asset::save_asset(source_asset));

        Variant open_params(VariantType::object);
        open_params["asset_guid"] = guid_string(Asset::get_asset_guid(source_asset)).c_str();
        Variant opened = invoke(service.frontend(), GameGUIEditor::OPEN_DOCUMENT_URL, open_params);
        Variant duplicate = invoke(service.frontend(), GameGUIEditor::OPEN_DOCUMENT_URL, open_params);
        lutest(opened["document_id"] == duplicate["document_id"]);
        lutest(!opened["dirty"].boolean());

        Variant rename(VariantType::object);
        rename["kind"] = "set_name";
        rename["node"] = guid_string(unknown.id).c_str();
        rename["name"] = "Renamed unsupported";
        Variant edited = invoke(service.frontend(), GameGUIEditor::APPLY_COMMANDS_URL,
            command_batch(opened, move(rename), "Rename unsupported"));
        lutest(edited["dirty"].boolean());

        Variant save_as_params = document_params(edited);
        save_as_params["path"] = SAVE_PATH;
        Variant saved = invoke(service.frontend(), GameGUIEditor::SAVE_AS_URL,
            save_as_params);
        lutest(!saved["dirty"].boolean());

        Ref<GameGUI::Document> registry_source =
            Asset::get_asset_data<GameGUI::Document>(source_asset);
        lutest(registry_source && registry_source->nodes[0].name == Name("Unsupported"));

        auto saved_asset_result = Asset::get_asset_by_path(SAVE_PATH);
        lutest(saved_asset_result.valid());
        Asset::asset_t saved_asset = saved_asset_result.get();
        lutest(Asset::get_asset_state(saved_asset) == Asset::AssetState::unloaded);
        lupanic_if_failed(Asset::load_asset(saved_asset, true));
        Ref<GameGUI::Document> loaded = Asset::get_asset_data<GameGUI::Document>(saved_asset);
        lutest(loaded && loaded->nodes.size() == 1);
        lutest(loaded->nodes[0].type == unknown.type);
        lutest(loaded->nodes[0].type_version == 19);
        lutest(loaded->nodes[0].properties["opaque"] == unknown.properties["opaque"]);
        lutest(loaded->nodes[0].name == Name("Renamed unsupported"));

        Variant close_params = document_params(saved);
        invoke(service.frontend(), GameGUIEditor::CLOSE_DOCUMENT_URL, close_params);
    }
}

int main()
{
    lupanic_if_failed(Luna::init());
    lupanic_if_failed(add_modules({
        GameGUI::module_game_gui(),
        Frontend::module_frontend()
    }));
    lupanic_if_failed(init_modules());
    const c8* current_dir = get_current_dir();
    RV mounted = VFS::mount(VFS::get_platform_filesystem_driver(), current_dir, MOUNT_PATH);
    release_current_dir(current_dir);
    lupanic_if_failed(mounted);
    {
        auto service_result = GameGUIEditor::new_service();
        lutest(service_result.valid());
        UniquePtr<GameGUIEditor::Service> service = move(service_result.get());
        service_history_test(*service);
        asset_boundary_test(*service);
    }
    remove_file_if_present("/GameGUIEditorServiceTest/UnknownSource.json");
    remove_file_if_present("/GameGUIEditorServiceTest/SavedDocument.json");
    remove_file_if_present("/GameGUIEditorServiceTest/SavedDocument.meta");
    lupanic_if_failed(VFS::unmount(MOUNT_PATH));
    Luna::close();
    return 0;
}
