/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Main.cpp
* @author JXMaster
* @date 2026/9/4
*/
#include "TestStorage.hpp"
#include "VFSPakTest.meta.generated.hpp"
#include <Luna/VFS/VFS.hpp>
#include <Luna/VFS/NativeFileSystem.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Random.hpp>
#include <Luna/Runtime/Guid.hpp>
#include <cstdio>

using namespace Luna;

template <typename _Ty>
static _Ty must(R<_Ty> result)
{
    lupanic_if_failed(result);
    return move(result.get());
}

template <typename _Ty>
static void expect_error(R<_Ty> result, ResultCode code)
{
    luassert_always(failed(result));
    if(unwrap_errcode(result.errcode()) != code)
    {
        printf("Unexpected error: %s\n", explain(result.errcode()));
        luassert_always(false);
    }
}

static bool equal_bytes(const Vector<byte_t>& left, const Vector<byte_t>& right)
{
    return left.size() == right.size() && (left.empty() || !memcmp(left.data(), right.data(), left.size()));
}

static Vector<byte_t> read_all(IStream* stream)
{
    Vector<byte_t> result;
    byte_t buffer[4096];
    for(;;)
    {
        usize count = 0;
        lupanic_if_failed(stream->read(buffer, sizeof(buffer), &count));
        if(!count) break;
        result.insert(result.end(), buffer, buffer + count);
    }
    return result;
}

static void expect_text(IStream* stream, const c8* text)
{
    auto bytes = read_all(stream);
    luassert_always(bytes.size() == strlen(text) && (bytes.empty() || !memcmp(bytes.data(), text, bytes.size())));
}

static void expect_vfs_text(const c8* path, const c8* text)
{
    auto file = must(VFS::open_file(path, FileOpenFlag::read, FileCreationMode::open_existing));
    expect_text(file, text);
}

static void put_vfs(const c8* path, const c8* text)
{
    auto file = must(VFS::open_file(path, FileOpenFlag::write, FileCreationMode::create_always));
    lupanic_if_failed(file->write(text, strlen(text)));
}

static Ref<VfsTestStorage> make_storage(const c8* text = "original contents")
{
    auto storage = new_object<VfsTestStorage>();
    storage->published = new_object<VfsTestStream>();
    auto package = must(Pak::new_pak());
    lupanic_if_failed(package->create_dir("data"));
    {
        auto file = must(package->open_file("data/seed", FileOpenFlag::write, FileCreationMode::create_new));
        lupanic_if_failed(file->write(text, strlen(text)));
    }
    lupanic_if_failed(package->flush(storage->published));
    return storage;
}

static Ref<VFS::IFileSystem> mount_storage(VfsTestStorage* storage, const c8* path,
    Pak::OpenMode mode = Pak::OpenMode::read_write, const Pak::Options& options = Pak::Options())
{
    auto instance = must(VFS::new_pak_file_system(storage, mode, options));
    lupanic_if_failed(VFS::mount(instance, path));
    return instance;
}

static void expect_published_text(VfsTestStorage* storage, const c8* path, const c8* text)
{
    auto package = must(Pak::open_pak(storage->published));
    auto file = must(package->open_file(path, FileOpenFlag::read, FileCreationMode::open_existing));
    expect_text(file, text);
}

static String child_path(const c8* parent, const c8* child)
{
    Path path(parent);
    path.append(Path(child));
    return path.encode();
}

static void create_native_package(const c8* path)
{
    auto storage = make_storage();
    auto file = must(Luna::open_file(path, FileOpenFlag::write, FileCreationMode::create_new));
    lupanic_if_failed(file->write(storage->published->bytes.data(), storage->published->bytes.size()));
}

static Vector<byte_t> read_native(const c8* path)
{
    auto file = must(Luna::open_file(path, FileOpenFlag::read, FileCreationMode::open_existing));
    return read_all(file);
}

static Vector<String> temporary_outputs(const c8* directory)
{
    Vector<String> result;
    auto iter = must(Luna::open_dir(directory));
    for(; iter->is_valid(); iter->move_next())
    {
        if(!strncmp(iter->get_filename(), ".luna-pak-", 10)) result.push_back(child_path(directory, iter->get_filename()));
    }
    return result;
}

static void native_mount_test(const c8* directory)
{
    auto path = child_path(directory, "native.pak");
    create_native_package(path.c_str());
    auto original = read_native(path.c_str());
    {
        auto native = must(VFS::new_native_file_system(directory));
        luassert_always(test_flags(must(native->get_file_attribute(Path())).attributes, FileAttributeFlag::directory));
        expect_error(native->get_file_attribute("/native.pak"), E_BAD_ARGUMENTS);
        expect_error(native->get_file_attribute("../escape"), E_BAD_ARGUMENTS);
        auto direct = must(native->open_file("native.pak", FileOpenFlag::read, FileCreationMode::open_existing));
        native = nullptr; // A Runtime native file independently owns its open handle.
        luassert_always(equal_bytes(original, read_all(direct)));
    }
    auto read_only = must(VFS::new_pak_file_system(path.c_str()));
    lupanic_if_failed(VFS::mount(read_only, "/Read"));
    expect_vfs_text("/Read/data/seed", "original contents");
    luassert_always(test_flags(must(VFS::get_file_attribute("/Read")).attributes, FileAttributeFlag::directory));
    expect_error(VFS::create_dir("/Read/no"), E_ACCESS_DENIED);
    expect_error(VFS::delete_file("/Read/data/seed"), E_ACCESS_DENIED);
    expect_error(VFS::copy_file("/Read/data/seed", "/Read/copy"), E_ACCESS_DENIED);
    expect_error(VFS::move_file("/Read/data", "/Read/moved"), E_ACCESS_DENIED);
    expect_error(VFS::open_file("/Read/new", FileOpenFlag::write, FileCreationMode::create_new), E_ACCESS_DENIED);
    expect_error(VFS::get_native_path("/Read/data/seed"), E_NOT_SUPPORTED);
    lupanic_if_failed(read_only->flush());
    lupanic_if_failed(VFS::unmount("/Read"));
    read_only = nullptr;
    luassert_always(equal_bytes(original, read_native(path.c_str())));

    Pak::Options options;
    options.compression = Pak::CompressionMethod::deflate;
    options.compression_level = 6;
    auto file_system = must(VFS::new_pak_file_system(path.c_str(), Pak::OpenMode::read_write, options));
    lupanic_if_failed(VFS::mount(file_system, "/Edit"));
    lupanic_if_failed(VFS::create_dir("/Edit/empty"));
    {
        auto file = must(VFS::open_file("/Edit/data/new", FileOpenFlag::read | FileOpenFlag::write, FileCreationMode::create_new));
        lupanic_if_failed(file->write("abcdefgh", 8));
        lupanic_if_failed(file->seek(2, SeekMode::begin));
        lupanic_if_failed(file->write("XY", 2));
        lupanic_if_failed(file->set_size(6));
        luassert_always(must(file->tell()) == 4 && file->get_size() == 6);
        lupanic_if_failed(file->seek(0, SeekMode::begin));
        expect_text(file, "abXYef");
        file->flush();
        expect_error(file_system->flush(), E_BUSY);
        expect_error(VFS::unmount("/Edit"), E_BUSY);
        luassert_always(equal_bytes(original, read_native(path.c_str())));
    }
    lupanic_if_failed(VFS::copy_file("/Edit/data/new", "/Edit/copy"));
    lupanic_if_failed(VFS::move_file("/Edit/data", "/Edit/renamed"));
    expect_error(VFS::delete_file("/Edit/renamed"), E_DIRECTORY_NOT_EMPTY);
    lupanic_if_failed(VFS::delete_file("/Edit/empty"));
    auto iter = must(VFS::open_dir("/Edit"));
    luassert_always(iter->is_valid() && !strcmp(iter->get_filename(), "copy"));
    expect_error(VFS::unmount("/Edit"), E_BUSY);
    // Factory-time resolution must survive changes to the process current directory.
    const c8* previous_directory = get_current_dir();
    lupanic_if_failed(set_current_dir(directory));
    lupanic_if_failed(file_system->flush());
    lupanic_if_failed(set_current_dir(previous_directory));
    release_current_dir(previous_directory);
    luassert_always(!equal_bytes(original, read_native(path.c_str())));
    luassert_always(temporary_outputs(directory).empty());
    luassert_always(iter->move_next() && !strcmp(iter->get_filename(), "renamed"));
    iter = nullptr;
    expect_vfs_text("/Edit/renamed/new", "abXYef");
    put_vfs("/Edit/renamed/new", "second edit");
    lupanic_if_failed(VFS::unmount("/Edit"));
    {
        auto input = must(Luna::open_file(path.c_str(), FileOpenFlag::read, FileCreationMode::open_existing));
        auto package = must(Pak::open_pak(input));
        auto file = must(package->open_file("renamed/new", FileOpenFlag::read, FileCreationMode::open_existing));
        expect_text(file, "second edit");
        luassert_always(must(package->get_file_compression("renamed/new")) == Pak::CompressionMethod::deflate);
        luassert_always(must(package->get_file_compression("copy")) == Pak::CompressionMethod::deflate);
        luassert_always(must(package->get_file_compression("renamed/seed")) == Pak::CompressionMethod::store);
    }
    expect_error(VFS::mount(nullptr, "/Invalid"), E_BAD_ARGUMENTS);
    expect_error(VFS::new_pak_file_system((const c8*)nullptr), E_BAD_ARGUMENTS);
    expect_error(VFS::new_pak_file_system((VFS::IPakStorage*)nullptr), E_BAD_ARGUMENTS);
    expect_error(VFS::new_pak_file_system(directory), E_IS_DIRECTORY);
    expect_error(VFS::new_pak_file_system(child_path(directory, "missing").c_str()), E_NOT_FOUND);
    expect_error(VFS::new_pak_file_system(path.c_str(), (Pak::OpenMode)99), E_BAD_ARGUMENTS);
    expect_error(VFS::new_native_file_system(path.c_str()), E_NOT_DIRECTORY);
    expect_error(VFS::new_native_file_system(nullptr), E_BAD_ARGUMENTS);
    file_system = nullptr;
    lupanic_if_failed(Luna::delete_file(path.c_str()));
    printf("Native mounting, deferred saves and handle lifetimes passed.\n");
}

static void storage_failure_test()
{
    auto storage = make_storage();
    auto original = storage->published->bytes;
    auto file_system = mount_storage(storage, "/Memory");
    put_vfs("/Memory/data/seed", "pending edit");
    const OutputFault faults[] = {OutputFault::create, OutputFault::null_stream, OutputFault::alias_source,
        OutputFault::write, OutputFault::resize, OutputFault::reopen};
    const ResultCode errors[] = {E_ACCESS_DENIED, E_BAD_DATA, E_BAD_ARGUMENTS, E_IO_ERROR, E_IO_ERROR, E_IO_ERROR};
    for(usize i = 0; i < sizeof(faults) / sizeof(faults[0]); ++i)
    {
        storage->fault = faults[i];
        expect_error(file_system->flush(), errors[i]);
        luassert_always(equal_bytes(original, storage->published->bytes) && !storage->publish_count);
        expect_vfs_text("/Memory/data/seed", "pending edit");
        put_vfs("/Memory/still-editable", "staged");
    }
    storage->fault = OutputFault::none;
    storage->fail_publish = true;
    expect_error(file_system->flush(), E_IO_ERROR);
    luassert_always(strstr(get_error().message.c_str(), storage->failure_message));
    usize outputs = storage->output_count;
    auto completed = storage->last_output;
    auto completed_bytes = completed->bytes;
    luassert_always(equal_bytes(original, storage->published->bytes));
    expect_vfs_text("/Memory/data/seed", "pending edit");
    expect_error(VFS::create_dir("/Memory/blocked"), E_BUSY);
    expect_error(VFS::delete_file("/Memory/data/seed"), E_BUSY);
    expect_error(VFS::copy_file("/Memory/data/seed", "/Memory/copy"), E_BUSY);
    expect_error(VFS::move_file("/Memory/data", "/Memory/moved"), E_BUSY);
    expect_error(VFS::open_file("/Memory/new", FileOpenFlag::write, FileCreationMode::create_new), E_BUSY);
    {
        auto reader = must(VFS::open_file("/Memory/data/seed", FileOpenFlag::read, FileCreationMode::open_existing));
        expect_error(file_system->flush(), E_BUSY);
    }
    expect_error(VFS::unmount("/Memory"), E_IO_ERROR);
    luassert_always(storage->output_count == outputs && storage->last_output == completed);
    luassert_always(equal_bytes(completed_bytes, completed->bytes));
    storage->fail_publish = false;
    lupanic_if_failed(file_system->flush());
    luassert_always(storage->output_count == outputs && storage->published == completed);
    expect_published_text(storage, "data/seed", "pending edit");
    lupanic_if_failed(file_system->flush());
    luassert_always(storage->output_count == outputs);
    put_vfs("/Memory/data/seed", "after retry");
    lupanic_if_failed(VFS::unmount("/Memory"));
    expect_published_text(storage, "data/seed", "after retry");
    file_system = nullptr;
    file_system = mount_storage(storage, "/Memory", Pak::OpenMode::read);
    outputs = storage->output_count;
    lupanic_if_failed(VFS::unmount("/Memory"));
    luassert_always(storage->output_count == outputs);
    printf("Output failures and publication retry passed.\n");
}

static void routing_and_flush_all_test()
{
    auto first = make_storage("nested");
    auto second = make_storage("parent");
    auto third = make_storage("third");
    mount_storage(first, "/Root/Child");
    mount_storage(second, "/Root");
    mount_storage(third, "Root");
    expect_vfs_text("/Root/Child/data/seed", "nested");
    expect_vfs_text("/Root/data/seed", "parent");
    expect_vfs_text("Root/data/seed", "third");
    expect_error(VFS::get_file_attribute("OtherRoot:/Root/data/seed"), E_NOT_FOUND);
    expect_error(VFS::remount("/Root", "/Root/Child"), E_ALREADY_EXISTS);
    auto duplicate = must(VFS::new_pak_file_system(first));
    expect_error(VFS::mount(duplicate, "/Root"), E_ALREADY_EXISTS);
    Path parents("../../escape");
    luassert_always(parents.size() == 3 && !strcmp(parents[0].c_str(), ".."));
    expect_error(VFS::get_file_attribute(parents), E_BAD_ARGUMENTS);
    Path malformed("/Root");
    malformed.push_back(Name("a/b"));
    expect_error(VFS::get_file_attribute(malformed), E_BAD_ARGUMENTS);
    lupanic_if_failed(VFS::remount("Root", "/Third"));
    expect_error(VFS::get_file_attribute("Root/data/seed"), E_NOT_FOUND);
    put_vfs("/Root/Child/new", "first edit");
    put_vfs("/Root/new", "second edit");
    put_vfs("/Third/new", "third edit");
    first->fail_publish = true;
    second->fail_publish = true;
    first->failure_message = "first mount failed";
    second->failure_message = "second mount failed";
    expect_error(VFS::flush_all(), E_IO_ERROR);
    luassert_always(!strcmp(get_error().message.c_str(), "first mount failed"));
    luassert_always(first->publish_count == 1 && second->publish_count == 1 && third->publish_count == 1);
    expect_published_text(third, "new", "third edit");
    first->fail_publish = false;
    second->fail_publish = false;
    lupanic_if_failed(VFS::flush_all());
    luassert_always(first->output_count == 1 && second->output_count == 1 && third->output_count == 1);
    lupanic_if_failed(VFS::unmount("/Root/Child"));
    lupanic_if_failed(VFS::unmount("/Root"));
    lupanic_if_failed(VFS::unmount("/Third"));
    printf("Routing, remount and flush-all failure reporting passed.\n");
}

static void cross_mount_test(const c8* directory)
{
    auto source = make_storage("cross-mount payload");
    auto destination = make_storage();
    mount_storage(source, "/Source");
    Pak::Options options;
    options.compression = Pak::CompressionMethod::deflate;
    auto destination_fs = mount_storage(destination, "/Destination", Pak::OpenMode::read_write, options);
    auto native_fs = must(VFS::new_native_file_system(directory));
    lupanic_if_failed(VFS::mount(native_fs, "/Native"));
    lupanic_if_failed(native_fs->create_dir("nested"));
    auto nested_fs = must(VFS::new_native_file_system(child_path(directory, "nested").c_str()));
    lupanic_if_failed(VFS::mount(nested_fs, "/NativeNested"));
    lupanic_if_failed(VFS::create_dir("/Native/moving-dir"));
    put_vfs("/Native/moving-dir/child", "native directory move");
    lupanic_if_failed(VFS::move_file("/Native/moving-dir", "/NativeNested/moved-dir"));
    expect_vfs_text("/NativeNested/moved-dir/child", "native directory move");
    lupanic_if_failed(VFS::copy_file("/NativeNested/moved-dir/child", "/Native/native-copy"));
    expect_vfs_text("/Native/native-copy", "native directory move");
    lupanic_if_failed(VFS::delete_file("/Native/native-copy"));
    lupanic_if_failed(VFS::delete_file("/NativeNested/moved-dir/child"));
    lupanic_if_failed(VFS::delete_file("/NativeNested/moved-dir"));
    lupanic_if_failed(VFS::unmount("/NativeNested"));
    lupanic_if_failed(native_fs->delete_file("nested"));
    lupanic_if_failed(VFS::copy_file("/Source/data/seed", "/Destination/copied"));
    lupanic_if_failed(VFS::copy_file("/Source/data/seed", "/Native/copied"));
    lupanic_if_failed(VFS::copy_file("/Native/copied", "/Destination/from-native"));
    expect_vfs_text("/Destination/from-native", "cross-mount payload");
    expect_error(VFS::copy_file("/Source/data/seed", "/Native/copied"), E_ALREADY_EXISTS);
    expect_error(VFS::copy_file("/Source/missing", "/Native/copied"), E_NOT_FOUND);
    expect_vfs_text("/Native/copied", "cross-mount payload");
    expect_error(VFS::copy_file("/Native/copied", "/Destination/copied"), E_ALREADY_EXISTS);
    expect_vfs_text("/Destination/copied", "cross-mount payload");
    expect_error(VFS::copy_file("/Source/data", "/Native/directory-copy"), E_IS_DIRECTORY);
    lupanic_if_failed(VFS::move_file("/Native/copied", "/Destination/moved"));
    expect_error(VFS::get_file_attribute("/Native/copied"), E_NOT_FOUND);
    lupanic_if_failed(VFS::move_file("/Source/data/seed", "/Destination/moved-again"));
    expect_error(VFS::get_file_attribute("/Source/data/seed"), E_NOT_FOUND);
    lupanic_if_failed(destination_fs->flush());
    {
        auto package = must(Pak::open_pak(destination->published));
        luassert_always(must(package->get_file_compression("copied")) == Pak::CompressionMethod::deflate);
    }
    // A staging size failure must remove only the newly created partial file.
    auto small = make_storage();
    options.max_memory_file_size = 4;
    mount_storage(small, "/Small", Pak::OpenMode::read_write, options);
    auto copied = VFS::copy_file("/Destination/copied", "/Small/partial");
    luassert_always(failed(copied));
    expect_error(VFS::get_file_attribute("/Small/partial"), E_NOT_FOUND);
    expect_vfs_text("/Small/data/seed", "original contents");
    lupanic_if_failed(VFS::unmount("/Small"));
    lupanic_if_failed(VFS::unmount("/Source"));
    lupanic_if_failed(VFS::unmount("/Destination"));
    lupanic_if_failed(VFS::unmount("/Native"));
    printf("Native/Pak and Pak/Pak copies and moves passed.\n");
}

static void instance_lifetime_test()
{
    FileSystemProbe probe, output_probe;
    probe.identity = 11;
    auto instance = new_object<TestFileSystem>();
    instance->probe = &probe;
    // Mount only attaches the object. It does not flush, recreate or replace it.
    lupanic_if_failed(VFS::mount(instance, "/First"));
    lupanic_if_failed(VFS::mount(instance, "/Alias"));
    luassert_always(!probe.flush_count && !probe.destroyed);
    luassert_always(must(VFS::get_file_attribute("/Alias/file")).size == 11);
    lupanic_if_failed(VFS::flush_all());
    luassert_always(probe.flush_count == 1);
    probe.fail_flush = true;
    expect_error(VFS::flush_all(), E_IO_ERROR);
    luassert_always(probe.flush_count == 2); // Aliases are attempted only once, even on error.
    expect_error(VFS::unmount("/First"), E_IO_ERROR);
    luassert_always(must(VFS::get_file_attribute("/First/file")).size == 11);
    probe.fail_flush = false;
    lupanic_if_failed(VFS::unmount("/First"));
    luassert_always(!probe.destroyed);
    instance->file = new_object<VfsTestStream>();
    instance->file->bytes.resize(70001);
    for(usize i = 0; i < instance->file->bytes.size(); ++i) instance->file->bytes[i] = (byte_t)(i % 251);
    instance->file->chunk_size = 11;
    auto output = new_object<TestFileSystem>();
    output->probe = &output_probe;
    lupanic_if_failed(VFS::mount(output, "/ShortOutput"));
    lupanic_if_failed(VFS::copy_file("/Alias/file", "/ShortOutput/file"));
    luassert_always(equal_bytes(output->file->bytes, instance->file->bytes));
    auto destination = make_storage();
    mount_storage(destination, "/ShortDestination");
    lupanic_if_failed(VFS::copy_file("/Alias/file", "/ShortDestination/file"));
    {
        auto copied = must(VFS::open_file("/ShortDestination/file", FileOpenFlag::read, FileCreationMode::open_existing));
        luassert_always(equal_bytes(read_all(copied), instance->file->bytes));
    }
    lupanic_if_failed(VFS::unmount("/ShortOutput"));
    luassert_always(!output_probe.destroyed); // The caller still owns the instance.
    output = nullptr;
    luassert_always(output_probe.destroyed == 1);
    lupanic_if_failed(VFS::unmount("/ShortDestination"));
    instance = nullptr; // The remaining mount is now the only owner.
    luassert_always(!probe.destroyed);
    lupanic_if_failed(VFS::unmount("/Alias"));
    luassert_always(probe.destroyed == 1);
    printf("Instance ownership, alias deduplication and short I/O passed.\n");
}

static void direct_pak_test()
{
    auto storage = make_storage();
    auto instance = must(VFS::new_pak_file_system(storage, Pak::OpenMode::read_write));
    auto original = storage->published->bytes;
    luassert_always(test_flags(must(instance->get_file_attribute(Path())).attributes, FileAttributeFlag::directory));
    expect_error(instance->get_file_attribute("/data/seed"), E_BAD_ARGUMENTS);
    expect_error(instance->get_file_attribute("../escape"), E_BAD_ARGUMENTS);
    expect_error(instance->get_file_attribute("Root:data/seed"), E_BAD_ARGUMENTS);
    {
        auto file = must(instance->open_file("data/direct", FileOpenFlag::write, FileCreationMode::create_new));
        lupanic_if_failed(file->write("direct edit", 11));
        expect_error(instance->flush(), E_BUSY);
    }
    lupanic_if_failed(instance->flush()); // No mount is involved.
    expect_published_text(storage, "data/direct", "direct edit");
    lupanic_if_failed(VFS::mount(instance, "/SharedA"));
    lupanic_if_failed(VFS::mount(instance, "/SharedB"));
    put_vfs("/SharedA/data/direct", "shared edit");
    expect_vfs_text("/SharedB/data/direct", "shared edit");
    {
        auto direct = must(instance->open_file("data/direct", FileOpenFlag::read, FileCreationMode::open_existing));
        expect_error(VFS::unmount("/SharedA"), E_BUSY);
        expect_error(instance->flush(), E_BUSY);
    }
    {
        auto alias_file = must(VFS::open_file("/SharedA/data/direct", FileOpenFlag::read, FileCreationMode::open_existing));
        expect_error(instance->flush(), E_BUSY);
        expect_error(VFS::unmount("/SharedB"), E_BUSY);
    }
    // Instance identity selects the in-filesystem operation even through another alias.
    lupanic_if_failed(VFS::copy_file("/SharedA/data/direct", "/SharedB/copy"));
    lupanic_if_failed(VFS::move_file("/SharedA/data", "/SharedB/renamed"));
    expect_vfs_text("/SharedA/renamed/direct", "shared edit");
    lupanic_if_failed(VFS::unmount("/SharedA"));
    expect_vfs_text("/SharedB/renamed/direct", "shared edit");
    lupanic_if_failed(VFS::unmount("/SharedB"));
    {
        auto file = must(instance->open_file("after-unmount", FileOpenFlag::write, FileCreationMode::create_new));
        lupanic_if_failed(file->write("still usable", 12));
    }
    lupanic_if_failed(instance->flush());
    expect_published_text(storage, "after-unmount", "still usable");
    storage->fail_publish = true;
    lupanic_if_failed(instance->create_dir("pending"));
    expect_error(instance->flush(), E_IO_ERROR);
    usize outputs = storage->output_count;
    {
        auto reader = must(instance->open_file("after-unmount", FileOpenFlag::read, FileCreationMode::open_existing));
        storage->fail_publish = false;
        expect_error(instance->flush(), E_BUSY);
    }
    lupanic_if_failed(instance->flush());
    luassert_always(storage->output_count == outputs);
    lupanic_if_failed(VFS::mount(instance, "/Remounted"));
    expect_vfs_text("/Remounted/after-unmount", "still usable");
    lupanic_if_failed(VFS::unmount("/Remounted"));
    auto published = storage->published->bytes;
    lupanic_if_failed(instance->create_dir("discarded"));
    instance = nullptr;
    luassert_always(equal_bytes(published, storage->published->bytes));
    luassert_always(!equal_bytes(original, published));
    printf("Direct Pak access, alias state and instance flush passed.\n");
}

#if !defined(LUNA_PLATFORM_WINDOWS)
static String block_native_publication(const c8* directory, const c8* mount_path, Ref<VFS::IFileSystem>& instance)
{
    auto path = child_path(directory, "blocked.pak");
    create_native_package(path.c_str());
    instance = must(VFS::new_pak_file_system(path.c_str(), Pak::OpenMode::read_write));
    lupanic_if_failed(VFS::mount(instance, mount_path));
    put_vfs(child_path(mount_path, "data/seed").c_str(), "retained output");
    // POSIX permits renaming an open input. This intentionally breaks exclusive
    // storage ownership solely to inject a deterministic replacement failure.
    auto backup = child_path(directory, "backup.pak");
    lupanic_if_failed(Luna::move_file(path.c_str(), backup.c_str()));
    lupanic_if_failed(Luna::create_dir(path.c_str()));
    expect_error(instance->flush(), E_IS_DIRECTORY);
    auto outputs = temporary_outputs(directory);
    luassert_always(outputs.size() == 1 && strstr(get_error().message.c_str(), outputs[0].c_str()));
    expect_vfs_text(child_path(mount_path, "data/seed").c_str(), "retained output");
    return outputs[0];
}

static void native_retry_test(const c8* directory)
{
    Ref<VFS::IFileSystem> instance;
    auto completed = block_native_publication(directory, "/Retry", instance);
    auto bytes = read_native(completed.c_str());
    expect_error(VFS::unmount("/Retry"), E_IS_DIRECTORY);
    luassert_always(equal_bytes(bytes, read_native(completed.c_str())));
    auto path = child_path(directory, "blocked.pak");
    auto backup = child_path(directory, "backup.pak");
    lupanic_if_failed(Luna::delete_file(path.c_str()));
    lupanic_if_failed(Luna::move_file(backup.c_str(), path.c_str()));
    lupanic_if_failed(instance->flush());
    luassert_always(temporary_outputs(directory).empty());
    luassert_always(equal_bytes(bytes, read_native(path.c_str())));
    lupanic_if_failed(VFS::unmount("/Retry"));
    instance = nullptr;
    lupanic_if_failed(Luna::delete_file(path.c_str()));
    printf("Native replacement failure and retry passed.\n");
}
#endif

static void shutdown_test(const c8* directory)
{
    auto path = child_path(directory, "shutdown.pak");
    create_native_package(path.c_str());
    auto instance = must(VFS::new_pak_file_system(path.c_str(), Pak::OpenMode::read_write));
    lupanic_if_failed(VFS::mount(instance, "/Shutdown"));
    lupanic_if_failed(VFS::mount(instance, "/ShutdownAlias"));
    auto file = must(VFS::open_file("/Shutdown/data/seed", FileOpenFlag::write, FileCreationMode::create_always));
    lupanic_if_failed(file->write("saved at shutdown", 17));
    auto iter = must(VFS::open_dir("/Shutdown"));
    FileSystemProbe failed_probe;
    failed_probe.fail_flush = true;
    auto failed_instance = new_object<TestFileSystem>();
    failed_instance->probe = &failed_probe;
    lupanic_if_failed(VFS::mount(failed_instance, "/CleanupFailure"));
    lupanic_if_failed(VFS::mount(failed_instance, "/CleanupFailureAlias"));
    auto direct_storage = make_storage();
    auto direct_instance = must(VFS::new_pak_file_system(direct_storage, Pak::OpenMode::read_write));
    lupanic_if_failed(VFS::mount(direct_instance, "/DirectAtShutdown"));
    auto direct_file = must(direct_instance->open_file("data/seed", FileOpenFlag::write, FileCreationMode::create_always));
    lupanic_if_failed(direct_file->write("direct handle retained", 22));
#if !defined(LUNA_PLATFORM_WINDOWS)
    Ref<VFS::IFileSystem> retained_instance;
    auto retained = block_native_publication(directory, "/ShutdownFailure", retained_instance);
#endif
    // Exercise the shutdown callback while Runtime/Pak are still alive so invalidated
    // wrappers and saved bytes can be inspected. The later module close is a no-op.
    module_vfs()->on_close();
    luassert_always(failed_probe.flush_count == 1 && !failed_probe.destroyed);
    failed_probe.fail_flush = false;
    lupanic_if_failed(failed_instance->flush());
    failed_instance = nullptr;
    luassert_always(failed_probe.destroyed == 1);
    luassert_always(!direct_storage->output_count && direct_file->get_size() == 22);
    direct_file = nullptr;
    lupanic_if_failed(direct_instance->flush());
    expect_published_text(direct_storage, "data/seed", "direct handle retained");
    expect_error(file->write("x", 1), E_BAD_CALLING_TIME);
    expect_error(file->tell(), E_BAD_CALLING_TIME);
    luassert_always(!iter->is_valid() && !iter->get_filename());
    expect_error(VFS::flush_all(), E_BAD_CALLING_TIME);
    file = nullptr;
    iter = nullptr;
    {
        auto input = must(Luna::open_file(path.c_str(), FileOpenFlag::read, FileCreationMode::open_existing));
        auto package = must(Pak::open_pak(input));
        auto saved = must(package->open_file("data/seed", FileOpenFlag::read, FileCreationMode::open_existing));
        expect_text(saved, "saved at shutdown");
    }
    instance = nullptr;
    lupanic_if_failed(Luna::delete_file(path.c_str()));
#if !defined(LUNA_PLATFORM_WINDOWS)
    {
        auto input = must(Luna::open_file(retained.c_str(), FileOpenFlag::read, FileCreationMode::open_existing));
        auto package = must(Pak::open_pak(input));
        auto saved = must(package->open_file("data/seed", FileOpenFlag::read, FileCreationMode::open_existing));
        expect_text(saved, "retained output");
    }
    retained_instance = nullptr;
    lupanic_if_failed(Luna::delete_file(retained.c_str()));
    lupanic_if_failed(Luna::delete_file(child_path(directory, "blocked.pak").c_str()));
    lupanic_if_failed(Luna::delete_file(child_path(directory, "backup.pak").c_str()));
#endif
    printf("Shutdown persistence, alias deduplication and caller ownership passed.\n");
}

int main()
{
    lupanic_if_failed(Luna::init());
    lupanic_if_failed(add_modules({module_vfs()}));
    lupanic_if_failed(init_modules());
    Meta::register_VFSPakTest_types();
    {
        c8 guid[GUID_STRING_LENGTH + 1]{};
        lupanic_if_failed(encode_guid(random_guid(), guid, GUID_STRING_LENGTH));
        String directory("VFSPakTest-");
        directory.append(guid);
        lupanic_if_failed(Luna::create_dir(directory.c_str()));
        native_mount_test(directory.c_str());
        storage_failure_test();
        routing_and_flush_all_test();
        cross_mount_test(directory.c_str());
        instance_lifetime_test();
        direct_pak_test();
#if !defined(LUNA_PLATFORM_WINDOWS)
        native_retry_test(directory.c_str());
#endif
        shutdown_test(directory.c_str());
        luassert_always(temporary_outputs(directory.c_str()).empty());
        lupanic_if_failed(Luna::delete_file(directory.c_str()));
    }
    Luna::close();
    printf("VFSPakTest passed.\n");
    return 0;
}
