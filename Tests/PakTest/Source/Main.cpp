/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Main.cpp
* @author JXMaster
* @date 2026/9/4
*/
#include "TestStream.hpp"
#include "PakTest.meta.generated.hpp"
#include <Luna/Pak/Pak.hpp>
#include <Luna/Zip/Zip.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
#include <cstdio>
#include <thread>

using namespace Luna;
using namespace Luna::Pak;

template <typename T>
static T must(R<T> result)
{
    lupanic_if_failed(result);
    return move(result.get());
}

template <typename T>
static void expect_error(R<T> result, ResultCode code)
{
    luassert_always(failed(result));
    if(unwrap_errcode(result.errcode()) != code)
    {
        printf("Unexpected error: %s\n", explain(result.errcode()));
        luassert_always(false);
    }
}

static Ref<PakTestStream> data_stream(const c8* text = "")
{
    auto stream = new_object<PakTestStream>();
    stream->bytes.resize(strlen(text));
    if(!stream->bytes.empty()) memcpy(stream->bytes.data(), text, stream->bytes.size());
    return stream;
}

static bool equal_bytes(const Vector<byte_t>& left, const Vector<byte_t>& right)
{
    return left.size() == right.size() && (left.empty() || !memcmp(left.data(), right.data(), left.size()));
}

static Vector<byte_t> read_all(IStream* file)
{
    Vector<byte_t> bytes;
    byte_t buffer[4096];
    while(true)
    {
        usize count = 0;
        lupanic_if_failed(file->read(buffer, sizeof(buffer), &count));
        if(!count) break;
        bytes.insert(bytes.end(), buffer, buffer + count);
    }
    return bytes;
}

static Vector<byte_t> read_file(IPak* package, const c8* path)
{
    auto file = must(package->open_file(path, FileOpenFlag::read, FileCreationMode::open_existing));
    return read_all(file);
}

static void expect_text(IPak* package, const c8* path, const c8* text)
{
    auto bytes = read_file(package, path);
    luassert_always(bytes.size() == strlen(text));
    luassert_always(bytes.empty() || !memcmp(bytes.data(), text, bytes.size()));
}

static void put_file(IPak* package, const c8* path, const c8* text)
{
    auto file = must(package->open_file(path, FileOpenFlag::write, FileCreationMode::create_always));
    lupanic_if_failed(file->write(text, strlen(text)));
}

static Ref<PakTestStream> create_test()
{
    auto package = must(new_pak());
    luassert_always(package->is_open() && package->is_dirty() && !package->is_read_only());
    luassert_always(test_flags(must(package->get_file_attribute("/")).attributes, FileAttributeFlag::directory));
    expect_error(package->create_dir("/"), E_ALREADY_EXISTS);
    expect_error(package->delete_file("/"), E_BAD_ARGUMENTS);
    expect_error(package->get_file_attribute(nullptr), E_BAD_ARGUMENTS);
    const c8* invalid_paths[] = {"//", "../escape", "a/../b", "a/./b", "./a", "a//b", "a//", "a\\b", "C:/a", "\xc0\xaf"};
    for(const auto path : invalid_paths) expect_error(package->create_dir(path), E_BAD_ARGUMENTS);
    expect_error(package->create_dir("missing/child"), E_NOT_FOUND);
    expect_error(package->open_file("missing/child", FileOpenFlag::write, FileCreationMode::create_new), E_NOT_FOUND);
    expect_error(package->open_file("none", FileOpenFlag::read, FileCreationMode::open_always), E_ACCESS_DENIED);
    expect_error(package->open_file("none", FileOpenFlag::none, FileCreationMode::open_existing), E_BAD_ARGUMENTS);
    expect_error(package->open_file("none", (FileOpenFlag)128, FileCreationMode::open_existing), E_BAD_ARGUMENTS);
    expect_error(package->open_file("none", FileOpenFlag::write, (FileCreationMode)99), E_BAD_ARGUMENTS);
    lupanic_if_failed(package->create_dir("/data/"));
    lupanic_if_failed(package->create_dir("empty"));
    auto snapshot = must(package->open_dir(""));
    luassert_always(!strcmp(snapshot->get_filename(), "data"));
    expect_error(package->open_file("data", FileOpenFlag::read, FileCreationMode::open_existing), E_IS_DIRECTORY);
    {
        auto file = must(package->open_file("data/\xe6\x96\x87\xe4\xbb\xb6.bin", FileOpenFlag::read | FileOpenFlag::write,
            FileCreationMode::create_new));
        lupanic_if_failed(file->write("abcdefgh", 8));
        lupanic_if_failed(file->seek(2, SeekMode::begin));
        lupanic_if_failed(file->write("XY", 2));
        lupanic_if_failed(file->seek(10, SeekMode::begin));
        lupanic_if_failed(file->write("Z", 1));
        lupanic_if_failed(file->set_size(14));
        lupanic_if_failed(file->seek(0, SeekMode::begin));
        auto bytes = read_all(file);
        const c8 expected[] = {'a', 'b', 'X', 'Y', 'e', 'f', 'g', 'h', 0, 0, 'Z', 0, 0, 0};
        luassert_always(bytes.size() == sizeof(expected) && !memcmp(bytes.data(), expected, sizeof(expected)));
        lupanic_if_failed(file->set_size(5));
        luassert_always(must(file->tell()) == 14 && file->get_size() == 5);
        expect_error(file->seek(I64_MIN, SeekMode::current), E_OUT_OF_RANGE);
        expect_error(file->seek(0, (SeekMode)99), E_BAD_ARGUMENTS);
        lupanic_if_failed(file->seek(I64_MAX, SeekMode::begin));
        expect_error(file->write("x", 1), E_OUT_OF_RANGE);
        expect_error(file->set_size(U64_MAX), E_OUT_OF_RANGE);
        expect_error(package->open_file("data/\xe6\x96\x87\xe4\xbb\xb6.bin", FileOpenFlag::read, FileCreationMode::open_existing), E_BUSY);
        expect_error(package->copy_file("data/\xe6\x96\x87\xe4\xbb\xb6.bin", "copy"), E_BUSY);
        expect_error(package->move_file("data", "moved"), E_BUSY);
        expect_error(package->flush(data_stream()), E_BUSY);
        expect_error(package->discard(), E_BUSY);
        put_file(package, "other", "other file");
        file->flush();
    }
    expect_text(package, "data/\xe6\x96\x87\xe4\xbb\xb6.bin", "abXYe");
    expect_error(package->create_dir("other/child"), E_NOT_DIRECTORY);
    expect_error(package->get_file_attribute("other/child"), E_NOT_DIRECTORY);
    expect_error(package->get_file_attribute("other/"), E_NOT_DIRECTORY);
    expect_error(package->open_dir("other"), E_NOT_DIRECTORY);
    expect_error(package->delete_file("data"), E_DIRECTORY_NOT_EMPTY);
    expect_error(package->move_file("data", "data/sub"), E_BAD_ARGUMENTS);
    expect_error(package->move_file("data", "empty"), E_ALREADY_EXISTS);
    lupanic_if_failed(package->copy_file("other", "copy"));
    put_file(package, "other", "changed");
    expect_text(package, "copy", "other file");
    expect_error(package->open_file("copy", FileOpenFlag::read, FileCreationMode::create_always), E_ACCESS_DENIED);
    expect_error(package->open_file("copy", FileOpenFlag::write, FileCreationMode::create_new), E_ALREADY_EXISTS);
    expect_error(package->open_file("absent", FileOpenFlag::write, FileCreationMode::open_existing_as_new), E_NOT_FOUND);
    {
        auto first = must(package->open_file("copy", FileOpenFlag::read, FileCreationMode::open_existing));
        auto second = must(package->open_file("copy", FileOpenFlag::read, FileCreationMode::open_existing));
        expect_error(package->open_file("copy", FileOpenFlag::write, FileCreationMode::open_existing), E_BUSY);
        expect_error(package->delete_file("copy"), E_BUSY);
        expect_error(package->set_file_compression("copy", CompressionMethod::deflate), E_BUSY);
        expect_error(first->write("x", 1), E_ACCESS_DENIED);
        expect_error(first->set_size(0), E_ACCESS_DENIED);
        lupanic_if_failed(first->seek(6, SeekMode::begin));
        luassert_always(read_all(first).size() == 4);
        luassert_always(read_all(second).size() == 10);
    }
    lupanic_if_failed(package->set_file_compression("copy", CompressionMethod::deflate, 9));
    expect_error(package->set_file_compression("other", CompressionMethod::store, 1), E_BAD_ARGUMENTS);
    expect_error(package->set_file_compression("other", (CompressionMethod)99), E_NOT_SUPPORTED);
    // Iteration remains a snapshot after unrelated mutations.
    luassert_always(snapshot->move_next() && !strcmp(snapshot->get_filename(), "empty"));
    luassert_always(!snapshot->move_next());
    luassert_always(!snapshot->get_filename() && snapshot->get_attributes() == FileAttributeFlag::none);
    auto output = data_stream("obsolete output contents");
    output->chunk_size = 7;
    lupanic_if_failed(package->flush(output));
    luassert_always(package->is_open() && !package->is_dirty());
    luassert_always(must(package->get_file_compression("copy")) == CompressionMethod::deflate);
    expect_error(package->flush(output), E_BAD_ARGUMENTS);
    auto archive = must(Zip::open_archive(output));
    auto stored = must(archive->get_entry(must(archive->find_entry("other"))));
    auto compressed = must(archive->get_entry(must(archive->find_entry("copy"))));
    luassert_always(stored.compression == Zip::CompressionMethod::store);
    luassert_always(compressed.compression == Zip::CompressionMethod::deflate && compressed.has_crc32);
    return output;
}

static Ref<PakTestStream> edit_test(PakTestStream* source)
{
    auto original = source->bytes;
    source->bytes_written = 0;
    source->read_only = true;
    auto package = must(open_pak(source, OpenMode::read_write));
    luassert_always(!package->is_dirty());
    {
        auto file = must(package->open_file("copy", FileOpenFlag::write, FileCreationMode::open_existing));
        luassert_always(!package->is_dirty());
        lupanic_if_failed(file->seek(6, SeekMode::begin));
        lupanic_if_failed(file->write("PAK!", 4));
        expect_error(file->read(nullptr, 0), E_ACCESS_DENIED);
    }
    expect_text(package, "copy", "other PAK!");
    lupanic_if_failed(package->move_file("data", "assets"));
    lupanic_if_failed(package->move_file("other", "temporary"));
    lupanic_if_failed(package->move_file("copy", "other"));
    lupanic_if_failed(package->move_file("temporary", "copy"));
    lupanic_if_failed(package->delete_file("empty"));
    lupanic_if_failed(package->create_dir("empty-again"));
    luassert_always(source->bytes_written == 0 && equal_bytes(source->bytes, original));
    // All failure stages preserve the source and the pending edits for retry.
    for(u32 phase = 0; phase < 4; ++phase)
    {
        auto output = data_stream();
        output->fail_resize = phase == 0;
        output->fail_write_at = phase == 1 ? 40 : U64_MAX;
        output->fail_commit = phase == 2;
        output->fail_read_after_write = phase == 3;
        auto result = package->flush(output);
        luassert_always(failed(result));
        luassert_always(package->is_open() && package->is_dirty());
        expect_text(package, "other", "other PAK!");
        expect_text(package, "assets/\xe6\x96\x87\xe4\xbb\xb6.bin", "abXYe");
        luassert_always(equal_bytes(source->bytes, original));
    }
    auto output = data_stream();
    lupanic_if_failed(package->flush(output));
    luassert_always(!package->is_dirty());
    expect_text(package, "copy", "changed");
    luassert_always(must(package->get_file_compression("other")) == CompressionMethod::deflate);
    expect_error(package->get_file_attribute("data"), E_NOT_FOUND);
    // Edit/flush again on the same Pak object. Empty files and directories survive.
    {
        auto file = must(package->open_file("other", FileOpenFlag::write, FileCreationMode::open_existing_as_new));
        luassert_always(file->get_size() == 0);
    }
    auto second = data_stream();
    lupanic_if_failed(package->flush(second));
    luassert_always(read_file(package, "other").empty());
    auto third = data_stream();
    lupanic_if_failed(package->flush(third));
    luassert_always(equal_bytes(second->bytes, third->bytes));
    lupanic_if_failed(package->discard());
    luassert_always(!package->is_open() && !package->is_dirty());
    expect_error(package->get_file_attribute("/"), E_BAD_CALLING_TIME);
    expect_error(package->flush(data_stream()), E_BAD_CALLING_TIME);
    lupanic_if_failed(package->discard());
    return output;
}

static void reading_test()
{
    auto archive = must(Zip::new_archive());
    auto data = data_stream();
    data->bytes.resize(2 * 1024 * 1024);
    for(usize i = 0; i < data->bytes.size(); ++i) data->bytes[i] = (byte_t)(i % 251);
    must(archive->add_file("implicit/nested/data", data, Zip::CompressionMethod::store));
    auto text = data_stream("0123456789abcdefghijklmnopqrstuvwxyz");
    must(archive->add_file("compressed", text, Zip::CompressionMethod::deflate));
    auto source = data_stream();
    lupanic_if_failed(archive->save(source));
    source->bytes_read = 0;
    source->bytes_written = 0;
    source->read_only = true;
    auto package = must(open_pak(source));
    luassert_always(package->is_read_only());
    auto children = must(package->open_dir("implicit/nested"));
    luassert_always(children->is_valid() && !strcmp(children->get_filename(), "data"));
    luassert_always(source->bytes_read < 128 * 1024 && source->bytes_written == 0);
    expect_error(package->create_dir("no"), E_ACCESS_DENIED);
    expect_error(package->flush(data_stream()), E_ACCESS_DENIED);
    expect_error(package->open_file("compressed", FileOpenFlag::write, FileCreationMode::open_existing), E_ACCESS_DENIED);
    auto flags = must(package->get_file_attribute("compressed")).attributes;
    luassert_always(test_flags(flags, FileAttributeFlag::read_only));
    {
        auto file = must(package->open_file("compressed", FileOpenFlag::read, FileCreationMode::open_existing));
        lupanic_if_failed(file->seek(10, SeekMode::begin));
        c8 bytes[5];
        usize count = 0;
        lupanic_if_failed(file->read(bytes, 5, &count));
        luassert_always(count == 5 && !memcmp(bytes, "abcde", 5));
        lupanic_if_failed(file->seek(-13, SeekMode::current));
        lupanic_if_failed(file->read(bytes, 5, &count));
        luassert_always(count == 5 && !memcmp(bytes, "23456", 5));
        lupanic_if_failed(file->seek(-3, SeekMode::end));
        luassert_always(read_all(file).size() == 3);
        lupanic_if_failed(file->seek(7, SeekMode::end));
        luassert_always(read_all(file).empty());
    }
    // Two real threads exercise independent cursors sharing one retained source.
    auto worker = [package]()
    {
        auto file = must(package->open_file("implicit/nested/data", FileOpenFlag::read, FileCreationMode::open_existing));
        for(u32 i = 0; i < 12; ++i)
        {
            u32 position = (i * 973) % 8000;
            lupanic_if_failed(file->seek(position, SeekMode::begin));
            byte_t bytes[127];
            usize count = 0;
            lupanic_if_failed(file->read(bytes, sizeof(bytes), &count));
            luassert_always(count == sizeof(bytes));
            for(usize j = 0; j < count; ++j) luassert_always(bytes[j] == (byte_t)((position + j) % 251));
        }
    };
    std::thread first(worker);
    std::thread second(worker);
    first.join();
    second.join();
    auto retained = must(package->open_file("compressed", FileOpenFlag::read, FileCreationMode::open_existing));
    package = nullptr;
    luassert_always(equal_bytes(read_all(retained), text->bytes));
}

static Ref<PakTestStream> g_staging;
static bool g_fail_factory = false;
static Ref<ISeekableStream> g_alias;
static R<Ref<ISeekableStream>> create_staging(object_t userdata)
{
    luassert_always(userdata);
    if(g_fail_factory) return E_IO_ERROR;
    if(g_alias) return g_alias;
    g_staging = data_stream("factory data must be reset");
    g_staging->chunk_size = 3;
    return Ref<ISeekableStream>(g_staging);
}

static void staging_test(PakTestStream* source)
{
    Options options;
    options.max_memory_file_size = 3;
    auto limited = must(open_pak(source, OpenMode::read_write, options));
    expect_text(limited, "other", "other PAK!");
    expect_error(limited->open_file("other", FileOpenFlag::write, FileCreationMode::open_existing), E_OUT_OF_RANGE);
    luassert_always(!limited->is_dirty());
    {
        auto file = must(limited->open_file("other", FileOpenFlag::read | FileOpenFlag::write, FileCreationMode::create_always));
        lupanic_if_failed(file->write("abc", 3));
        expect_error(file->write("x", 1), E_OUT_OF_RANGE);
        expect_error(file->set_size(4), E_OUT_OF_RANGE);
        luassert_always(file->get_size() == 3 && must(file->tell()) == 3);
    }
    options.create_staging_stream = create_staging;
    options.staging_userdata = data_stream();
    auto package = must(open_pak(source, OpenMode::read_write, options));
    g_fail_factory = true;
    expect_error(package->open_file("other", FileOpenFlag::write, FileCreationMode::open_existing), E_IO_ERROR);
    luassert_always(!package->is_dirty());
    g_fail_factory = false;
    g_alias = source;
    expect_error(package->open_file("other", FileOpenFlag::write, FileCreationMode::open_existing), E_BAD_ARGUMENTS);
    g_alias = nullptr;
    expect_text(package, "other", "other PAK!");
    {
        auto file = must(package->open_file("other", FileOpenFlag::read | FileOpenFlag::write, FileCreationMode::open_existing));
        luassert_always(file->get_size() == 10 && !package->is_dirty());
        lupanic_if_failed(file->seek(14, SeekMode::begin));
        lupanic_if_failed(file->write("custom", 6));
        lupanic_if_failed(file->set_size(23));
        lupanic_if_failed(file->seek(10, SeekMode::begin));
        auto bytes = read_all(file);
        luassert_always(bytes.size() == 13);
        for(usize i = 0; i < 4; ++i) luassert_always(bytes[i] == 0);
        luassert_always(!memcmp(bytes.data() + 4, "custom", 6));
        for(usize i = 10; i < 13; ++i) luassert_always(bytes[i] == 0);
        g_staging->fail_write_at = 24;
        lupanic_if_failed(file->seek(23, SeekMode::begin));
        usize count = 0;
        expect_error(file->write("failure", 7, &count), E_IO_ERROR);
        luassert_always(count == 1 && file->get_size() == 24 && must(file->tell()) == 24);
        g_staging->fail_write_at = U64_MAX;
    }
    expect_error(package->flush(g_staging), E_BAD_ARGUMENTS);
    g_alias = g_staging;
    expect_error(package->copy_file("other", "alias-copy"), E_BAD_ARGUMENTS);
    g_alias = nullptr;
    auto saved = data_stream();
    lupanic_if_failed(package->flush(saved));
    luassert_always(read_file(package, "other").size() == 24);
    g_staging = nullptr;
}

static Ref<PakTestStream> make_zip(const c8* first, const c8* second = nullptr)
{
    auto archive = must(Zip::new_archive());
    auto data = data_stream("fixture bytes");
    must(archive->add_file(first, data, Zip::CompressionMethod::store));
    if(second) must(archive->add_file(second, data, Zip::CompressionMethod::store));
    auto result = data_stream();
    lupanic_if_failed(archive->save(result));
    return result;
}

static void rewrite_name(PakTestStream* source, const c8* from, const c8* to)
{
    usize size = strlen(from);
    luassert_always(size == strlen(to));
    usize replacements = 0;
    for(usize i = 0; i + size <= source->bytes.size(); ++i)
    {
        if(!memcmp(source->bytes.data() + i, from, size))
        {
            memcpy(source->bytes.data() + i, to, size);
            ++replacements;
        }
    }
    luassert_always(replacements == 2);
}

static void validation_test()
{
    expect_error(open_pak(nullptr), E_BAD_ARGUMENTS);
    Options invalid;
    invalid.compression = (CompressionMethod)99;
    expect_error(new_pak(invalid), E_NOT_SUPPORTED);
    invalid.compression = CompressionMethod::store;
    invalid.compression_level = 1;
    expect_error(new_pak(invalid), E_BAD_ARGUMENTS);
    auto valid = make_zip("data");
    expect_error(open_pak(valid, (OpenMode)99), E_BAD_ARGUMENTS);
    luassert_always(failed(open_pak(data_stream("not a zip"))));
    const c8* paths[] = {"../escape", "/absolute", "a//b", "a/./b", "a\\b", "C:/drive"};
    for(const auto path : paths) expect_error(open_pak(make_zip(path)), E_BAD_DATA);
    expect_error(open_pak(make_zip("file", "file/child")), E_BAD_DATA);
    expect_error(open_pak(make_zip("file/child", "file")), E_BAD_DATA);
    auto duplicate = make_zip("dupA", "dupB");
    rewrite_name(duplicate, "dupA", "dupB");
    expect_error(open_pak(duplicate), E_BAD_DATA);
    auto directory_data = make_zip("dirX");
    rewrite_name(directory_data, "dirX", "dir/");
    expect_error(open_pak(directory_data), E_BAD_DATA);
    // Mutate structural flags only, so unsupported formats can be rejected without decoding.
    for(bool encrypted : {false, true})
    {
        auto unsupported = make_zip("data");
        for(usize i = 0; i + 46 <= unsupported->bytes.size(); ++i)
        {
            auto p = unsupported->bytes.data() + i;
            if(!memcmp(p, "PK\x03\x04", 4))
            {
                if(encrypted) p[6] |= 1;
                else p[8] = 12;
            }
            if(!memcmp(p, "PK\x01\x02", 4))
            {
                if(encrypted) p[8] |= 1;
                else p[10] = 12;
            }
        }
        expect_error(open_pak(unsupported), E_NOT_SUPPORTED);
    }
    auto corrupt = make_zip("crc");
    const c8 payload[] = "fixture bytes";
    bool corrupted = false;
    for(usize i = 0; i + sizeof(payload) - 1 <= corrupt->bytes.size(); ++i)
    {
        if(!memcmp(corrupt->bytes.data() + i, payload, sizeof(payload) - 1))
        {
            corrupt->bytes[i] ^= 1;
            corrupted = true;
            break;
        }
    }
    luassert_always(corrupted);
    auto package = must(open_pak(corrupt, OpenMode::read_write));
    {
        auto file = must(package->open_file("crc", FileOpenFlag::read, FileCreationMode::open_existing));
        byte_t bytes[128];
        expect_error(file->read(bytes, sizeof(bytes)), E_BAD_DATA);
    }
    expect_error(package->open_file("crc", FileOpenFlag::write, FileCreationMode::open_existing), E_BAD_DATA);
    luassert_always(!package->is_dirty());
}

static void empty_and_compression_test()
{
    Options options;
    options.compression = CompressionMethod::deflate;
    options.compression_level = 6;
    auto package = must(new_pak(options));
    auto empty = data_stream();
    lupanic_if_failed(package->flush(empty));
    luassert_always(empty->bytes.size() == 22);
    put_file(package, "compressed", "repeated repeated repeated repeated repeated");
    put_file(package, "zero", "");
    luassert_always(must(package->get_file_compression("compressed")) == CompressionMethod::deflate);
    auto saved = data_stream();
    lupanic_if_failed(package->flush(saved));
    auto zip = must(Zip::open_archive(saved));
    auto info = must(zip->get_entry(must(zip->find_entry("compressed"))));
    luassert_always(info.compression == Zip::CompressionMethod::deflate && info.compressed_size < info.size);
    luassert_always(read_file(package, "zero").empty());
    lupanic_if_failed(package->delete_file("compressed"));
    lupanic_if_failed(package->delete_file("zero"));
    auto cleared = data_stream();
    lupanic_if_failed(package->flush(cleared));
    luassert_always(cleared->bytes.size() == 22);
}

static void import_test(const c8* path)
{
    auto input = must(Luna::open_file(path, FileOpenFlag::read, FileCreationMode::open_existing));
    auto package = must(open_pak(input, OpenMode::read_write));
    expect_text(package, "python/stored.txt", "stored by Python");
    expect_text(package, "python/\xe6\x96\x87\xe4\xbb\xb6.txt", "compressed by Python");
    expect_text(package, "small-zip64", "ZIP64 local entry");
    lupanic_if_failed(package->move_file("python", "imported"));
    put_file(package, "imported/added.txt", "written by Pak");
    String output_path(path);
    output_path.append(".edited.pak");
    auto output = must(Luna::open_file(output_path.c_str(), FileOpenFlag::read | FileOpenFlag::write | FileOpenFlag::user_buffering,
        FileCreationMode::create_always));
    lupanic_if_failed(package->flush(output));
    expect_text(package, "imported/added.txt", "written by Pak");
    expect_text(package, "imported/\xe6\x96\x87\xe4\xbb\xb6.txt", "compressed by Python");
}

int main(int argc, char** argv)
{
    lupanic_if_failed(Luna::init());
    lupanic_if_failed(add_modules({module_pak()}));
    lupanic_if_failed(init_modules());
    Meta::register_PakTest_types();
    {
        auto first = create_test();
        auto second = edit_test(first);
        reading_test();
        staging_test(second);
        validation_test();
        empty_and_compression_test();
        if(argc > 2) import_test(argv[2]);
        if(argc > 1)
        {
            auto output = must(Luna::open_file(argv[1], FileOpenFlag::write, FileCreationMode::create_always));
            lupanic_if_failed(output->write(second->bytes.data(), second->bytes.size()));
        }
    }
    Luna::close();
    printf("PakTest passed.\n");
    return 0;
}
