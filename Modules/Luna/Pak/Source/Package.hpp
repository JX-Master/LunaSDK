/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Package.hpp
* @author JXMaster
* @date 2026/9/4
*/
#pragma once
#include "../Pak.hpp"
#include <Luna/Zip/Zip.hpp>
#include <Luna/Runtime/HashMap.hpp>
#include <Luna/Runtime/Hash.hpp>
#include <Luna/Runtime/UniquePtr.hpp>
#include <Luna/Runtime/Mutex.hpp>
#include "Package.generated.hpp"

namespace Luna::Pak
{
    struct PathHash
    {
        usize operator()(const String& path) const { return memhash(path.data(), path.size()); }
    };
    struct PathEqual
    {
        bool operator()(const String& lhs, const String& rhs) const { return lhs.compare(rhs) == 0; }
    };

    struct Node
    {
        bool directory = false;
        u64 index = U64_MAX;
        u64 size = 0;
        String original_name;
        Ref<ISeekableStream> data;
        CompressionMethod compression = CompressionMethod::store;
        u32 compression_level = 0;
        bool data_changed = false;
        bool compression_changed = false;
        usize readers = 0;
        bool writer = false;
    };

    struct [[Luna::struct("{5A02E1F3-58E0-445B-9CED-4BB9CC650B72}")]] Package : IPak
    {
        luiimpl();
        Ref<IMutex> m_mutex;
        Options m_options;
        Ref<ISeekableStream> m_source;
        Ref<Zip::IArchive> m_archive;
        HashMap<String, UniquePtr<Node>, PathHash, PathEqual> m_nodes;
        usize m_handles = 0;
        bool m_open = true;
        bool m_writable = false;
        bool m_dirty = false;
        RV init(ISeekableStream* source, OpenMode mode, const Options& options);
        RV load_index();
        RV check_open(bool writing = false);
        R<Node*> find_node(const String& path, bool require_directory = false);
        RV check_parent(const String& path);
        R<Ref<ISeekableStream>> make_staging(u64 size);
        R<Ref<ISeekableStream>> copy_contents(Node* node, bool truncate);
        bool is_open() override;
        bool is_read_only() override;
        bool is_dirty() override;
        R<Ref<IFile>> open_file(const c8* path, FileOpenFlag flags, FileCreationMode creation) override;
        R<FileAttribute> get_file_attribute(const c8* path) override;
        R<Ref<IFileIterator>> open_dir(const c8* path) override;
        RV create_dir(const c8* path) override;
        RV copy_file(const c8* from_path, const c8* to_path) override;
        RV move_file(const c8* from_path, const c8* to_path, FileMoveFlag flags) override;
        RV delete_file(const c8* path) override;
        R<CompressionMethod> get_file_compression(const c8* path) override;
        RV set_file_compression(const c8* path, CompressionMethod method, u32 level) override;
        RV flush(ISeekableStream* destination) override;
        RV discard() override;
    };

    struct [[Luna::struct("{C3A4B920-6786-4AF6-B3C0-2C729E4BF61B}")]] PakFile : IFile
    {
        luiimpl();
        Ref<Package> m_owner;
        Node* m_node = nullptr;
        FileOpenFlag m_flags = FileOpenFlag::none;
        u64 m_position = 0;
        u64 m_decode_position = 0;
        Ref<IStream> m_reader;
        ~PakFile();
        RV prepare_reader();
        RV resize(u64 size);
        RV read(void* buffer, usize size, usize* read_bytes) override;
        RV write(const void* buffer, usize size, usize* write_bytes) override;
        R<u64> tell() override;
        RV seek(i64 offset, SeekMode mode) override;
        u64 get_size() override;
        RV set_size(u64 size) override;
        void flush() override;
    };

    struct DirectoryEntry
    {
        String name;
        FileAttributeFlag attributes;
    };

    struct [[Luna::struct("{200E81BA-7DE4-421D-B06A-50C1B1671A3F}")]] DirectoryIterator : IFileIterator
    {
        luiimpl();
        Vector<DirectoryEntry> m_entries;
        usize m_index = 0;
        bool is_valid() override { return m_index < m_entries.size(); }
        const c8* get_filename() override { return is_valid() ? m_entries[m_index].name.c_str() : nullptr; }
        FileAttributeFlag get_attributes() override { return is_valid() ? m_entries[m_index].attributes : FileAttributeFlag::none; }
        bool move_next() override { if(is_valid()) ++m_index; return is_valid(); }
    };

    struct [[Luna::struct("{7AD00597-6E50-43AC-AEA0-699BF3BCC095}")]] MemoryStream : ISeekableStream
    {
        luiimpl();
        Vector<byte_t> m_bytes;
        u64 m_position = 0;
        u64 m_limit = 0;
        RV read(void* buffer, usize size, usize* read_bytes) override;
        RV write(const void* buffer, usize size, usize* write_bytes) override;
        R<u64> tell() override { return m_position; }
        RV seek(i64 offset, SeekMode mode) override;
        u64 get_size() override { return m_bytes.size(); }
        RV set_size(u64 size) override;
    };

    R<u64> seek_position(u64 position, u64 size, i64 offset, SeekMode mode);
    RV write_all(IStream* stream, const void* buffer, usize size, usize* written = nullptr);
}
