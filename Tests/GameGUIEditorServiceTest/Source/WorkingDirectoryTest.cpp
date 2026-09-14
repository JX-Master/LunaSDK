/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file WorkingDirectoryTest.cpp
* @author JXMaster
* @date 2026/9/11
*/
#include "../../../Programs/GameGUIEditor/Service/GameGUIEditorService.hpp"
#include <Luna/Asset/Database.hpp>
#include <Luna/GameGUI/GameGUI.hpp>
#include <Luna/Runtime/Guid.hpp>
#include <Luna/Runtime/Random.hpp>
#include <Luna/VFS/VFS.hpp>
#include <Luna/VFS/NativeFileSystem.hpp>
#include <filesystem>

using namespace Luna;
using namespace Luna::GameGUIEditor;

namespace
{
    Path child_path(Path parent, const c8* name) { parent.push_back(name); return parent; }
    Variant call(Service& service, const c8* url, const Variant& params = Variant(VariantType::object))
    {
        auto result = service.frontend()->invoke(url, params);
        lupanic_if_failed(result);
        return move(result.get());
    }
    Variant document_params(const Variant& metadata)
    {
        Variant params(VariantType::object);
        params["document_id"] = metadata["document_id"];
        params["expected_revision"] = metadata["revision"];
        return params;
    }
    Variant directory_params(const Variant& metadata)
    {
        Variant params(VariantType::object);
        params["working_directory_id"] = metadata["working_directory_id"];
        return params;
    }
    Variant open_directory(Service& service, const Path& path)
    {
        Variant params(VariantType::object);
        params["native_path"] = path.encode().c_str();
        return call(service, OPEN_DIRECTORY_URL, params);
    }
    Path asset_path(const Variant& directory, const c8* name)
    {
        return child_path(Path(directory["vfs_path"].c_str()), name);
    }
    Variant save_as(Service& service, const Variant& document, const Path& path)
    {
        Variant params = document_params(document);
        params["path"] = path.encode().c_str();
        return call(service, SAVE_AS_URL, params);
    }
    Variant rename_root(Service& service, const Variant& document, const c8* name)
    {
        Variant snapshot = call(service, GET_SNAPSHOT_URL, document_params(document));
        Variant command(VariantType::object);
        command["kind"] = "set_name";
        command["node"] = snapshot["document"]["root"];
        command["name"] = name;
        Variant params = document_params(snapshot);
        params["commands"] = Variant(VariantType::array);
        params["commands"].push_back(move(command));
        return call(service, APPLY_COMMANDS_URL, params);
    }
    Variant add_reference(Service& service, const Variant& document, const Variant& referenced)
    {
        Variant snapshot = call(service, GET_SNAPSHOT_URL, document_params(document));
        Variant command(VariantType::object);
        command["kind"] = "insert_node";
        command["parent"] = snapshot["document"]["root"];
        c8 type[GUID_STRING_LENGTH + 1] = {};
        lupanic_if_failed(encode_guid(GameGUI::get_asset_instance_node_type(), type, GUID_STRING_LENGTH));
        command["type"] = type;
        command["properties"] = Variant(VariantType::object);
        command["properties"]["asset"] = referenced["asset_guid"];
        Variant params = document_params(snapshot);
        params["commands"] = Variant(VariantType::array);
        params["commands"].push_back(move(command));
        return call(service, APPLY_COMMANDS_URL, params);
    }
    bool preview_ready(Service& service, const Variant& document)
    {
        auto preview = service.prepare_preview(document["document_id"].unum());
        lupanic_if_failed(preview);
        if(!preview.get().document) return false;
        auto instance = GameGUI::new_instance(preview.get());
        return succeeded(instance->prepare());
    }
    Variant close_plan(Service& service, const Variant& directory, bool discard)
    {
        Variant plan = call(service, PREPARE_CLOSE_DIRECTORY_URL, directory_params(directory));
        Variant decisions(VariantType::array);
        for(const Variant& metadata : plan["documents"].values())
        {
            Variant decision = document_params(metadata);
            decision["discard"] = discard;
            decisions.push_back(move(decision));
        }
        plan["documents"] = move(decisions);
        return plan;
    }
}

void working_directory_test()
{
    const c8* current = get_current_dir();
    Path fixture(current);
    release_current_dir(current);
    c8 unique[GUID_STRING_LENGTH + 1] = {};
    lupanic_if_failed(encode_guid(random_guid(), unique, GUID_STRING_LENGTH));
    String fixture_name = "GameGUIWorkingDirectoryTest-";
    fixture_name.append(unique);
    fixture.push_back(fixture_name);
    lupanic_if_failed(create_dir(fixture.encode().c_str()));
    Path a = child_path(fixture, "Central");
    Path b = child_path(fixture, "Sidecar");
    Path duplicate = child_path(fixture, "Duplicate");
    for(const Path& path : {a, b, duplicate}) lupanic_if_failed(create_dir(path.encode().c_str()));
    lupanic_if_failed(create_dir(child_path(b, "Empty Folder").encode().c_str()));
    {
        auto fs = VFS::new_native_file_system(a.encode().c_str());
        lupanic_if_failed(fs);
        lupanic_if_failed(VFS::mount(fs.get(), "/GameGUIWorkingDirectoryFixture"));
        auto db = Asset::create_file_database("/GameGUIWorkingDirectoryFixture");
        lupanic_if_failed(db);
        db.get() = nullptr;
        lupanic_if_failed(VFS::unmount("/GameGUIWorkingDirectoryFixture"));
    }
    {
        auto created = new_service();
        lupanic_if_failed(created);
        auto service = move(created.get());
        luassert_always(!service->frontend()->invoke(CREATE_DOCUMENT_URL, Variant(VariantType::object)).valid());
        Variant central = open_directory(*service, a);
        Variant sidecar = open_directory(*service, b);
        luassert_always(open_directory(*service, b)["working_directory_id"] == sidecar["working_directory_id"]);
        luassert_always(call(*service, LIST_DIRECTORIES_URL).size() == 2);
        luassert_always(sidecar["folders"].size() == 1);
        Variant params(VariantType::object);
        params["native_path"] = child_path(b, "Empty Folder").encode().c_str();
        luassert_always(!service->frontend()->invoke(OPEN_DIRECTORY_URL, params).valid());
        params["native_path"] = child_path(fixture, "Outside.json").encode().c_str();
        luassert_always(!service->frontend()->invoke(RESOLVE_PATH_URL, params).valid());
        params["native_path"] = child_path(b, "Menu with spaces.json").encode().c_str();
        Variant resolved = call(*service, RESOLVE_PATH_URL, params);
        luassert_always(resolved["working_directory_id"] == sidecar["working_directory_id"]);

        Variant first = call(*service, CREATE_DOCUMENT_URL, directory_params(central));
        Variant second = call(*service, CREATE_DOCUMENT_URL, directory_params(sidecar));
        params = document_params(first);
        params["path"] = "/OutsideWorkingDirectories/NoAsset";
        luassert_always(!service->frontend()->invoke(SAVE_AS_URL, params).valid());
        first = save_as(*service, first, asset_path(central, "Menu"));
        second = save_as(*service, second, asset_path(sidecar, "Menu"));
        luassert_always(first["asset_guid"] != second["asset_guid"]);
        luassert_always(get_file_attribute(child_path(a, "Menu.json").encode().c_str()).valid());
        luassert_always(!get_file_attribute(child_path(a, "Menu.meta").encode().c_str()).valid());
        luassert_always(get_file_attribute(child_path(b, "Menu.meta").encode().c_str()).valid());

        // Saving an untitled document during close needs the current close token. A stale
        // pre-save decision cannot close it, and cancellation leaves the successful save intact.
        Variant untitled = call(*service, CREATE_DOCUMENT_URL, directory_params(sidecar));
        Variant closing = close_plan(*service, sidecar, true);
        params = document_params(untitled);
        params["path"] = asset_path(sidecar, "SavedWhileClosing").encode().c_str();
        luassert_always(!service->frontend()->invoke(SAVE_AS_URL, params).valid());
        params["close_token"] = closing["close_token"];
        untitled = call(*service, SAVE_AS_URL, params);
        luassert_always(!untitled["dirty"].boolean());
        luassert_always(!service->frontend()->invoke(CLOSE_DIRECTORY_URL, closing).valid());
        call(*service, CANCEL_CLOSE_DIRECTORY_URL, closing);
        call(*service, CLOSE_DOCUMENT_URL, document_params(untitled));
        auto closing_asset = Asset::get_asset_by_path(asset_path(sidecar, "SavedWhileClosing"));
        lupanic_if_failed(closing_asset);
        lupanic_if_failed(Asset::delete_asset(closing_asset.get()));

        // Existing destinations require explicit replacement and never overwrite an open peer.
        params = document_params(first);
        params["path"] = asset_path(sidecar, "Menu").encode().c_str();
        params["overwrite"] = true;
        luassert_always(!service->frontend()->invoke(SAVE_AS_URL, params).valid());

        // Register non-GameGUI metadata and cooked-only GameGUI metadata, without loading either.
        auto foreign = Asset::new_asset(asset_path(sidecar, "Foreign"), "Tests.UnknownAssetType", true);
        lupanic_if_failed(foreign);
        auto cooked_only = Asset::new_asset(asset_path(sidecar, "CookedOnly"), GameGUI::get_asset_type(), true);
        lupanic_if_failed(cooked_only);
        sidecar = call(*service, REFRESH_DIRECTORY_URL, directory_params(sidecar));
        luassert_always(sidecar["assets"].size() == 3);
        params = Variant(VariantType::object);
        params["path"] = asset_path(sidecar, "CookedOnly").encode().c_str();
        luassert_always(!service->frontend()->invoke(OPEN_DOCUMENT_URL, params).valid());
        Vector<Asset::AssetDataUnitDesc> units;
        lupanic_if_failed(Asset::get_asset_data_units(cooked_only.get(), units));
        luassert_always(units.size() == 1);
        second = call(*service, GET_SNAPSHOT_URL, document_params(second));

        // Current edits in another directory are available without publishing or saving cooked data.
        second = rename_root(*service, second, "Unsaved nested edit");
        first = add_reference(*service, first, second);
        luassert_always(preview_ready(*service, first));
        {
            auto preview = service->prepare_preview(first["document_id"].unum());
            lupanic_if_failed(preview);
            auto nested_asset = Asset::get_asset_by_path(asset_path(sidecar, "Menu"));
            lupanic_if_failed(nested_asset);
            auto nested = preview.get().resource_resolver.resolve(preview.get().resource_resolver.userdata.get(), nested_asset.get());
            lupanic_if_failed(nested);
            auto document = cast_object<GameGUI::Document>(nested.get().get());
            luassert_always(document && document->nodes[0].name == Name("Unsaved nested edit"));
            auto state = Asset::get_asset_data_unit_state(nested_asset.get(), Name());
            luassert_always(state.valid() && state.get() == Asset::AssetDataUnitState::unloaded);
        }
        luassert_always(!get_file_attribute(child_path(b, "Menu.cooked").encode().c_str()).valid());
        first = call(*service, SAVE_URL, document_params(first));

        // Prepare is non-destructive, freezes edits, and cannot close without explicit decisions.
        Variant plan = call(*service, PREPARE_CLOSE_DIRECTORY_URL, directory_params(sidecar));
        luassert_always(!service->frontend()->invoke(CREATE_DOCUMENT_URL, directory_params(sidecar)).valid());
        luassert_always(!service->frontend()->invoke(SAVE_URL, document_params(second)).valid());
        luassert_always(!service->frontend()->invoke(CLOSE_DIRECTORY_URL, plan).valid());
        call(*service, CANCEL_CLOSE_DIRECTORY_URL, plan);
        auto unchanged = call(*service, GET_SNAPSHOT_URL, document_params(second));
        luassert_always(unchanged["dirty"].boolean() && unchanged["revision"] == second["revision"]);
        second = call(*service, SAVE_URL, document_params(second));

        // A live file handle prevents VFS unmount; the directory and documents remain recoverable.
        {
            auto file = VFS::open_file(child_path(Path(sidecar["vfs_path"].c_str()), "Menu.json"), FileOpenFlag::read,
                FileCreationMode::open_existing);
            lupanic_if_failed(file);
            plan = close_plan(*service, sidecar, false);
            luassert_always(!service->frontend()->invoke(CLOSE_DIRECTORY_URL, plan).valid());
            luassert_always(call(*service, LIST_DOCUMENTS_URL).size() == 2);
            luassert_always(Asset::get_asset_by_path(asset_path(sidecar, "Menu")).valid());
        }
        plan = close_plan(*service, sidecar, false);
        call(*service, CLOSE_DIRECTORY_URL, plan);
        luassert_always(!preview_ready(*service, first));
        sidecar = open_directory(*service, b);
        luassert_always(preview_ready(*service, first));
        params = Variant(VariantType::object);
        params["asset_guid"] = second["asset_guid"];
        second = call(*service, OPEN_DOCUMENT_URL, params);
        luassert_always(!second["dirty"].boolean());

        // A duplicated GUID rejects the whole directory, without a partial registration.
        lupanic_if_failed(copy_file(child_path(b, "Menu.meta").encode().c_str(), child_path(duplicate, "Copied.meta").encode().c_str()));
        params = Variant(VariantType::object);
        params["native_path"] = duplicate.encode().c_str();
        luassert_always(!service->frontend()->invoke(OPEN_DIRECTORY_URL, params).valid());
        luassert_always(call(*service, LIST_DIRECTORIES_URL).size() == 2);
        // Centralized metadata is authoritative even when an unrelated sidecar is present.
        lupanic_if_failed(copy_file(child_path(b, "Menu.meta").encode().c_str(), child_path(a, "Stray.meta").encode().c_str()));
        central = call(*service, REFRESH_DIRECTORY_URL, directory_params(central));
        luassert_always(central["assets"].size() == 1);
        first = call(*service, GET_SNAPSHOT_URL, document_params(first));

        // A failed metadata flush must preserve the previous binding and the dirty save point.
        first = rename_root(*service, first, "After failed flush");
        Path database = child_path(a, "assets.db");
        Path backup = child_path(a, "DatabaseBackup");
        lupanic_if_failed(move_file(database.encode().c_str(), backup.encode().c_str()));
        lupanic_if_failed(create_dir(database.encode().c_str()));
        luassert_always(!service->frontend()->invoke(SAVE_URL, document_params(first)).valid());
        auto failed = call(*service, GET_SNAPSHOT_URL, document_params(first));
        luassert_always(failed["dirty"].boolean() && failed["revision"] == first["revision"]);
        lupanic_if_failed(delete_file(database.encode().c_str()));
        lupanic_if_failed(move_file(backup.encode().c_str(), database.encode().c_str()));
        first = call(*service, SAVE_URL, document_params(failed));
        luassert_always(!first["dirty"].boolean());

#if !defined(LUNA_PLATFORM_WINDOWS)
        // Symlink traversal is checked even after the directory has already been opened.
        std::error_code link_error;
        Path escape = child_path(a, "Escape");
        std::filesystem::create_directory_symlink(std::filesystem::path((const char8_t*)b.encode().c_str()),
            std::filesystem::path((const char8_t*)escape.encode().c_str()), link_error);
        luassert_always(!link_error);
        auto escaped = VFS::open_file(child_path(asset_path(central, "Escape"), "Menu.json"),
            FileOpenFlag::read, FileCreationMode::open_existing);
        luassert_always(!escaped.valid());
        std::filesystem::remove(std::filesystem::path((const char8_t*)escape.encode().c_str()), link_error);
        luassert_always(!link_error);
#endif

        // Reopening preserves the GUID and reads the successful save, rather than a cached object.
        plan = close_plan(*service, central, false);
        call(*service, CLOSE_DIRECTORY_URL, plan);
        central = open_directory(*service, a);
        params = Variant(VariantType::object);
        params["asset_guid"] = first["asset_guid"];
        first = call(*service, OPEN_DOCUMENT_URL, params);
        auto snapshot = call(*service, GET_SNAPSHOT_URL, document_params(first));
        auto decoded = decode_authoring_document(snapshot["document"]);
        lupanic_if_failed(decoded);
        luassert_always(decoded.get()->nodes[0].name == Name("After failed flush"));
        luassert_always(!first["dirty"].boolean() && preview_ready(*service, first));

        // Cycles across roots remain diagnostics and do not recurse indefinitely.
        second = add_reference(*service, second, first);
        luassert_always(!preview_ready(*service, first));
        plan = close_plan(*service, sidecar, true);
        call(*service, CLOSE_DIRECTORY_URL, plan);
        plan = close_plan(*service, central, false);
        call(*service, CLOSE_DIRECTORY_URL, plan);
        luassert_always(call(*service, LIST_DIRECTORIES_URL).empty());
        luassert_always(call(*service, LIST_DOCUMENTS_URL).empty());
    }
    // Only this test's newly created, uniquely named fixture is removed.
    luassert_always(fixture.back() == Name(fixture_name));
    std::error_code cleanup;
    std::filesystem::remove_all(std::filesystem::path((const char8_t*)fixture.encode().c_str()), cleanup);
    luassert_always(!cleanup);
}
