/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file TestTypes.hpp
* @author JXMaster
* @date 2026/9/4
*/
#pragma once
#include <Luna/VFS/FileSystem.hpp>
#include "TestTypes.generated.hpp"

namespace Luna
{
    struct [[Luna::struct("{F47D6E2C-D71B-44F7-B10C-9C9230BC53D7}")]] CountingFileSystem : VFS::IFileSystem
    {
        luiimpl();
        Ref<VFS::IFileSystem> storage;
        usize read_files = 0;
        usize directories = 0;
        bool fail_publish = false;
        bool fail_sidecar_publish = false;
        bool reject_moves = false;
        R<Ref<IFile>> open_file(const Path& path, FileOpenFlag flags, FileCreationMode mode) override
        {
            if(test_flags(flags, FileOpenFlag::read)) ++read_files;
            return storage->open_file(path, flags, mode);
        }
        R<FileAttribute> get_file_attribute(const Path& path) override { return storage->get_file_attribute(path); }
        R<Ref<IFileIterator>> open_dir(const Path& path) override { ++directories; return storage->open_dir(path); }
        RV move_file(const Path& from, const Path& to, VFS::IFileSystem* target, FileMoveFlag flags) override
        {
            if(reject_moves || (target && target != this)) return E_NOT_SUPPORTED;
            if(fail_publish && test_flags(flags, FileMoveFlag::allow_overwrite)) return E_IO_ERROR;
            if(fail_sidecar_publish && to.extension() == "meta") return E_IO_ERROR;
            return storage->move_file(from, to, nullptr, flags);
        }
        RV copy_file(const Path& from, const Path& to, VFS::IFileSystem* target) override
        {
            if(target && target != this) return E_NOT_SUPPORTED;
            return storage->copy_file(from, to);
        }
        RV delete_file(const Path& path) override { return storage->delete_file(path); }
        RV create_dir(const Path& path) override { return storage->create_dir(path); }
        RV flush() override { return storage->flush(); }
    };
    struct [[Luna::struct("{06CD0C16-BF28-495D-BB82-BED90A55AE98}")]] DatabaseTestData
    {
        i32 value = 42;
    };
}
