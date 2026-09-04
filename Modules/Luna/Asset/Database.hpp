/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Database.hpp
* @author JXMaster
* @date 2026/9/4
*/
#pragma once
#include "Asset.hpp"
#include <Luna/Runtime/Span.hpp>
#include "Database.generated.hpp"

namespace Luna::Asset
{
    //! @addtogroup Asset
    //! @{

    //! One persistent asset record, independent of runtime handles and loaded objects.
    struct AssetMetadata
    {
        //! The nonzero persistent asset GUID.
        Guid guid;
        //! The nonempty asset path relative to its database root.
        Path path;
        //! The asset type name. Its loader need not be registered when reading metadata.
        Name type;
        //! Named data units. The main data unit is implicit in the asset type.
        Vector<AssetDataUnitDesc> data_units;
    };

    //! Selects metadata database access. This does not change the underlying VFS access mode.
    enum class DatabaseMode : u8
    {
        //! Permits reading metadata only.
        read,
        //! Permits reading and modifying metadata.
        read_write
    };

    //! Stores metadata for one VFS root and its complete descendant tree.
    //! @details All methods are pure virtual. Implementations synchronize their operations.
    //! Paths in records and method arguments are relative to get_root(). No method reads asset payloads.
    //! A registered database is retained by Asset; mutate/reload it only through coordinating Asset APIs.
    //! Provider methods must not reenter Asset. Callers must keep its VFS bindings and backing files stable.
    struct [[Luna::interface("{7EC400D1-196D-4353-A0D0-F231BA52D9A7}")]] IAssetDatabase : virtual Interface
    {
        //! Returns the immutable VFS root of the database.
        virtual Path get_root() = 0;
        //! Checks whether metadata writes are prohibited.
        virtual bool is_read_only() = 0;
        //! Checks whether accepted record edits still need flush.
        virtual bool is_dirty() = 0;
        //! Checks whether a relative file path is owned by metadata storage and must not be treated as payload.
        virtual bool is_metadata_file(const Path& path) = 0;
        //! Reads the logical record at a relative asset path, including accepted unflushed edits.
        //! Returns E_META_FILE_NOT_FOUND when no record exists.
        virtual R<AssetMetadata> read_record(const Path& path) = 0;
        //! Returns the complete logical record snapshot, including unflushed edits.
        virtual R<Vector<AssetMetadata>> get_records() = 0;
        //! Reads a fresh snapshot from VFS without modifying the provider's logical records.
        //! This enables validation before registry publication; failure leaves both views unchanged.
        virtual R<Vector<AssetMetadata>> read_snapshot() = 0;
        //! Accepts a validated storage snapshot as the clean logical view, without writing VFS.
        //! Returns E_BUSY when dirty. Validation failure leaves the old view unchanged.
        //! Registered providers are updated through reload_asset_database.
        virtual RV accept_snapshot(Span<const AssetMetadata> records) = 0;
        //! Inserts or updates a record. An existing destination must belong to the same GUID.
        //! @param[in] record The record to write.
        //! @param[in] previous_path Optional old relative path when relocating this record within the database.
        //! @details Sidecars save immediately; single-file databases stage changes until flush.
        //! Read-only providers return E_NOT_SUPPORTED before changing storage.
        virtual RV write_record(const AssetMetadata& record, const Path& previous_path = Path()) = 0;
        //! Removes a record without deleting asset payloads. Missing records return E_META_FILE_NOT_FOUND.
        virtual RV remove_record(const Path& path) = 0;
        //! Writes pending metadata through VFS. Does not save data units or flush a VFS filesystem.
        //! Failure retains dirty state for retry. No physical-media durability is promised.
        virtual RV flush() = 0;
    };

    //! Creates a sidecar provider for a VFS root. Does not scan or register assets.
    //! Reads legacy metadata versions 1 and 2 and writes version 2.
    LUNA_ASSET_API R<Ref<IAssetDatabase>> new_sidecar_database(const Path& root,
        DatabaseMode mode = DatabaseMode::read_write);
    //! Opens one centralized JSON database. Reads this file only and does not register assets.
    //! @param[in] root The VFS root covered by all records.
    //! @param[in] filename The nonempty database file path relative to root. Its filename must not use the
    //! `.meta` extension or the reserved `.luna-asset-` prefix.
    //! @param[in] mode The metadata access policy.
    LUNA_ASSET_API R<Ref<IAssetDatabase>> open_file_database(const Path& root,
        const Path& filename = Path("assets.db"), DatabaseMode mode = DatabaseMode::read);
    //! Creates an empty writable centralized database. An existing destination is never overwritten.
    //! The parent directory must exist. Does not register assets.
    //! Root and filename follow the same rules as open_file_database.
    LUNA_ASSET_API R<Ref<IAssetDatabase>> create_file_database(const Path& root,
        const Path& filename = Path("assets.db"));
    //! Exports a complete logical metadata snapshot to a new centralized database under another root.
    //! Preserves GUIDs and relative paths. Does not register assets, copy payloads or remove source sidecars.
    //! The destination must not exist and its parent must exist.
    //! Destination root and filename follow the same rules as open_file_database.
    LUNA_ASSET_API RV export_asset_database(IAssetDatabase* source, const Path& destination_root,
        const Path& filename = Path("assets.db"));

    //! Registers a retained provider and all its records after validating the complete snapshot.
    //! Rejects overlapping roots, existing registered assets owned elsewhere and duplicate GUIDs/paths.
    //! Unregistered GUID placeholders may be populated. Failure does not partially register records.
    LUNA_ASSET_API RV register_asset_database(IAssetDatabase* database);
    //! Reloads a registered provider from storage. Rejects dirty providers and incompatible loaded units.
    //! Removed records are unregistered only when their data units are unloaded and idle.
    //! Failure preserves the registry and any cached provider snapshot.
    LUNA_ASSET_API RV reload_asset_database(IAssetDatabase* database);
    //! Flushes and unregisters a provider. All owned assets must be unloaded and idle.
    //! Handles remain valid unregistered placeholders; failure retains the registration.
    LUNA_ASSET_API RV unregister_asset_database(IAssetDatabase* database);
    //! Flushes all registered metadata databases, attempts each, and returns the first failure.
    //! Does not implicitly call save_asset_meta or save any data unit.
    LUNA_ASSET_API RV flush_asset_databases();
    //! Returns the explicitly registered database covering a path, or E_NOT_FOUND for legacy sidecar paths.
    LUNA_ASSET_API R<Ref<IAssetDatabase>> get_asset_database(const Path& path);
    //! @}
}
