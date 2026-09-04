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
#include "ZipTest.meta.generated.hpp"
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/File.hpp>
#include <Luna/Zip/Zip.hpp>
#include <cstdio>
using namespace Luna;
using namespace Luna::Zip;

static bool same_bytes(const Vector<byte_t>& lhs, const Vector<byte_t>& rhs)
{
    return lhs.size() == rhs.size() && (lhs.empty() || !memcmp(lhs.data(), rhs.data(), lhs.size()));
}

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
    luassert_always(unwrap_errcode(result.errcode()) == code);
}

static Ref<ZipTestStream> data_stream(const c8* text)
{
    auto stream = new_object<ZipTestStream>();
    stream->bytes.resize(strlen(text));
    memcpy(stream->bytes.data(), text, stream->bytes.size());
    return stream;
}

static Vector<byte_t> read_all(IStream* stream)
{
    Vector<byte_t> result;
    byte_t buffer[4096];
    while(true)
    {
        usize size = 0;
        lupanic_if_failed(stream->read(buffer, sizeof(buffer), &size));
        if(!size) break;
        result.insert(result.end(), buffer, buffer + size);
    }
    return result;
}

static Vector<byte_t> read_entry(IArchive* archive, u64 index)
{
    auto stream = must(archive->open_entry(index));
    return read_all(stream);
}

static Ref<ZipTestStream> create_test()
{
    auto archive = must(new_archive());
    auto output = data_stream("old output to be replaced");
    auto text = data_stream("Luna ZIP stream test. Luna ZIP stream test.");
    text->chunk_size = 3;
    text->position = 7;
    text->read_only = true;
    u64 stored = must(archive->add_file("stored.txt", text, CompressionMethod::store));
    u64 deflated = must(archive->add_file("nested/\xe6\x96\x87\xe4\xbb\xb6.txt", text, CompressionMethod::deflate, 9));
    u64 directory = must(archive->add_directory("empty"));
    auto empty = data_stream("");
    must(archive->add_file("zero", empty));
    luassert_always(text->bytes_read == 0 && output->bytes_written == 0);
    expect_error(archive->add_file("stored.txt", text), E_ALREADY_EXISTS);
    expect_error(archive->add_file("bad/", text), E_BAD_ARGUMENTS);
    expect_error(archive->add_file("bad", text, CompressionMethod::store, 1), E_BAD_ARGUMENTS);
    expect_error(archive->add_file("bad", text, (CompressionMethod)99), E_NOT_SUPPORTED);
    expect_error(archive->open_entry(directory), E_IS_DIRECTORY);
    expect_error(archive->get_entry(999), E_NOT_FOUND);
    luassert_always(same_bytes(read_entry(archive, stored), text->bytes));
    luassert_always(same_bytes(read_entry(archive, deflated), text->bytes));
    // Two independent entry cursors share the same underlying source object.
    {
        auto first = must(archive->open_entry(stored));
        auto second = must(archive->open_entry(deflated));
        c8 prefix[5];
        usize count;
        lupanic_if_failed(first->read(prefix, sizeof(prefix), &count));
        luassert_always(count == sizeof(prefix));
        luassert_always(same_bytes(read_all(second), text->bytes));
        auto tail = read_all(first);
        luassert_always(tail.size() == text->bytes.size() - sizeof(prefix));
        luassert_always(!memcmp(tail.data(), text->bytes.data() + sizeof(prefix), tail.size()));
        expect_error(first->write(prefix, sizeof(prefix)), E_ACCESS_DENIED);
        expect_error(archive->delete_entry(stored), E_BUSY);
        expect_error(archive->save(output), E_BUSY);
        expect_error(archive->discard(), E_BUSY);
    }
    expect_error(archive->save(text), E_BAD_ARGUMENTS);
    output->chunk_size = 7;
    lupanic_if_failed(archive->save(output));
    luassert_always(!archive->is_open());
    expect_error(archive->get_entries(), E_BAD_CALLING_TIME);
    lupanic_if_failed(archive->discard());
    auto reopened = must(open_archive(output));
    luassert_always(must(reopened->get_entries()).size() == 4);
    luassert_always(same_bytes(read_entry(reopened, must(reopened->find_entry("stored.txt"))), text->bytes));
    auto info = must(reopened->get_entry(must(reopened->find_entry("nested/\xe6\x96\x87\xe4\xbb\xb6.txt"))));
    luassert_always(info.compression == CompressionMethod::deflate && info.has_crc32 && info.has_compressed_size);
    luassert_always(info.compressed_size < info.size);
    expect_error(reopened->add_directory("no"), E_ACCESS_DENIED);
    expect_error(reopened->save(text), E_ACCESS_DENIED);
    auto retained = must(reopened->open_entry(stored));
    reopened = nullptr;
    luassert_always(same_bytes(read_all(retained), text->bytes));
    return output;
}

static void edit_test(ZipTestStream* original)
{
    auto unchanged_bytes = original->bytes;
    auto archive = must(open_archive(original, OpenMode::read_write));
    auto text = data_stream("replacement bytes");
    auto output = new_object<ZipTestStream>();
    auto stored = must(archive->find_entry("stored.txt"));
    auto compressed = must(archive->find_entry("nested/\xe6\x96\x87\xe4\xbb\xb6.txt"));
    lupanic_if_failed(archive->replace_file(stored, text));
    lupanic_if_failed(archive->replace_file(compressed, text));
    luassert_always(same_bytes(read_entry(archive, stored), text->bytes));
    lupanic_if_failed(archive->rename_entry(stored, "renamed.txt"));
    expect_error(archive->rename_entry(stored, "bad/"), E_BAD_ARGUMENTS);
    lupanic_if_failed(archive->delete_entry(must(archive->find_entry("empty/"))));
    expect_error(archive->find_entry("empty/"), E_NOT_FOUND);
    luassert_always(must(archive->get_entries()).size() == 3);
    luassert_always(same_bytes(original->bytes, unchanged_bytes));
    expect_error(archive->save(original), E_BAD_ARGUMENTS);
    // Fail before writing, during writing, and at commit, then retry the same session.
    output->fail_resize = true;
    expect_error(archive->save(output), E_IO_ERROR);
    luassert_always(archive->is_open());
    output->fail_resize = false;
    output->fail_write_at = 35;
    expect_error(archive->save(output), E_IO_ERROR);
    luassert_always(archive->is_open() && same_bytes(original->bytes, unchanged_bytes));
    output->fail_write_at = U64_MAX;
    output->fail_commit = true;
    expect_error(archive->save(output), E_IO_ERROR);
    luassert_always(archive->is_open() && same_bytes(original->bytes, unchanged_bytes));
    output->fail_commit = false;
    lupanic_if_failed(archive->save(output));
    auto reopened = must(open_archive(output, OpenMode::read_write));
    auto info = must(reopened->get_entry(must(reopened->find_entry("renamed.txt"))));
    luassert_always(info.compression == CompressionMethod::store);
    auto compressed_info = must(reopened->get_entry(must(reopened->find_entry("nested/\xe6\x96\x87\xe4\xbb\xb6.txt"))));
    luassert_always(compressed_info.compression == CompressionMethod::deflate);
    luassert_always(same_bytes(read_entry(reopened, info.index), text->bytes));
    lupanic_if_failed(reopened->set_compression(info.index, CompressionMethod::deflate, 1));
    auto recompressed = new_object<ZipTestStream>();
    lupanic_if_failed(reopened->save(recompressed));
    reopened = must(open_archive(recompressed));
    luassert_always(must(reopened->get_entry(must(reopened->find_entry("renamed.txt")))).compression == CompressionMethod::deflate);
    luassert_always(same_bytes(original->bytes, unchanged_bytes));
}

static void unchanged_test(ZipTestStream* original)
{
    auto archive = must(open_archive(original, OpenMode::read_write));
    auto index = must(archive->find_entry("stored.txt"));
    // Renames that cancel must still produce a complete archive on save.
    lupanic_if_failed(archive->rename_entry(index, "temporary.txt"));
    lupanic_if_failed(archive->rename_entry(index, "stored.txt"));
    auto output = new_object<ZipTestStream>();
    output->fail_write_at = 9;
    expect_error(archive->save(output), E_IO_ERROR);
    luassert_always(archive->is_open());
    output->fail_write_at = U64_MAX;
    lupanic_if_failed(archive->save(output));
    luassert_always(same_bytes(output->bytes, original->bytes));
    archive = must(open_archive(original, OpenMode::read_write));
    lupanic_if_failed(archive->set_compression(index, CompressionMethod::store));
    lupanic_if_failed(archive->save(output));
    archive = must(open_archive(output));
    luassert_always(must(archive->get_entries()).size() == 4);
    archive = must(open_archive(original, OpenMode::read_write));
    lupanic_if_failed(archive->rename_entry(index, "discarded"));
    lupanic_if_failed(archive->discard());
    archive = must(open_archive(original));
    must(archive->find_entry("stored.txt"));
}

static void empty_and_zip64_test()
{
    auto output = new_object<ZipTestStream>();
    auto archive = must(new_archive());
    lupanic_if_failed(archive->save(output));
    luassert_always(output->bytes.size() == 22);
    archive = must(open_archive(output, OpenMode::read_write));
    luassert_always(must(archive->get_entries()).empty());
    // Crossing the classic ZIP entry-count limit exercises ZIP64 directory records.
    for(u32 i = 0; i < 65536; ++i)
    {
        c8 name[32];
        snprintf(name, sizeof(name), "directory-%u/", i);
        must(archive->add_directory(name));
    }
    auto zip64 = new_object<ZipTestStream>();
    lupanic_if_failed(archive->save(zip64));
    archive = must(open_archive(zip64, OpenMode::read_write));
    luassert_always(must(archive->get_entries()).size() == 65536);
    luassert_always(must(archive->find_entry("directory-65535/")) == 65535);
    for(u64 i = 0; i < 65536; ++i) lupanic_if_failed(archive->delete_entry(i));
    lupanic_if_failed(archive->save(output));
    archive = must(open_archive(output));
    luassert_always(must(archive->get_entries()).empty() && output->bytes.size() == 22);
}

static void corruption_and_metadata_test()
{
    auto input = new_object<ZipTestStream>();
    input->bytes.resize(2 * 1024 * 1024, (byte_t)0x61);
    auto output = new_object<ZipTestStream>();
    auto archive = must(new_archive());
    must(archive->add_file("large", input, CompressionMethod::store));
    lupanic_if_failed(archive->save(output));
    output->bytes_read = 0;
    output->chunk_size = 13;
    archive = must(open_archive(output));
    must(archive->get_entries());
    // Central-directory lookup may overread a bounded tail to locate EOCD, but
    // it must not stream the 2 MiB payload or decompress it during enumeration.
    luassert_always(output->bytes_read < 128 * 1024);
    archive = nullptr;
    auto invalid = data_stream("not a ZIP");
    expect_error(open_archive(invalid), E_BAD_DATA);
    invalid->bytes = output->bytes;
    invalid->bytes.resize(50);
    expect_error(open_archive(invalid), E_BAD_DATA);
    // Stored entry data starts after its 30-byte local header and 5-byte name.
    output->bytes[35] = (byte_t)0x62;
    archive = must(open_archive(output));
    auto reader = must(archive->open_entry(0));
    byte_t buffer[65536];
    bool crc_failed = false;
    while(true)
    {
        usize count;
        auto result = reader->read(buffer, sizeof(buffer), &count);
        if(failed(result))
        {
            luassert_always(unwrap_errcode(result.errcode()) == E_BAD_DATA);
            crc_failed = true;
            break;
        }
        if(!count) break;
    }
    luassert_always(crc_failed);
}

static void imported_archive_test(const c8* path)
{
    auto file = must(open_file(path, FileOpenFlag::read, FileCreationMode::open_existing));
    auto archive = must(open_archive(file, OpenMode::read_write));
    auto expected = data_stream("Created by Python zipfile.");
    for(auto name : {"stored.txt", "deflated.txt", "zip64.txt"})
    {
        luassert_always(same_bytes(read_entry(archive, must(archive->find_entry(name))), expected->bytes));
    }
    auto unsupported = must(archive->find_entry("unsupported.txt"));
    expect_error(archive->open_entry(unsupported), E_NOT_SUPPORTED);
    auto stored = must(archive->find_entry("stored.txt"));
    lupanic_if_failed(archive->rename_entry(stored, "renamed.txt"));
    auto output = new_object<ZipTestStream>();
    lupanic_if_failed(archive->save(output));
    archive = must(open_archive(output));
    luassert_always(same_bytes(read_entry(archive, must(archive->find_entry("renamed.txt"))), expected->bytes));
    expect_error(archive->open_entry(must(archive->find_entry("unsupported.txt"))), E_NOT_SUPPORTED);
}

static void unsupported_entry_test(ZipTestStream* original)
{
    auto input = new_object<ZipTestStream>();
    input->bytes = original->bytes;
    // Flag the first entry as encrypted in both headers. The wrapper must reject
    // decoding before treating these unencrypted bytes as encrypted input.
    input->bytes[6] |= 1;
    for(usize i = 0; i + 46 < input->bytes.size(); ++i)
    {
        if(!memcmp(input->bytes.data() + i, "PK\x01\x02", 4))
        {
            input->bytes[i + 8] |= 1;
            break;
        }
    }
    auto archive = must(open_archive(input, OpenMode::read_write));
    luassert_always(must(archive->get_entry(0)).encrypted);
    expect_error(archive->open_entry(0), E_NOT_SUPPORTED);
    expect_error(archive->replace_file(0, original), E_NOT_SUPPORTED);
    expect_error(archive->set_compression(0, CompressionMethod::deflate), E_NOT_SUPPORTED);
}

static void run_tests(const c8* output_path, const c8* import_path)
{
    auto ordinary = create_test();
    edit_test(ordinary);
    unchanged_test(ordinary);
    corruption_and_metadata_test();
    unsupported_entry_test(ordinary);
    empty_and_zip64_test();
    if(import_path) imported_archive_test(import_path);
    if(output_path)
    {
        auto file = must(open_file(output_path, FileOpenFlag::write, FileCreationMode::create_always));
        auto archive = must(open_archive(ordinary, OpenMode::read_write));
        lupanic_if_failed(archive->save(file));
        file->flush();
    }
}

int main(int argc, char** argv)
{
    lupanic_if_failed(Luna::init());
    lupanic_if_failed(add_modules({module_zip()}));
    lupanic_if_failed(init_modules());
    Meta::register_ZipTest_types();
    run_tests(argc > 1 ? argv[1] : nullptr, argc > 2 ? argv[2] : nullptr);
    Luna::close();
    printf("ZipTest: all tests passed.\n");
    return 0;
}
