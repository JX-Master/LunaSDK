/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file VFS.cpp
* @author JXMaster
* @date 2022/5/24
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_VFS_API LUNA_EXPORT
#include "VFS.hpp"
#include "VFS.meta.generated.hpp"
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Log.hpp>
#include "Paths.hpp"
#include <Luna/Pak/Pak.hpp>

namespace Luna::VFS
{
    static Vector<Ref<Mount>> g_mounts;
    static Ref<IMutex> g_mutex;
    static bool g_closing = false;

    static Error capture_error(ResultCode code)
    {
        if(code == E_ERROR_OBJECT) return get_error();
        return Error(code, String(explain(code)));
    }

    LUNA_VFS_API RV mount(IFileSystem* file_system, const Path& mount_path)
    {
        if(!g_mutex) return E_BAD_CALLING_TIME;
        if(!file_system) return E_BAD_ARGUMENTS;
        MutexGuard guard(g_mutex);
        if(g_closing) return E_BAD_CALLING_TIME;
        lutry
        {
            luexp(validate_path(mount_path));
            for(const auto& existing : g_mounts)
            {
                if(existing->m_mount_path == mount_path) return E_ALREADY_EXISTS;
            }
            auto device = new_object<Mount>();
            device->m_mutex = new_mutex();
            device->m_mount_path = mount_path;
            device->m_file_system = file_system;
            g_mounts.push_back(move(device));
        }
        lucatchret;
        return ok;
    }

    static R<usize> find_mount(const Path& path)
    {
        if(g_closing) return E_BAD_CALLING_TIME;
        auto valid = validate_path(path);
        if(failed(valid)) return valid.errcode();
        for(usize i = 0; i < g_mounts.size(); ++i)
        {
            if(g_mounts[i]->m_mount_path == path) return i;
        }
        return E_NOT_FOUND;
    }

    static Vector<Ref<IFileSystem>> unique_file_systems()
    {
        Vector<Ref<IFileSystem>> result;
        for(const auto& mount : g_mounts)
        {
            bool found = false;
            for(const auto& existing : result)
            {
                if(existing == mount->m_file_system)
                {
                    found = true;
                    break;
                }
            }
            if(!found) result.push_back(mount->m_file_system);
        }
        return result;
    }

    LUNA_VFS_API RV flush_all()
    {
        if(!g_mutex) return E_BAD_CALLING_TIME;
        MutexGuard guard(g_mutex);
        if(g_closing) return E_BAD_CALLING_TIME;
        Error first_error;
        for(const auto& file_system : unique_file_systems())
        {
            auto result = file_system->flush();
            if(failed(result) && succeeded(first_error.code)) first_error = capture_error(result.errcode());
        }
        if(failed(first_error.code))
        {
            get_error() = move(first_error);
            return E_ERROR_OBJECT;
        }
        return ok;
    }

    LUNA_VFS_API RV unmount(const Path& mount_path)
    {
        if(!g_mutex) return E_BAD_CALLING_TIME;
        MutexGuard guard(g_mutex);
        lutry
        {
            lulet(index, find_mount(mount_path));
            auto device = g_mounts[index];
            MutexGuard device_guard(device->m_mutex);
            if(!device->m_files.empty() || !device->m_iterators.empty()) return E_BUSY;
            luexp(device->m_file_system->flush());
            g_mounts.erase(g_mounts.begin() + index);
        }
        lucatchret;
        return ok;
    }

    LUNA_VFS_API RV remount(const Path& from_path, const Path& to_path)
    {
        if(!g_mutex) return E_BAD_CALLING_TIME;
        MutexGuard guard(g_mutex);
        lutry
        {
            lulet(index, find_mount(from_path));
            luexp(validate_path(to_path));
            if(from_path == to_path) return ok;
            for(const auto& device : g_mounts)
            {
                if(device->m_mount_path == to_path) return E_ALREADY_EXISTS;
            }
            g_mounts[index]->m_mount_path = to_path;
        }
        lucatchret;
        return ok;
    }

    static R<Ref<Mount>> route_path(const Path& path, Path& relative)
    {
        if(g_closing) return E_BAD_CALLING_TIME;
        auto valid = validate_path(path);
        if(failed(valid)) return valid.errcode();
        Ref<Mount> best;
        for(const auto& device : g_mounts)
        {
            const auto& base = device->m_mount_path;
            if(path.root() != base.root() || path.flags() != base.flags()) continue;
            if(path.is_subpath_of(base) && (!best || base.size() > best->m_mount_path.size())) best = device;
        }
        if(!best) return E_NOT_FOUND;
        relative.assign_relative(best->m_mount_path, path);
        relative.root().reset();
        return best;
    }

    static RV copy_between_file_systems(IFileSystem* source, IFileSystem* destination, const Path& from_path, const Path& to_path)
    {
        Ref<IFile> input;
        Ref<IFile> output;
        bool created = false;
        lutry
        {
            lulet(attr, source->get_file_attribute(from_path));
            if(test_flags(attr.attributes, FileAttributeFlag::directory)) return E_IS_DIRECTORY;
            luset(input, source->open_file(from_path,
                FileOpenFlag::read, FileCreationMode::open_existing));
            if(!input) luthrow(E_BAD_DATA);
            luset(output, destination->open_file(to_path,
                FileOpenFlag::write, FileCreationMode::create_new));
            if(!output) luthrow(E_BAD_DATA);
            created = true;
            byte_t buffer[65536];
            u64 remaining = input->get_size();
            while(remaining)
            {
                usize count = 0;
                usize amount = (usize)min<u64>(remaining, sizeof(buffer));
                luexp(input->read(buffer, amount, &count));
                if(!count || count > amount) luthrow(E_BAD_DATA);
                remaining -= count;
                usize written = 0;
                while(written < count)
                {
                    usize progress = 0;
                    luexp(output->write(buffer + written, count - written, &progress));
                    if(!progress || progress > count - written) luthrow(E_IO_ERROR);
                    written += progress;
                }
            }
            usize count = 0;
            luexp(input->read(buffer, 1, &count));
            if(count) luthrow(E_BAD_DATA);
        }
        lucatch
        {
            Error error = capture_error(luerr);
            input = nullptr;
            output = nullptr;
            if(created)
            {
                auto cleanup = destination->delete_file(to_path);
                if(failed(cleanup) && unwrap_errcode(cleanup.errcode()) != E_NOT_FOUND)
                {
                    error.message.append("; failed to remove the partial destination: ");
                    error.message.append(explain(cleanup.errcode()));
                }
            }
            get_error() = move(error);
            return E_ERROR_OBJECT;
        }
        return ok;
    }

    LUNA_VFS_API R<Ref<IFile>> open_file(const Path& path, FileOpenFlag flags, FileCreationMode creation)
    {
        if(!g_mutex) return E_BAD_CALLING_TIME;
        MutexGuard guard(g_mutex);
        Ref<IFile> result;
        lutry
        {
            Path relative;
            lulet(device, route_path(path, relative));
            MutexGuard device_guard(device->m_mutex);
            lulet(file, device->m_file_system->open_file(relative, flags, creation));
            if(!file) return E_BAD_DATA;
            auto wrapped = new_object<MountedFile>();
            wrapped->m_mutex = device->m_mutex;
            wrapped->m_mount = device;
            wrapped->m_file = move(file);
            device->m_files.push_back(wrapped.get());
            result = move(wrapped);
        }
        lucatchret;
        return result;
    }

    LUNA_VFS_API R<FileAttribute> get_file_attribute(const Path& path)
    {
        if(!g_mutex) return E_BAD_CALLING_TIME;
        MutexGuard guard(g_mutex);
        FileAttribute result{};
        lutry
        {
            Path relative;
            lulet(device, route_path(path, relative));
            MutexGuard device_guard(device->m_mutex);
            luset(result, device->m_file_system->get_file_attribute(relative));
        }
        lucatchret;
        return result;
    }

    LUNA_VFS_API RV copy_file(const Path& from_path, const Path& to_path)
    {
        if(!g_mutex) return E_BAD_CALLING_TIME;
        MutexGuard guard(g_mutex);
        lutry
        {
            Path source_path, destination_path;
            lulet(from, route_path(from_path, source_path));
            lulet(to, route_path(to_path, destination_path));
            MutexGuard source_guard(from->m_mutex);
            MutexGuard destination_guard(to->m_mutex);
            auto result = from->m_file_system->copy_file(source_path, destination_path, to->m_file_system);
            if(succeeded(result) || unwrap_errcode(result.errcode()) != E_NOT_SUPPORTED) return result;
            luexp(copy_between_file_systems(from->m_file_system, to->m_file_system, source_path, destination_path));
        }
        lucatchret;
        return ok;
    }

    LUNA_VFS_API RV move_file(const Path& from_path, const Path& to_path, FileMoveFlag flags)
    {
        if(!g_mutex) return E_BAD_CALLING_TIME;
        if((flags & ~(FileMoveFlag::allow_overwrite | FileMoveFlag::no_copy)) != FileMoveFlag::none) return E_BAD_ARGUMENTS;
        MutexGuard guard(g_mutex);
        lutry
        {
            Path source_path, destination_path;
            lulet(from, route_path(from_path, source_path));
            lulet(to, route_path(to_path, destination_path));
            MutexGuard source_guard(from->m_mutex);
            MutexGuard destination_guard(to->m_mutex);
            auto result = from->m_file_system->move_file(source_path, destination_path, to->m_file_system, flags);
            if(succeeded(result) || unwrap_errcode(result.errcode()) != E_NOT_SUPPORTED) return result;
            if(test_flags(flags, FileMoveFlag::no_copy)) return result;
            if(test_flags(flags, FileMoveFlag::allow_overwrite))
            {
                auto destination = to->m_file_system->get_file_attribute(destination_path);
                if(succeeded(destination)) return E_NOT_SUPPORTED;
                if(unwrap_errcode(destination.errcode()) != E_NOT_FOUND) return destination.errcode();
            }
            luexp(copy_between_file_systems(from->m_file_system, to->m_file_system, source_path, destination_path));
            luexp(from->m_file_system->delete_file(source_path));
        }
        lucatchret;
        return ok;
    }

    LUNA_VFS_API RV delete_file(const Path& path)
    {
        if(!g_mutex) return E_BAD_CALLING_TIME;
        MutexGuard guard(g_mutex);
        lutry
        {
            Path relative;
            lulet(device, route_path(path, relative));
            MutexGuard device_guard(device->m_mutex);
            luexp(device->m_file_system->delete_file(relative));
        }
        lucatchret;
        return ok;
    }

    LUNA_VFS_API R<Ref<IFileIterator>> open_dir(const Path& path)
    {
        if(!g_mutex) return E_BAD_CALLING_TIME;
        MutexGuard guard(g_mutex);
        Ref<IFileIterator> result;
        lutry
        {
            Path relative;
            lulet(device, route_path(path, relative));
            MutexGuard device_guard(device->m_mutex);
            lulet(iterator, device->m_file_system->open_dir(relative));
            if(!iterator) return E_BAD_DATA;
            auto wrapped = new_object<MountedIterator>();
            wrapped->m_mutex = device->m_mutex;
            wrapped->m_mount = device;
            wrapped->m_iterator = move(iterator);
            device->m_iterators.push_back(wrapped.get());
            result = move(wrapped);
        }
        lucatchret;
        return result;
    }

    LUNA_VFS_API RV create_dir(const Path& path)
    {
        if(!g_mutex) return E_BAD_CALLING_TIME;
        MutexGuard guard(g_mutex);
        lutry
        {
            Path relative;
            lulet(device, route_path(path, relative));
            MutexGuard device_guard(device->m_mutex);
            luexp(device->m_file_system->create_dir(relative));
        }
        lucatchret;
        return ok;
    }

    LUNA_VFS_API R<Name> get_native_path(const Path& path)
    {
        if(!g_mutex) return E_BAD_CALLING_TIME;
        MutexGuard guard(g_mutex);
        Name result;
        lutry
        {
            Path relative;
            lulet(device, route_path(path, relative));
            MutexGuard device_guard(device->m_mutex);
            luset(result, device->m_file_system->get_native_path(relative));
        }
        lucatchret;
        return result;
    }

    struct VFSModule : Module
    {
        const c8* get_name() override { return "VFS"; }
        RV on_register() override
        {
            return add_dependency_module(this, module_pak());
        }
        RV on_init() override
        {
            Meta::register_VFS_types();
            g_closing = false;
            g_mutex = new_mutex();
            return ok;
        }
        void on_close() override
        {
            if(!g_mutex) return;
            MutexGuard guard(g_mutex);
            g_closing = true;
            for(auto& device : g_mounts)
            {
                MutexGuard device_guard(device->m_mutex);
                while(!device->m_files.empty())
                {
                    auto file = device->m_files.back();
                    device->m_files.pop_back();
                    file->invalidate();
                }
                while(!device->m_iterators.empty())
                {
                    auto iterator = device->m_iterators.back();
                    device->m_iterators.pop_back();
                    iterator->invalidate();
                }
            }
            for(const auto& file_system : unique_file_systems())
            {
                auto flushed = file_system->flush();
                if(failed(flushed)) log_error("VFS", "Failed to flush a filesystem at shutdown: %s", explain(flushed.errcode()));
            }
            g_mounts.clear();
            g_mounts.shrink_to_fit();
            g_mutex = nullptr;
        }
    };
}

namespace Luna
{
    LUNA_VFS_API Module* module_vfs()
    {
        static VFS::VFSModule module;
        return &module;
    }
}
