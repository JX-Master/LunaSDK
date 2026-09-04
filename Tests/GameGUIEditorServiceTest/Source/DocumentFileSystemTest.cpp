/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file DocumentFileSystemTest.cpp
* @author JXMaster
* @date 2026/9/4
*/
#include "../../../Programs/GameGUIEditor/Service/GameGUIEditorService.hpp"
#include "../../../Programs/GameGUIEditor/Source/DocumentFileSystem.hpp"
#include <Luna/Runtime/Guid.hpp>
#include <Luna/Runtime/Random.hpp>
#include <Luna/VFS/VFS.hpp>
#include <Luna/VariantUtils/JSON.hpp>

using namespace Luna;
using namespace Luna::GameGUIEditor;

namespace
{
    Path child_path(Path directory, const c8* name)
    {
        directory.push_back(name);
        return directory;
    }

    Variant call(Service& service, const c8* url, const Variant& params)
    {
        auto result = service.frontend()->invoke(url, params);
        lupanic_if_failed(result);
        return move(result.get());
    }

    Variant params_for(const Variant& metadata)
    {
        Variant params(VariantType::object);
        params["document_id"] = metadata["document_id"];
        params["expected_revision"] = metadata["revision"];
        return params;
    }

    Variant rename_root(Service& service, const Variant& metadata, const c8* name)
    {
        Variant snapshot = call(service, GET_SNAPSHOT_URL, params_for(metadata));
        Variant params = params_for(snapshot);
        Variant command(VariantType::object);
        command["kind"] = "set_name";
        command["node"] = snapshot["document"]["root"];
        command["name"] = name;
        params["commands"] = Variant(VariantType::array);
        params["commands"].push_back(move(command));
        return call(service, APPLY_COMMANDS_URL, params);
    }

    Path resolve(Internal::DocumentFileSystem& files, const Path& workspace, const Path& native_path)
    {
        auto result = files.resolve_document_path(workspace, native_path);
        lupanic_if_failed(result);
        return result.get();
    }
}

void document_file_system_test()
{
    // Use isolated sibling directories to model an app workspace and the user's Desktop.
    const c8* current_dir = get_current_dir();
    Path fixture(current_dir);
    release_current_dir(current_dir);
    c8 unique_name[GUID_STRING_LENGTH + 1] = {};
    lupanic_if_failed(encode_guid(random_guid(), unique_name, GUID_STRING_LENGTH));
    fixture.push_back(unique_name);
    lupanic_if_failed(create_dir(fixture.encode().c_str()));
    Path workspace = child_path(fixture, "Workspace");
    Path desktop = child_path(fixture, "Desktop");
    lupanic_if_failed(create_dir(workspace.encode().c_str()));
    lupanic_if_failed(create_dir(desktop.encode().c_str()));
    lupanic_if_failed(VFS::mount(VFS::get_platform_filesystem_driver(), workspace.encode().c_str(), "/"));
    lupanic_if_failed(VFS::create_dir("/__GameGUIEditorExternal_1"));
    Internal::DocumentFileSystem files;
    auto service_result = new_service();
    lupanic_if_failed(service_result);
    UniquePtr<Service> service = move(service_result.get());

    // Existing workspace paths are unchanged; external paths and extensionless names round-trip.
    luassert_always(resolve(files, workspace, child_path(workspace, "Inside.json")) == Path("/Inside"));
    Path native_file = child_path(desktop, "Menu with spaces.json");
    Path asset_path = resolve(files, workspace, native_file);
    luassert_always(resolve(files, workspace, native_file) == asset_path);
    luassert_always(resolve(files, workspace, child_path(desktop, "Menu with spaces")) == asset_path);
    luassert_always(files.external_mounts.size() == 1);
    luassert_always(asset_path.is_subpath_of(Path("/__GameGUIEditorExternal_2")));
    resolve(files, workspace, child_path(fixture, "Parent.json"));
    luassert_always(resolve(files, workspace, native_file) == asset_path);
    Path source_path = asset_path;
    source_path.append_extension("json");
    auto native_result = VFS::get_native_path(source_path);
    lupanic_if_failed(native_result);
    luassert_always(Path(native_result.get().c_str()) == native_file);
    luassert_always(!files.resolve_document_path(workspace, child_path(desktop, "Invalid.txt")).valid());
    luassert_always(!files.resolve_document_path(workspace, Path("Relative.json")).valid());
    luassert_always(!Internal::load_document_meta(asset_path, false).valid());
    lupanic_if_failed(Internal::load_document_meta(asset_path, true));

    Variant metadata = call(*service, CREATE_DOCUMENT_URL, Variant(VariantType::object));
    metadata = rename_root(*service, metadata, "First save");
    Variant params = params_for(metadata);
    params["path"] = asset_path.encode().c_str();
    metadata = call(*service, SAVE_AS_URL, params);
    luassert_always(!metadata["dirty"].boolean());
    luassert_always(get_file_attribute(native_file.encode().c_str()).valid());
    luassert_always(get_file_attribute(child_path(desktop, "Menu with spaces.meta").encode().c_str()).valid());

    // Ordinary Save writes to the same external file and clears the savepoint.
    metadata = rename_root(*service, metadata, "Second save");
    luassert_always(metadata["dirty"].boolean());
    metadata = call(*service, SAVE_URL, params_for(metadata));
    luassert_always(!metadata["dirty"].boolean());
    call(*service, CLOSE_DOCUMENT_URL, params_for(metadata));
    auto asset = Asset::get_asset_by_path(asset_path);
    lupanic_if_failed(asset);
    Guid saved_guid = Asset::get_asset_guid(asset.get());
    lupanic_if_failed(Asset::set_asset_data_unit_object(asset.get(), get_authoring_data_unit(), nullptr));
    lupanic_if_failed(Internal::load_document_meta(asset_path, false));
    params = Variant(VariantType::object);
    params["path"] = asset_path.encode().c_str();
    metadata = call(*service, OPEN_DOCUMENT_URL, params);
    Variant snapshot = call(*service, GET_SNAPSHOT_URL, params_for(metadata));
    auto decoded = decode_authoring_document(snapshot["document"]);
    lupanic_if_failed(decoded);
    luassert_always(decoded.get()->nodes[0].name == Name("Second save"));
    luassert_always(!metadata["dirty"].boolean());

    // A failed write must not rebind the document or clear its dirty flag.
    metadata = rename_root(*service, metadata, "Unsaved edit");
    Path blocked_file = child_path(desktop, "Blocked.json");
    lupanic_if_failed(create_dir(blocked_file.encode().c_str()));
    Path blocked_asset = resolve(files, workspace, blocked_file);
    params = params_for(metadata);
    params["path"] = blocked_asset.encode().c_str();
    auto failed_save = service->frontend()->invoke(SAVE_AS_URL, params);
    luassert_always(!failed_save.valid());
    snapshot = call(*service, GET_SNAPSHOT_URL, params_for(metadata));
    luassert_always(snapshot["dirty"].boolean());
    luassert_always(snapshot["asset_path"].str() == Name(asset_path.encode()));
    luassert_always(snapshot["asset_guid"] == metadata["asset_guid"]);
    lupanic_if_failed(delete_file(blocked_file.encode().c_str()));
    metadata = call(*service, SAVE_URL, params_for(snapshot));
    call(*service, CLOSE_DOCUMENT_URL, params_for(metadata));

    // Model files encountered for the first time in a fresh registry. Preserve their saved GUID.
    Path backup_json = child_path(desktop, "Backup.json");
    Path backup_meta = child_path(desktop, "Backup.meta");
    Path native_meta = child_path(desktop, "Menu with spaces.meta");
    lupanic_if_failed(copy_file(native_file.encode().c_str(), backup_json.encode().c_str()));
    lupanic_if_failed(copy_file(native_meta.encode().c_str(), backup_meta.encode().c_str()));
    lupanic_if_failed(Asset::delete_asset(asset.get()));
    lupanic_if_failed(move_file(backup_json.encode().c_str(), native_file.encode().c_str()));
    lupanic_if_failed(move_file(backup_meta.encode().c_str(), native_meta.encode().c_str()));
    lupanic_if_failed(Internal::load_document_meta(asset_path, true));
    asset = Asset::get_asset_by_path(asset_path);
    lupanic_if_failed(asset);
    luassert_always(Asset::get_asset_guid(asset.get()) == saved_guid);
    metadata = call(*service, CREATE_DOCUMENT_URL, Variant(VariantType::object));
    params = params_for(metadata);
    params["path"] = asset_path.encode().c_str();
    metadata = call(*service, SAVE_AS_URL, params);
    luassert_always(!metadata["dirty"].boolean());
    luassert_always(Asset::get_asset_guid(asset.get()) == saved_guid);
    call(*service, CLOSE_DOCUMENT_URL, params_for(metadata));

    // Remove only this test's assets and empty fixture directories.
    service.reset();
    lupanic_if_failed(Asset::delete_asset(asset.get()));
    auto blocked = Asset::get_asset_by_path(blocked_asset);
    lupanic_if_failed(blocked);
    lupanic_if_failed(Asset::delete_asset(blocked.get()));
    lupanic_if_failed(files.close());
    luassert_always(files.external_mounts.empty());
    luassert_always(VFS::unmount("/__GameGUIEditorExternal_2").errcode() == E_NOT_FOUND);
    lupanic_if_failed(VFS::delete_file("/__GameGUIEditorExternal_1"));
    lupanic_if_failed(VFS::unmount("/"));
    lupanic_if_failed(delete_file(desktop.encode().c_str()));
    lupanic_if_failed(delete_file(workspace.encode().c_str()));
    lupanic_if_failed(delete_file(fixture.encode().c_str()));
}
