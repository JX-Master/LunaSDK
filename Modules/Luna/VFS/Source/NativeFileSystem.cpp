/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file NativeFileSystem.cpp
* @author JXMaster
* @date 2026/9/4
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_VFS_API LUNA_EXPORT
#include "NativeFileSystem.hpp"
#include "Paths.hpp"

namespace Luna::VFS
{
    R<String> NativeFileSystem::resolve(const Path& path)
    {
        auto valid = validate_relative_path(path);
        if(failed(valid)) return valid.errcode();
        Path absolute = m_root;
        absolute.append(path);
        return absolute.encode();
    }
    R<Ref<IFile>> NativeFileSystem::open_file(const Path& path, FileOpenFlag flags, FileCreationMode creation)
    {
        auto native = resolve(path);
        if(failed(native)) return native.errcode();
        return Luna::open_file(native.get().c_str(), flags, creation);
    }
    R<FileAttribute> NativeFileSystem::get_file_attribute(const Path& path)
    {
        auto native = resolve(path);
        if(failed(native)) return native.errcode();
        return Luna::get_file_attribute(native.get().c_str());
    }
    R<Ref<IFileIterator>> NativeFileSystem::open_dir(const Path& path)
    {
        auto native = resolve(path);
        if(failed(native)) return native.errcode();
        return Luna::open_dir(native.get().c_str());
    }
    RV NativeFileSystem::copy_file(const Path& from_path, const Path& to_path, IFileSystem* to_file_system)
    {
        Ref<IFileSystem> destination(to_file_system ? to_file_system : this);
        auto native = destination.as<NativeFileSystem>();
        if(!native) return E_NOT_SUPPORTED;
        auto from = resolve(from_path);
        if(failed(from)) return from.errcode();
        auto to = native->resolve(to_path);
        if(failed(to)) return to.errcode();
        return Luna::copy_file(from.get().c_str(), to.get().c_str());
    }
    RV NativeFileSystem::move_file(const Path& from_path, const Path& to_path, IFileSystem* to_file_system)
    {
        Ref<IFileSystem> destination(to_file_system ? to_file_system : this);
        auto native = destination.as<NativeFileSystem>();
        if(!native) return E_NOT_SUPPORTED;
        auto from = resolve(from_path);
        if(failed(from)) return from.errcode();
        auto to = native->resolve(to_path);
        if(failed(to)) return to.errcode();
        return Luna::move_file(from.get().c_str(), to.get().c_str());
    }
    RV NativeFileSystem::delete_file(const Path& path)
    {
        auto native = resolve(path);
        if(failed(native)) return native.errcode();
        return Luna::delete_file(native.get().c_str());
    }
    RV NativeFileSystem::create_dir(const Path& path)
    {
        auto native = resolve(path);
        if(failed(native)) return native.errcode();
        return Luna::create_dir(native.get().c_str());
    }
    R<Name> NativeFileSystem::get_native_path(const Path& path)
    {
        auto native = resolve(path);
        if(failed(native)) return native.errcode();
        return Name(native.get());
    }
    LUNA_VFS_API R<Ref<IFileSystem>> new_native_file_system(const c8* native_path)
    {
        Ref<IFileSystem> result;
        lutry
        {
            lulet(root, absolute_native_path(native_path));
            lulet(attr, Luna::get_file_attribute(root.encode().c_str()));
            if(!test_flags(attr.attributes, FileAttributeFlag::directory)) return E_NOT_DIRECTORY;
            auto instance = new_object<NativeFileSystem>();
            instance->m_root = move(root);
            result = move(instance);
        }
        lucatchret;
        return result;
    }
}
