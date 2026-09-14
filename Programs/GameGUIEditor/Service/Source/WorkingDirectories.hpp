/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file WorkingDirectories.hpp
* @author JXMaster
* @date 2026/9/11
*/
#pragma once
#include <Luna/Asset/Database.hpp>
#include <Luna/VFS/FileSystem.hpp>
#include <Luna/Runtime/UniquePtr.hpp>
#include "WorkingDirectories.generated.hpp"

namespace Luna::GameGUIEditor
{
    // NativeFileSystem supplies storage; this wrapper enforces the editor's directory boundary,
    // including symlink targets. It does not change VFS or native filesystem semantics globally.
    struct [[Luna::struct("{EBBF92C8-52B3-4E8F-A23A-5B36EF739724}")]] DirectoryFileSystem : VFS::IFileSystem
    {
        luiimpl();
        Path native_root;
        Ref<VFS::IFileSystem> storage;
        R<Path> check(const Path& path, bool directory = false);
        virtual R<Ref<IFile>> open_file(const Path& path, FileOpenFlag flags, FileCreationMode creation) override;
        virtual R<FileAttribute> get_file_attribute(const Path& path) override;
        virtual R<Ref<IFileIterator>> open_dir(const Path& path) override;
        virtual RV copy_file(const Path& from, const Path& to, VFS::IFileSystem* destination) override;
        virtual RV move_file(const Path& from, const Path& to, VFS::IFileSystem* destination, FileMoveFlag flags) override;
        virtual RV delete_file(const Path& path) override;
        virtual RV create_dir(const Path& path) override;
        virtual R<Name> get_native_path(const Path& path) override;
        virtual RV flush() override;
    };

    struct WorkingDirectory
    {
        u64 id = 0;
        Path native_root;
        Path vfs_root;
        Ref<DirectoryFileSystem> file_system;
        Ref<Asset::IAssetDatabase> database;
        Vector<Asset::AssetMetadata> records;
        Vector<Path> folders;
        Vector<Asset::asset_t> touched_assets;
        bool closing = false;
        u64 close_token = 0;
        bool registered = false;
    };

    class WorkingDirectories
    {
    public:
        Vector<UniquePtr<WorkingDirectory>> directories;
        R<WorkingDirectory*> open(const Path& native_root);
        R<WorkingDirectory*> get(u64 id, bool allow_closing = false);
        R<WorkingDirectory*> owner(const Path& path, bool allow_closing = false);
        R<Path> resolve_native(Path path);
        RV rebuild_index(WorkingDirectory& directory);
        RV flush(WorkingDirectory& directory);
        RV unload_data(WorkingDirectory& directory);
        RV close(WorkingDirectory& directory);
        Variant describe(const WorkingDirectory& directory, bool entries = false) const;
        RV validate_asset_path(const Path& path, bool allow_closing = false);
        void track_asset(WorkingDirectory& directory, Asset::asset_t asset);
    };
}
