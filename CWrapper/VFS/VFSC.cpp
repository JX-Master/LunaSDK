#include "VFS.h"

#include <Luna/Runtime/Error.hpp>
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Memory.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/VFS/VFS.hpp>

#include <cstring>

namespace
{
luna_errcode_t from_errcode(Luna::ErrCode code)
{
    return static_cast<luna_errcode_t>(code.code);
}

luna_errcode_t from_result(const Luna::RV& result)
{
    return from_errcode(result.errcode());
}

template <typename T>
T* object_as(luna_handle_t object)
{
    return object ? Luna::query_interface<T>(object) : nullptr;
}

const char* duplicate_string(const char* source)
{
    if (!source)
    {
        return nullptr;
    }
    auto size = std::strlen(source);
    auto* buffer = static_cast<char*>(Luna::memalloc(size + 1));
    if (!buffer)
    {
        return nullptr;
    }
    std::memcpy(buffer, source, size + 1);
    return buffer;
}

LunaFileAttribute from_file_attribute(const Luna::FileAttribute& attribute)
{
    return LunaFileAttribute
    {
        static_cast<uint64_t>(attribute.size),
        static_cast<int64_t>(attribute.creation_time),
        static_cast<int64_t>(attribute.last_access_time),
        static_cast<int64_t>(attribute.last_write_time),
        static_cast<uint32_t>(attribute.attributes)
    };
}
}

extern "C"
{
LUNA_VFS_C_API luna_errcode_t luna_vfs_init_module(void)
{
    Luna::Module* module = Luna::module_vfs();
    Luna::RV result = Luna::add_module(module);
    if (!result.valid())
    {
        return from_result(result);
    }
    result = Luna::init_module(module);
    return from_result(result);
}

LUNA_VFS_C_API luna_errcode_t luna_vfs_mount(const char* driver, const char* driver_path, const char* mount_path)
{
    if (!driver || !driver_path || !mount_path)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::VFS::mount(driver, driver_path, mount_path));
}

LUNA_VFS_C_API luna_errcode_t luna_vfs_unmount(const char* mount_path)
{
    if (!mount_path)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::VFS::unmount(mount_path));
}

LUNA_VFS_C_API luna_errcode_t luna_vfs_remount(const char* from_path, const char* to_path)
{
    if (!from_path || !to_path)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::VFS::remount(from_path, to_path));
}

LUNA_VFS_C_API luna_errcode_t luna_vfs_open_file(const char* path, uint32_t flags, uint32_t creation, LunaFileHandle* out_file)
{
    if (!path || !out_file)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_file->object = nullptr;
    out_file->ifile = nullptr;
    out_file->iseekable_stream = nullptr;
    out_file->istream = nullptr;

    auto result = Luna::VFS::open_file(
        path,
        static_cast<Luna::FileOpenFlag>(flags),
        static_cast<Luna::FileCreationMode>(creation));
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }

    Luna::Ref<Luna::IFile> file = Luna::move(result.get());
    Luna::object_t object = file.detach();
    auto ifile = object_as<Luna::IFile>(object);
    auto iseekable_stream = object_as<Luna::ISeekableStream>(object);
    auto istream = object_as<Luna::IStream>(object);
    if (!ifile || !iseekable_stream || !istream)
    {
        Luna::object_release(object);
        return from_errcode(Luna::BasicError::bad_cast());
    }

    out_file->object = object;
    out_file->ifile = ifile;
    out_file->iseekable_stream = iseekable_stream;
    out_file->istream = istream;
    return 0;
}

LUNA_VFS_C_API luna_errcode_t luna_vfs_get_file_attribute(const char* path, LunaFileAttribute* out_attribute)
{
    if (!path || !out_attribute)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    auto result = Luna::VFS::get_file_attribute(path);
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }
    *out_attribute = from_file_attribute(result.get());
    return 0;
}

LUNA_VFS_C_API luna_errcode_t luna_vfs_copy_file(const char* from_path, const char* to_path)
{
    if (!from_path || !to_path)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::VFS::copy_file(from_path, to_path));
}

LUNA_VFS_C_API luna_errcode_t luna_vfs_move_file(const char* from_path, const char* to_path)
{
    if (!from_path || !to_path)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::VFS::move_file(from_path, to_path));
}

LUNA_VFS_C_API luna_errcode_t luna_vfs_delete_file(const char* path)
{
    if (!path)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::VFS::delete_file(path));
}

LUNA_VFS_C_API luna_errcode_t luna_vfs_open_dir(const char* path, LunaFileIteratorHandle* out_iterator)
{
    if (!path || !out_iterator)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_iterator->object = nullptr;
    out_iterator->ifile_iterator = nullptr;

    auto result = Luna::VFS::open_dir(path);
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }

    Luna::Ref<Luna::IFileIterator> iterator = Luna::move(result.get());
    Luna::object_t object = iterator.detach();
    auto ifile_iterator = object_as<Luna::IFileIterator>(object);
    if (!ifile_iterator)
    {
        Luna::object_release(object);
        return from_errcode(Luna::BasicError::bad_cast());
    }

    out_iterator->object = object;
    out_iterator->ifile_iterator = ifile_iterator;
    return 0;
}

LUNA_VFS_C_API luna_errcode_t luna_vfs_create_dir(const char* path)
{
    if (!path)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::VFS::create_dir(path));
}

LUNA_VFS_C_API luna_errcode_t luna_vfs_get_native_path(const char* vfs_path, const char** out_path)
{
    if (!vfs_path || !out_path)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_path = nullptr;

    auto result = Luna::VFS::get_native_path(vfs_path);
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }

    *out_path = duplicate_string(result.get().c_str());
    return *out_path ? 0 : from_errcode(Luna::BasicError::out_of_memory());
}

LUNA_VFS_C_API luna_errcode_t luna_vfs_get_platform_filesystem_driver(const char** out_name)
{
    if (!out_name)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    *out_name = duplicate_string(Luna::VFS::get_platform_filesystem_driver().c_str());
    return *out_name ? 0 : from_errcode(Luna::BasicError::out_of_memory());
}
}
