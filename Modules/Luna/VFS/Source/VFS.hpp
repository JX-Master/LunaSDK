/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file VFS.hpp
* @author JXMaster
* @date 2022/5/24
*/
#pragma once
#include "../VFS.hpp"
#include <Luna/Runtime/Mutex.hpp>
#include "VFS.generated.hpp"

namespace Luna
{
    namespace VFS
    {
        struct MountedFile;
        struct MountedIterator;
        struct [[Luna::struct("{B343C178-0CFE-47B5-B20F-E3BD4687A015}")]] Mount
        {
            Ref<IMutex> m_mutex;
            Path m_mount_path;
            Ref<IFileSystem> m_file_system;
            Vector<MountedFile*> m_files;
            Vector<MountedIterator*> m_iterators;
        };
        struct [[Luna::struct("{0B4F9060-E0D2-4696-BC40-AC9CB8A1C2B2}")]] MountedFile : IFile
        {
            luiimpl();
            Ref<IMutex> m_mutex;
            Ref<Mount> m_mount;
            Ref<IFile> m_file;
            ~MountedFile();
            void invalidate();
            RV read(void* buffer, usize size, usize* count) override;
            RV write(const void* buffer, usize size, usize* count) override;
            R<u64> tell() override;
            RV seek(i64 offset, SeekMode mode) override;
            u64 get_size() override;
            RV set_size(u64 size) override;
            void flush() override;
        };
        struct [[Luna::struct("{FC45F8A4-2465-4767-97B8-3A24591707C0}")]] MountedIterator : IFileIterator
        {
            luiimpl();
            Ref<IMutex> m_mutex;
            Ref<Mount> m_mount;
            Ref<IFileIterator> m_iterator;
            ~MountedIterator();
            void invalidate();
            bool is_valid() override;
            const c8* get_filename() override;
            FileAttributeFlag get_attributes() override;
            bool move_next() override;
        };
    }
}
