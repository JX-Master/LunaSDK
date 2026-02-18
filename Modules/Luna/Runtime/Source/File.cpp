/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file File.cpp
* @author JXMaster
* @date 2019/9/29
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "File.hpp"

namespace Luna
{
    LUNA_RUNTIME_API R<Ref<IFile>> open_file(const c8* filename, FileOpenFlag flags, FileCreationMode creation)
    {
        auto file = new_object<File>();
        auto r = Platform::open_file(filename, flags, creation, file->m_file);
        if(r != Platform::Result::success)
        {
            ErrCode err = encode_platform_result(r).errcode();
#ifdef LUNA_DEBUG
            return set_error(err, "Failed to open file %s: %s", filename, explain(err));
#else
            return err;
#endif
        }
        return Ref<IFile>(file);
    }
    LUNA_RUNTIME_API R<Blob> load_file_data(IFile* file)
    {
        Blob ret;
        lutry
        {
            lulet(cursor, file->tell());
            luexp(file->seek(0, SeekMode::begin));
            ret.resize((usize)file->get_size());
            luexp(file->read(ret.data(), ret.size()));
            luexp(file->seek((i64)cursor, SeekMode::begin));
        }
        lucatchret;
        return ret;
    }
    LUNA_RUNTIME_API R<FileAttribute> get_file_attribute(const c8* filename)
    {
        FileAttribute attr;
        auto r = Platform::get_file_attribute(filename, attr);
        if(r != Platform::Result::success) return encode_platform_result(r).errcode();
        return attr;
    }
    LUNA_RUNTIME_API RV copy_file(const c8* from_path, const c8* to_path)
    {
        return encode_platform_result(Platform::copy_file(from_path, to_path));
    }
    LUNA_RUNTIME_API RV move_file(const c8* from_path, const c8* to_path)
    {
        return encode_platform_result(Platform::move_file(from_path, to_path));
    }
    LUNA_RUNTIME_API RV delete_file(const c8* filename)
    {
        return encode_platform_result(Platform::delete_file(filename));
    }
    LUNA_RUNTIME_API R<Ref<IFileIterator>> open_dir(const c8* dir_path)
    {
        auto iter = new_object<FileIterator>();
        auto r = Platform::open_dir(dir_path, iter->m_iter);
        if(r != Platform::Result::success) return encode_platform_result(r).errcode();
        return Ref<IFileIterator>(iter);
    }
    LUNA_RUNTIME_API RV create_dir(const c8* pathname)
    {
        return encode_platform_result(Platform::create_dir(pathname));
    }
    LUNA_RUNTIME_API usize get_current_dir(c8* buffer, usize buffer_size)
    {
        return Platform::get_current_dir(buffer, buffer_size);
    }
    LUNA_RUNTIME_API RV set_current_dir(const c8* path)
    {
        return encode_platform_result(Platform::set_current_dir(path));
    }
    LUNA_RUNTIME_API usize get_process_path(c8* buffer, usize buffer_size)
    {
        return Platform::get_process_path(buffer, buffer_size);
    }
}