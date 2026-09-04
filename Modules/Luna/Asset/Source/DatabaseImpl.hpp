/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file DatabaseImpl.hpp
* @author JXMaster
* @date 2026/9/4
*/
#pragma once
#include "../Database.hpp"
#include "AssetMetaFile.hpp"
#include <Luna/Runtime/Mutex.hpp>
#include <Luna/Runtime/Variant.hpp>
#include "DatabaseImpl.generated.hpp"

namespace Luna::Asset
{
    bool database_contains(const Path& root, const Path& path);
    RV validate_database_path(const Path& path, bool relative, bool allow_empty = false);
    RV validate_metadata(const AssetMetadata& record);
    RV validate_records(Span<const AssetMetadata> records);
    Path database_relative_path(const Path& root, const Path& path);
    Path database_absolute_path(const Path& root, const Path& path);
    AssetMetadata metadata_from_file(const AssetMetaFile& file, const Path& path);
    AssetMetaFile metadata_to_file(const AssetMetadata& record);
    R<AssetMetaFile> read_sidecar(const Path& asset_path);
    RV write_metadata_json(const Path& path, const Variant& data, bool overwrite);

    struct [[Luna::struct("{5966C1D1-FF7F-4505-8280-BBABC28EF0F3}")]] SidecarDatabase : IAssetDatabase
    {
        luiimpl();
        Path m_root;
        DatabaseMode m_mode;
        Ref<IMutex> m_mutex;
        Path get_root() override { return m_root; }
        bool is_read_only() override { return m_mode == DatabaseMode::read; }
        bool is_dirty() override { return false; }
        bool is_metadata_file(const Path& path) override;
        R<AssetMetadata> read_record(const Path& path) override;
        R<Vector<AssetMetadata>> get_records() override;
        R<Vector<AssetMetadata>> read_snapshot() override;
        RV accept_snapshot(Span<const AssetMetadata> records) override;
        RV write_record(const AssetMetadata& record, const Path& previous_path) override;
        RV remove_record(const Path& path) override;
        RV flush() override { return ok; }
    };
    struct [[Luna::struct("{8B9DBFC5-23E9-47E9-8BB3-B0E1EBC99868}")]] FileDatabase : IAssetDatabase
    {
        luiimpl();
        Path m_root;
        Path m_filename;
        DatabaseMode m_mode;
        Ref<IMutex> m_mutex;
        Vector<AssetMetadata> m_records;
        HashMap<Path, usize> m_paths;
        HashMap<Guid, usize> m_guids;
        bool m_dirty = false;
        Path get_root() override { return m_root; }
        bool is_read_only() override { return m_mode == DatabaseMode::read; }
        bool is_dirty() override;
        bool is_metadata_file(const Path& path) override;
        R<AssetMetadata> read_record(const Path& path) override;
        R<Vector<AssetMetadata>> get_records() override;
        R<Vector<AssetMetadata>> read_snapshot() override;
        RV accept_snapshot(Span<const AssetMetadata> records) override;
        RV write_record(const AssetMetadata& record, const Path& previous_path) override;
        RV remove_record(const Path& path) override;
        RV flush() override;
    };
}
