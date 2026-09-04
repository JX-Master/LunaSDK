/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file PakFileSystem.cpp
* @author JXMaster
* @date 2026/9/4
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_VFS_API LUNA_EXPORT
#include "PakFileSystemImpl.hpp"
#include "Paths.hpp"

namespace Luna::VFS
{
    static String pak_path(const Path& path)
    {
        return path.empty() ? String() : path.encode();
    }

    LUNA_VFS_API R<Ref<IFileSystem>> new_pak_file_system(IPakStorage* storage, Pak::OpenMode mode, const Pak::Options& options)
    {
        if(!storage) return E_BAD_ARGUMENTS;
        if(mode != Pak::OpenMode::read && mode != Pak::OpenMode::read_write) return E_BAD_ARGUMENTS;
        Ref<IFileSystem> result;
        lutry
        {
            auto instance = new_object<PakFileSystem>();
            instance->m_mutex = new_mutex();
            instance->m_storage = storage;
            lulet(source, storage->open_source());
            if(!source) return E_BAD_DATA;
            luset(instance->m_package, Pak::open_pak(source, mode, options));
            result = move(instance);
        }
        lucatchret;
        return result;
    }

    LUNA_VFS_API R<Ref<IFileSystem>> new_pak_file_system(const c8* native_path, Pak::OpenMode mode, const Pak::Options& options)
    {
        auto path = absolute_native_path(native_path);
        if(failed(path)) return path.errcode();
        auto storage = new_object<NativePakStorage>();
        storage->m_path = path.get().encode();
        return new_pak_file_system(storage, mode, options);
    }

    RV PakFileSystem::flush()
    {
        MutexGuard guard(m_mutex);
        if(m_open_files) return E_BUSY;
        if(m_pending)
        {
            auto result = m_storage->publish(m_pending);
            if(failed(result)) return result;
            m_pending = nullptr;
            return ok;
        }
        if(m_package->is_read_only() || !m_package->is_dirty()) return ok;
        auto created = m_storage->create_output();
        if(failed(created)) return created.errcode();
        if(!created.get()) return E_BAD_DATA;
        auto result = m_package->flush(created.get());
        if(failed(result))
        {
            Error error = result.errcode() == E_ERROR_OBJECT ? get_error() : Error(result.errcode(), String(explain(result.errcode())));
            created.get() = nullptr;
            get_error() = move(error);
            return E_ERROR_OBJECT;
        }
        m_pending = move(created.get());
        result = m_storage->publish(m_pending);
        if(failed(result)) return result;
        m_pending = nullptr;
        return ok;
    }

    R<Ref<IFile>> PakFileSystem::open_file(const Path& path, FileOpenFlag flags, FileCreationMode creation)
    {
        MutexGuard guard(m_mutex);
        Ref<IFile> result;
        lutry
        {
            luexp(validate_relative_path(path));
            if(m_pending && test_flags(flags, FileOpenFlag::write)) return E_BUSY;
            lulet(file, m_package->open_file(pak_path(path).c_str(), flags, creation));
            auto wrapper = new_object<PakFileSystemFile>();
            wrapper->m_file = move(file);
            wrapper->m_file_system = this;
            ++m_open_files;
            result = move(wrapper);
        }
        lucatchret;
        return result;
    }

    R<FileAttribute> PakFileSystem::get_file_attribute(const Path& path)
    {
        MutexGuard guard(m_mutex);
        auto valid = validate_relative_path(path);
        if(failed(valid)) return valid.errcode();
        return m_package->get_file_attribute(pak_path(path).c_str());
    }

    R<Ref<IFileIterator>> PakFileSystem::open_dir(const Path& path)
    {
        MutexGuard guard(m_mutex);
        auto valid = validate_relative_path(path);
        if(failed(valid)) return valid.errcode();
        return m_package->open_dir(pak_path(path).c_str());
    }

    RV PakFileSystem::copy_file(const Path& from_path, const Path& to_path, IFileSystem* to_file_system)
    {
        if(to_file_system && to_file_system != this) return E_NOT_SUPPORTED;
        MutexGuard guard(m_mutex);
        auto from = validate_relative_path(from_path);
        if(failed(from)) return from;
        auto to = validate_relative_path(to_path);
        if(failed(to)) return to;
        if(m_pending) return E_BUSY;
        return m_package->copy_file(pak_path(from_path).c_str(), pak_path(to_path).c_str());
    }

    RV PakFileSystem::move_file(const Path& from_path, const Path& to_path, IFileSystem* to_file_system)
    {
        if(to_file_system && to_file_system != this) return E_NOT_SUPPORTED;
        MutexGuard guard(m_mutex);
        auto from = validate_relative_path(from_path);
        if(failed(from)) return from;
        auto to = validate_relative_path(to_path);
        if(failed(to)) return to;
        if(m_pending) return E_BUSY;
        return m_package->move_file(pak_path(from_path).c_str(), pak_path(to_path).c_str());
    }

    RV PakFileSystem::delete_file(const Path& path)
    {
        MutexGuard guard(m_mutex);
        auto valid = validate_relative_path(path);
        if(failed(valid)) return valid;
        if(m_pending) return E_BUSY;
        return m_package->delete_file(pak_path(path).c_str());
    }

    RV PakFileSystem::create_dir(const Path& path)
    {
        MutexGuard guard(m_mutex);
        auto valid = validate_relative_path(path);
        if(failed(valid)) return valid;
        if(m_pending) return E_BUSY;
        return m_package->create_dir(pak_path(path).c_str());
    }

    PakFileSystemFile::~PakFileSystemFile()
    {
        if(!m_file_system) return;
        MutexGuard guard(m_file_system->m_mutex);
        m_file = nullptr;
        --m_file_system->m_open_files;
    }
    RV PakFileSystemFile::read(void* buffer, usize size, usize* count)
    {
        MutexGuard guard(m_file_system->m_mutex);
        return m_file->read(buffer, size, count);
    }
    RV PakFileSystemFile::write(const void* buffer, usize size, usize* count)
    {
        MutexGuard guard(m_file_system->m_mutex);
        return m_file->write(buffer, size, count);
    }
    R<u64> PakFileSystemFile::tell()
    {
        MutexGuard guard(m_file_system->m_mutex);
        return m_file->tell();
    }
    RV PakFileSystemFile::seek(i64 offset, SeekMode mode)
    {
        MutexGuard guard(m_file_system->m_mutex);
        return m_file->seek(offset, mode);
    }
    u64 PakFileSystemFile::get_size()
    {
        MutexGuard guard(m_file_system->m_mutex);
        return m_file->get_size();
    }
    RV PakFileSystemFile::set_size(u64 size)
    {
        MutexGuard guard(m_file_system->m_mutex);
        return m_file->set_size(size);
    }
    void PakFileSystemFile::flush()
    {
        MutexGuard guard(m_file_system->m_mutex);
        m_file->flush();
    }
}
