/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Archive.hpp
* @author JXMaster
* @date 2026/9/4
*/
#pragma once
#include "../Zip.hpp"
#include <Luna/Runtime/HashMap.hpp>
#include <zip.h>
#include "Archive.generated.hpp"

namespace Luna::Zip
{
    struct StreamSource;
    struct [[Luna::struct("{581638DC-22BC-4C8B-8F4D-6389C298E48C}")]] Archive : IArchive
    {
        luiimpl();
        zip_t* m_archive = nullptr;
        StreamSource* m_source = nullptr;
        Vector<Ref<ISeekableStream>> m_entry_sources;
        HashMap<u64, Ref<ISeekableStream>> m_staged_entries;
        usize m_readers = 0;
        bool m_writable = false;
        // Data/compression edits and tombstones cannot be undone through this API.
        // Renames can cancel each other, so save compares names separately.
        bool m_content_changed = false;
        ~Archive();
        RV init(ISeekableStream* source, OpenMode mode, bool create);
        RV check_edit();
        bool is_open() override;
        R<Vector<EntryInfo>> get_entries() override;
        R<EntryInfo> get_entry(u64 index) override;
        R<u64> find_entry(const c8* name) override;
        R<Ref<IStream>> open_entry(u64 index) override;
        R<u64> add_file(const c8* name, ISeekableStream* source, CompressionMethod compression, u32 level) override;
        R<u64> add_directory(const c8* name) override;
        RV replace_file(u64 index, ISeekableStream* source) override;
        RV rename_entry(u64 index, const c8* name) override;
        RV delete_entry(u64 index) override;
        RV set_compression(u64 index, CompressionMethod compression, u32 level) override;
        RV save(ISeekableStream* destination) override;
        RV discard() override;
    };

    struct [[Luna::struct("{5D516453-2B4E-49AD-A65B-FB3AD864C829}")]] EntryStream : IStream
    {
        luiimpl();
        Ref<Archive> m_owner;
        zip_file_t* m_file = nullptr;
        StreamSource* m_staged_source = nullptr;
        ~EntryStream();
        RV read(void* buffer, usize size, usize* read_bytes) override;
        RV write(const void* buffer, usize size, usize* write_bytes) override;
    };
}
