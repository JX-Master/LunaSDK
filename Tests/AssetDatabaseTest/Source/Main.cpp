/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Main.cpp
* @author JXMaster
* @date 2026/9/4
*/
#include "TestTypes.hpp"
#include <Luna/Asset/Database.hpp>
#include <Luna/Pak/Pak.hpp>
#include <Luna/Runtime/Guid.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Random.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Serialization.hpp>
#include <Luna/VariantUtils/JSON.hpp>
#include <Luna/VFS/NativeFileSystem.hpp>
#include <Luna/VFS/PakFileSystem.hpp>
#include <Luna/VFS/VFS.hpp>
#include <cstdio>
#include "AssetDatabaseTest.meta.generated.hpp"

using namespace Luna;
using namespace Luna::Asset;

template <typename T> static T must(R<T> result)
{
    lupanic_if_failed(result);
    return move(result.get());
}
template <typename T> static void expect_error(R<T> result, ResultCode code)
{
    luassert_always(failed(result));
    if(unwrap_errcode(result.errcode()) != code)
    {
        printf("Unexpected error: %s\n", explain(result.errcode()));
        luassert_always(false);
    }
}
static Path child(const Path& root, const c8* name)
{
    Path result(root);
    result.append(Path(name));
    return result;
}
static void put(const Path& path, const c8* text)
{
    auto file = must(VFS::open_file(path, FileOpenFlag::write, FileCreationMode::create_always));
    lupanic_if_failed(file->write(text, strlen(text)));
}
static String text_at(const Path& path)
{
    auto file = must(VFS::open_file(path, FileOpenFlag::read, FileCreationMode::open_existing));
    auto bytes = must(load_file_data(file));
    return String((const c8*)bytes.data(), bytes.size());
}
static bool exists(const Path& path) { return succeeded(VFS::get_file_attribute(path)); }
static AssetMetadata record(const c8* path)
{
    AssetMetadata value;
    value.guid = random_guid();
    value.path = path;
    value.type = "DatabaseTest.Type";
    value.data_units.push_back({Name("Editor"), Name("Unregistered.Loader")});
    return value;
}
static Variant encode_record(const AssetMetadata& value)
{
    Variant data(VariantType::object);
    data["guid"] = must(serialize(value.guid));
    data["path"] = value.path.encode().c_str();
    data["type"] = value.type;
    data["data_units"] = must(serialize(value.data_units));
    return data;
}
static void write_snapshot(const Path& root, const Variant& records)
{
    Variant data(VariantType::object);
    data["format_version"] = Variant((u64)1);
    data["assets"] = records;
    auto text = VariantUtils::write_json(data);
    put(child(root, "assets.db"), text.c_str());
}
static Variant records_array(const AssetMetadata& value)
{
    Variant data(VariantType::array);
    data.push_back(encode_record(value));
    return data;
}
static Ref<IAssetDatabase> seed_database(const Path& root, const AssetMetadata& value)
{
    lupanic_if_failed(VFS::create_dir(root));
    auto database = must(create_file_database(root));
    lupanic_if_failed(database->write_record(value));
    lupanic_if_failed(database->flush());
    return database;
}
static void remove_tree(const Path& root)
{
    Vector<Pair<Path, bool>> children;
    {
        auto iter = must(VFS::open_dir(root));
        for(; iter->is_valid(); iter->move_next())
        {
            auto name = iter->get_filename();
            if(!strcmp(name, ".") || !strcmp(name, "..")) continue;
            children.push_back(make_pair(child(root, name), test_flags(iter->get_attributes(), FileAttributeFlag::directory)));
        }
    }
    for(const auto& item : children)
    {
        if(item.second) remove_tree(item.first);
        else lupanic_if_failed(VFS::delete_file(item.first));
    }
    lupanic_if_failed(VFS::delete_file(root));
}

static void discovery_and_reload(CountingFileSystem* storage)
{
    Path root("/DatabaseTest/discovery");
    auto hero = record("Characters/Hero");
    auto seeded = seed_database(root, hero);
    auto peer = record("Peer");
    lupanic_if_failed(seeded->write_record(peer));
    lupanic_if_failed(seeded->flush());
    // The payload deliberately does not exist. Discovery must still register the record.
    storage->read_files = storage->directories = 0;
    auto database = must(open_file_database(root, "assets.db", DatabaseMode::read_write));
    auto placeholder = get_asset(hero.guid);
    lupanic_if_failed(register_asset_database(database));
    luassert_always(storage->read_files == 1 && storage->directories == 0);
    luassert_always(must(get_asset_by_path(child(root, "Characters/Hero"))) == placeholder);
    lupanic_if_failed(load_asset_meta(placeholder));
    luassert_always(storage->read_files == 1 && storage->directories == 0);
    auto nested = must(new_sidecar_database(child(root, "Characters")));
    expect_error(register_asset_database(nested), E_ALREADY_EXISTS);

    AssetDataUnitDesc added{Name("Extra"), Name("Another.Loader")};
    lupanic_if_failed(add_asset_data_unit(placeholder, added));
    lupanic_if_failed(save_asset_meta(placeholder));
    luassert_always(database->is_dirty());
    expect_error(reload_asset_database(database), E_BUSY);
    lupanic_if_failed(database->flush());

    lupanic_if_failed(load_asset_data_unit_default_data(placeholder, Name()));
    auto original = must(database->read_record(hero.path));
    auto changed = original;
    changed.type = "Changed.Type";
    auto peer_changed = peer;
    peer_changed.type = "Peer.Changed";
    auto candidate_batch = records_array(peer_changed);
    candidate_batch.push_back(encode_record(changed));
    write_snapshot(root, candidate_batch);
    expect_error(reload_asset_database(database), E_ASSET_DATA_UNIT_BUSY);
    luassert_always(get_asset_type(placeholder) == original.type);
    luassert_always(must(database->read_record(hero.path)).type == original.type);
    luassert_always(get_asset_type(get_asset(peer.guid)) == peer.type);
    luassert_always(must(database->read_record(peer.path)).type == peer.type);
    expect_error(unregister_asset_database(database), E_ASSET_DATA_UNIT_BUSY);
    lupanic_if_failed(set_asset_data_unit_object(placeholder, Name(), nullptr));
    lupanic_if_failed(reload_asset_database(database));
    luassert_always(get_asset_type(placeholder) == changed.type);

    // Invalid late records must not partially overwrite earlier registry records.
    auto candidate = changed;
    candidate.type = "Would.Partially.Change";
    auto duplicate = records_array(candidate);
    auto second = encode_record(record("Second"));
    second["guid"] = must(serialize(hero.guid));
    duplicate.push_back(second);
    write_snapshot(root, duplicate);
    expect_error(reload_asset_database(database), E_ALREADY_EXISTS);
    luassert_always(get_asset_type(placeholder) == changed.type);
    auto bad_path = records_array(changed);
    bad_path[(usize)0]["path"] = "../Escape";
    write_snapshot(root, bad_path);
    expect_error(reload_asset_database(database), E_BAD_DATA);
    luassert_always(get_asset_type(placeholder) == changed.type);
    auto bad_guid = records_array(changed);
    bad_guid[(usize)0]["guid"] = Variant((u64)7);
    write_snapshot(root, bad_guid);
    expect_error(reload_asset_database(database), E_BAD_DATA);
    bad_guid[(usize)0]["guid"] = Variant(VariantType::array);
    bad_guid[(usize)0]["guid"].push_back(Variant((u64)7));
    write_snapshot(root, bad_guid);
    expect_error(reload_asset_database(database), E_BAD_DATA);
    put(child(root, "assets.db"), "{\"format_version\":4294967297,\"assets\":[]}");
    expect_error(reload_asset_database(database), E_BAD_DATA);
    luassert_always(get_asset_type(placeholder) == changed.type);
    auto main_only = records_array(changed);
    main_only[(usize)0].erase("data_units");
    write_snapshot(root, main_only);
    luassert_always(must(database->read_snapshot())[0].data_units.empty());
    write_snapshot(root, Variant(VariantType::array));
    lupanic_if_failed(reload_asset_database(database));
    luassert_always(get_asset_type(placeholder).empty());
    expect_error(get_asset_by_path(child(root, "Characters/Hero")), E_NOT_FOUND);
    lupanic_if_failed(unregister_asset_database(database));
    luassert_always(get_asset(hero.guid) == placeholder);
    puts("Metadata-only discovery, batch validation and reload passed.");
}

static void maintenance_and_failures(CountingFileSystem* storage)
{
    Path root("/DatabaseTest/writable");
    lupanic_if_failed(VFS::create_dir(root));
    auto database = must(create_file_database(root));
    lupanic_if_failed(register_asset_database(database));
    // This base name also matches assets.db: the shared metadata file must never be payload.
    auto asset = must(new_asset(child(root, "assets"), "DatabaseTest.Type"));
    put(child(root, "assets.payload"), "original payload");
    luassert_always(!exists(child(root, "assets.meta")));
    Vector<Name> files;
    lupanic_if_failed(get_asset_files(asset, files));
    luassert_always(files.size() == 1 && files[0] == "assets.payload");
    String old_database = text_at(child(root, "assets.db"));
    storage->fail_publish = true;
    expect_error(database->flush(), E_IO_ERROR);
    luassert_always(database->is_dirty());
    luassert_always(!text_at(child(root, "assets.db")).compare(old_database));
    storage->fail_publish = false;
    storage->reject_moves = true;
    expect_error(database->flush(), E_NOT_SUPPORTED);
    luassert_always(!text_at(child(root, "assets.db")).compare(old_database));
    storage->reject_moves = false;
    lupanic_if_failed(database->flush());
    lupanic_if_failed(move_asset(asset, child(root, "Moved")));
    expect_error(database->read_record("assets"), E_META_FILE_NOT_FOUND);
    luassert_always(must(database->read_record("Moved")).guid == get_asset_guid(asset));
    auto copy = must(copy_asset(asset, child(root, "Copied")));
    luassert_always(get_asset_guid(copy) != get_asset_guid(asset));
    luassert_always(!text_at(child(root, "Copied.payload")).compare("original payload"));
    lupanic_if_failed(delete_asset(copy));
    luassert_always(exists(child(root, "assets.db")));

    Path loose("/DatabaseTest/loose");
    lupanic_if_failed(VFS::create_dir(loose));
    auto sidecar = must(new_sidecar_database(loose));
    lupanic_if_failed(register_asset_database(sidecar));
    Guid guid = get_asset_guid(asset);
    // Failure after payload transfer must restore both the source payload and registry path.
    storage->fail_sidecar_publish = true;
    expect_error(move_asset(asset, child(loose, "Hero")), E_IO_ERROR);
    expect_error(copy_asset(asset, child(loose, "Copy")), E_IO_ERROR);
    luassert_always(get_asset_path(asset) == child(root, "Moved"));
    luassert_always(must(database->read_record("Moved")).guid == guid);
    luassert_always(exists(child(root, "Moved.payload")));
    luassert_always(!exists(child(loose, "Hero.payload")) && !exists(child(loose, "Copy.payload")));
    luassert_always(!exists(child(loose, "Hero.meta")) && !exists(child(loose, "Copy.meta")));
    storage->fail_sidecar_publish = false;
    lupanic_if_failed(move_asset(asset, child(loose, "Hero")));
    luassert_always(exists(child(loose, "Hero.meta")));
    expect_error(database->read_record("Moved"), E_META_FILE_NOT_FOUND);
    lupanic_if_failed(move_asset(asset, child(root, "Returned")));
    luassert_always(!exists(child(loose, "Hero.meta")));
    luassert_always(get_asset_guid(asset) == guid);
    // Explicit path edits must relocate the stored record on save.
    lupanic_if_failed(VFS::move_file(child(root, "Returned.payload"), child(root, "Renamed.payload")));
    lupanic_if_failed(set_asset_path(asset, child(root, "Renamed")));
    lupanic_if_failed(save_asset_meta(asset));
    expect_error(database->read_record("Returned"), E_META_FILE_NOT_FOUND);
    luassert_always(must(database->read_record("Renamed")).guid == guid);
    lupanic_if_failed(flush_asset_databases());
    lupanic_if_failed(unregister_asset_database(sidecar));
    lupanic_if_failed(unregister_asset_database(database));

    auto readonly = must(open_file_database(root));
    lupanic_if_failed(register_asset_database(readonly));
    asset = must(get_asset_by_path(child(root, "Renamed")));
    expect_error(new_asset(child(root, "Forbidden"), "DatabaseTest.Type"), E_NOT_SUPPORTED);
    expect_error(save_asset_meta(asset), E_NOT_SUPPORTED);
    expect_error(move_asset(asset, child(root, "Forbidden")), E_NOT_SUPPORTED);
    expect_error(delete_asset(asset), E_NOT_SUPPORTED);
    luassert_always(!text_at(child(root, "Renamed.payload")).compare("original payload"));
    auto readonly_copy = must(copy_asset(asset, child(loose, "ReadOnlyCopy")));
    lupanic_if_failed(delete_asset(readonly_copy));
    auto path_only = must(new_asset(Path(), "DatabaseTest.Type", false));
    lupanic_if_failed(set_asset_path(path_only, child(root, "PathOnly")));
    put(child(root, "PathOnly.payload"), "preserve despite unsaved runtime path");
    expect_error(delete_asset(path_only), E_NOT_SUPPORTED);
    expect_error(move_asset(path_only, child(loose, "PathOnly")), E_NOT_SUPPORTED);
    luassert_always(exists(child(root, "PathOnly.payload")));
    lupanic_if_failed(set_asset_path(path_only, Path()));
    lupanic_if_failed(VFS::delete_file(child(root, "PathOnly.payload")));
    lupanic_if_failed(unregister_asset_database(readonly));
    puts("Metadata maintenance, safe replacement, retry and read-only access passed.");
}

static void conflicts_and_conversion(CountingFileSystem* storage)
{
    auto first = record("First");
    auto database = seed_database("/DatabaseTest/owner", first);
    lupanic_if_failed(register_asset_database(database));
    auto second = record("Second");
    second.guid = first.guid;
    auto conflict = seed_database("/DatabaseTest/conflict", second);
    expect_error(register_asset_database(conflict), E_ALREADY_EXISTS);
    expect_error(get_asset_by_path("/DatabaseTest/conflict/Second"), E_NOT_FOUND);
    lupanic_if_failed(unregister_asset_database(database));

    Path source_root("/DatabaseTest/convert");
    Path destination_root("/DatabaseTest/exported");
    lupanic_if_failed(VFS::create_dir(source_root));
    lupanic_if_failed(VFS::create_dir(child(source_root, "Nested")));
    lupanic_if_failed(VFS::create_dir(destination_root));
    auto sidecar = must(new_sidecar_database(source_root));
    auto original = record("Nested/Legacy");
    lupanic_if_failed(sidecar->write_record(original));
    auto before = text_at(child(source_root, "Nested/Legacy.meta"));
    lupanic_if_failed(export_asset_database(sidecar, destination_root));
    expect_error(export_asset_database(sidecar, destination_root), E_ALREADY_EXISTS);
    luassert_always(!text_at(child(source_root, "Nested/Legacy.meta")).compare(before));
    auto exported = must(open_file_database(destination_root));
    auto saved = must(exported->read_record(original.path));
    luassert_always(saved.guid == original.guid && saved.data_units.size() == original.data_units.size());
    lupanic_if_failed(register_asset_database(exported));
    luassert_always(get_asset_guid(must(get_asset_by_path("/DatabaseTest/exported/Nested/Legacy"))) == original.guid);
    lupanic_if_failed(unregister_asset_database(exported));
    // An empty relative VFS root covers relative paths, but never owns dynamic assets without paths.
    lupanic_if_failed(VFS::mount(storage, Path()));
    auto relative = must(open_file_database(Path(), "exported/assets.db"));
    lupanic_if_failed(register_asset_database(relative));
    luassert_always(get_asset_guid(must(get_asset_by_path("Nested/Legacy"))) == original.guid);
    auto dynamic_asset = must(new_asset(Path(), "DatabaseTest.Type", false));
    lupanic_if_failed(unregister_asset_database(relative));
    luassert_always(get_asset_type(dynamic_asset) == "DatabaseTest.Type");
    lupanic_if_failed(VFS::unmount(Path()));
    puts("Database ownership, independent export and relative roots passed.");
}

static void pak_round_trip(const c8* native_directory)
{
    auto source = must(open_file_database("/DatabaseTest/exported"));
    auto records = must(source->get_records());
    auto package = must(Pak::new_pak());
    {
        auto input = must(VFS::open_file("/DatabaseTest/exported/assets.db", FileOpenFlag::read, FileCreationMode::open_existing));
        auto bytes = must(load_file_data(input));
        auto output = must(package->open_file("assets.db", FileOpenFlag::write, FileCreationMode::create_new));
        lupanic_if_failed(output->write(bytes.data(), bytes.size()));
    }
    Path native_path = child(Path(native_directory), "game.pak");
    {
        auto output = must(Luna::open_file(native_path.encode().c_str(), FileOpenFlag::read | FileOpenFlag::write, FileCreationMode::create_new));
        lupanic_if_failed(package->flush(output));
    }
    package = nullptr;
    auto file_system = must(VFS::new_pak_file_system(native_path.encode().c_str(), Pak::OpenMode::read_write));
    lupanic_if_failed(VFS::mount(file_system, "/Packed"));
    auto database = must(open_file_database("/Packed", "assets.db", DatabaseMode::read_write));
    lupanic_if_failed(register_asset_database(database));
    luassert_always(get_asset_guid(must(get_asset_by_path("/Packed/Nested/Legacy"))) == records[0].guid);
    auto added = must(new_asset("/Packed/Added", "DatabaseTest.Type"));
    put("/Packed/Added.payload", "package payload");
    luassert_always(!exists("/Packed/Added.meta"));
    {
        auto reader = must(VFS::open_file("/Packed/assets.db", FileOpenFlag::read, FileCreationMode::open_existing));
        expect_error(database->flush(), E_BUSY);
        luassert_always(database->is_dirty());
    }
    lupanic_if_failed(database->flush());
    // Database flush updates the mounted view; the native package is still the previous version.
    {
        auto input = must(Luna::open_file(native_path.encode().c_str(), FileOpenFlag::read, FileCreationMode::open_existing));
        auto old = must(Pak::open_pak(input));
        expect_error(old->get_file_attribute("Added.payload"), E_NOT_FOUND);
    }
    lupanic_if_failed(file_system->flush());
    lupanic_if_failed(unregister_asset_database(database));
    database = nullptr;
    lupanic_if_failed(VFS::unmount("/Packed"));
    file_system = nullptr;
    file_system = must(VFS::new_pak_file_system(native_path.encode().c_str()));
    lupanic_if_failed(VFS::mount(file_system, "/Relocated"));
    database = must(open_file_database("/Relocated"));
    lupanic_if_failed(register_asset_database(database));
    luassert_always(must(get_asset_by_path("/Relocated/Added")) == added);
    luassert_always(!text_at("/Relocated/Added.payload").compare("package payload"));
    lupanic_if_failed(unregister_asset_database(database));
    database = nullptr;
    lupanic_if_failed(VFS::unmount("/Relocated"));
    puts("Pak database persistence, flush ordering and mount relocation passed.");
}

int main()
{
    lupanic_if_failed(Luna::init());
    lupanic_if_failed(add_modules({module_asset()}));
    lupanic_if_failed(init_modules());
    Meta::register_AssetDatabaseTest_types();
    AssetLoaderDesc loader;
    loader.name = "DatabaseTest.Loader";
    loader.on_load_asset_data_unit_default_data = [](object_t, asset_t, const Name&) -> R<ObjRef>
    { return ObjRef(new_object<DatabaseTestData>()); };
    register_asset_loader(loader);
    register_asset_type({Name("DatabaseTest.Type"), loader.name});
    c8 guid[GUID_STRING_LENGTH + 1]{};
    lupanic_if_failed(encode_guid(random_guid(), guid, GUID_STRING_LENGTH));
    String directory("AssetDatabaseTest-");
    directory.append(guid);
    lupanic_if_failed(Luna::create_dir(directory.c_str()));
    {
        auto storage = new_object<CountingFileSystem>();
        storage->storage = must(VFS::new_native_file_system(directory.c_str()));
        lupanic_if_failed(VFS::mount(storage, "/DatabaseTest"));
        discovery_and_reload(storage);
        maintenance_and_failures(storage);
        conflicts_and_conversion(storage);
        pak_round_trip(directory.c_str());
        // Metadata shutdown flushes registered dirty databases before VFS shutdown.
        Path closing_root("/DatabaseTest/closing");
        lupanic_if_failed(VFS::create_dir(closing_root));
        auto closing = must(create_file_database(closing_root));
        lupanic_if_failed(register_asset_database(closing));
        must(new_asset(child(closing_root, "SavedAtClose"), "DatabaseTest.Type"));
        Asset::close();
        auto reopened = must(open_file_database(closing_root));
        must(reopened->read_record("SavedAtClose"));
        remove_tree("/DatabaseTest/discovery");
        remove_tree("/DatabaseTest/writable");
        remove_tree("/DatabaseTest/loose");
        remove_tree("/DatabaseTest/owner");
        remove_tree("/DatabaseTest/conflict");
        remove_tree("/DatabaseTest/convert");
        remove_tree("/DatabaseTest/exported");
        remove_tree(closing_root);
        lupanic_if_failed(VFS::delete_file("/DatabaseTest/game.pak"));
        lupanic_if_failed(VFS::unmount("/DatabaseTest"));
    }
    lupanic_if_failed(Luna::delete_file(directory.c_str()));
    Luna::close();
    puts("AssetDatabaseTest passed.");
    return 0;
}
