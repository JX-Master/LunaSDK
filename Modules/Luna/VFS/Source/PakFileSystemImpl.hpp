/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file PakFileSystemImpl.hpp
* @author JXMaster
* @date 2026/9/4
*/
#pragma once
#include "../PakFileSystem.hpp"
#include <Luna/Runtime/Mutex.hpp>
#include "PakFileSystemImpl.generated.hpp"

namespace Luna::VFS
{
    struct NativePakStorage;
    struct [[Luna::struct("{D410CF9C-1D2A-4696-BFBD-CC728DEC0B9D}")]] NativePakStream : ISeekableStream
    {
        luiimpl();
        NativePakStorage* m_owner = nullptr;
        String m_path;
        Ref<IFile> m_file;
        u64 m_position = 0;
        u64 m_size = 0;
        bool m_writable = false;
        bool m_delete_on_close = false;
        ~NativePakStream();
        RV ensure_file();
        void close_file();
        RV read(void* buffer, usize size, usize* count) override;
        RV write(const void* buffer, usize size, usize* count) override;
        R<u64> tell() override { return m_position; }
        RV seek(i64 offset, SeekMode mode) override;
        u64 get_size() override { return m_size; }
        RV set_size(u64 size) override;
    };
    struct [[Luna::struct("{D0F5F0C4-C056-49F4-A064-DDDC585A1314}")]] NativePakStorage : IPakStorage
    {
        luiimpl();
        String m_path;
        R<Ref<ISeekableStream>> open_source() override;
        R<Ref<ISeekableStream>> create_output() override;
        RV publish(ISeekableStream* output) override;
    };
    struct [[Luna::struct("{C7A7E0F2-C07C-42D9-B034-0EFE4A014583}")]] PakFileSystem : IFileSystem
    {
        luiimpl();
        Ref<IMutex> m_mutex;
        Ref<IPakStorage> m_storage;
        Ref<Pak::IPak> m_package;
        Ref<ISeekableStream> m_pending;
        usize m_open_files = 0;
        R<Ref<IFile>> open_file(const Path& path, FileOpenFlag flags, FileCreationMode creation) override;
        R<FileAttribute> get_file_attribute(const Path& path) override;
        R<Ref<IFileIterator>> open_dir(const Path& path) override;
        RV copy_file(const Path& from_path, const Path& to_path, IFileSystem* to_file_system) override;
        RV move_file(const Path& from_path, const Path& to_path, IFileSystem* to_file_system, FileMoveFlag flags) override;
        RV delete_file(const Path& path) override;
        RV create_dir(const Path& path) override;
        RV flush() override;
    };
    struct [[Luna::struct("{AE9D3960-CAA5-4D95-B9E3-68D011D0AB14}")]] PakFileSystemFile : IFile
    {
        luiimpl();
        Ref<PakFileSystem> m_file_system;
        Ref<IFile> m_file;
        ~PakFileSystemFile();
        RV read(void* buffer, usize size, usize* count) override;
        RV write(const void* buffer, usize size, usize* count) override;
        R<u64> tell() override;
        RV seek(i64 offset, SeekMode mode) override;
        u64 get_size() override;
        RV set_size(u64 size) override;
        void flush() override;
    };
}
