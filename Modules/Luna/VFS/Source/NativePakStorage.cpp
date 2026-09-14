/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file NativePakStorage.cpp
* @author JXMaster
* @date 2026/9/4
*/
#include "PakFileSystemImpl.hpp"
#include <Luna/Runtime/Guid.hpp>
#include <Luna/Runtime/Random.hpp>
#include <Luna/Runtime/Log.hpp>

namespace Luna::VFS
{
    NativePakStream::~NativePakStream()
    {
        close_file();
        if(m_delete_on_close)
        {
            auto result = Luna::delete_file(m_path.c_str());
            if(failed(result) && unwrap_errcode(result.errcode()) != E_NOT_FOUND)
            {
                log_error("VFS", "Failed to remove incomplete Pak output %s: %s", m_path.c_str(), explain(result.errcode()));
            }
        }
    }

    RV NativePakStream::ensure_file()
    {
        lutry
        {
            if(!m_file)
            {
                auto flags = FileOpenFlag::read;
                if(m_writable) flags |= FileOpenFlag::write;
                luset(m_file, Luna::open_file(m_path.c_str(), flags, FileCreationMode::open_existing));
            }
            luexp(m_file->seek((i64)m_position, SeekMode::begin));
        }
        lucatchret;
        return ok;
    }

    void NativePakStream::close_file()
    {
        if(m_file && m_writable) m_file->flush();
        m_file = nullptr;
    }

    RV NativePakStream::read(void* buffer, usize size, usize* count)
    {
        if(count) *count = 0;
        auto ready = ensure_file();
        if(failed(ready)) return ready;
        usize amount = 0;
        auto result = m_file->read(buffer, min<usize>(size, 1024 * 1024), &amount);
        m_position += amount;
        if(count) *count = amount;
        return result;
    }

    RV NativePakStream::write(const void* buffer, usize size, usize* count)
    {
        if(count) *count = 0;
        if(!m_writable) return E_ACCESS_DENIED;
        if(size > (u64)I64_MAX - m_position) return E_OUT_OF_RANGE;
        auto ready = ensure_file();
        if(failed(ready)) return ready;
        usize written = 0;
        while(written < size)
        {
            usize amount = 0;
            auto result = m_file->write((const byte_t*)buffer + written, min<usize>(size - written, 1024 * 1024), &amount);
            written += amount;
            m_position += amount;
            m_size = max(m_size, m_position);
            if(count) *count = written;
            if(failed(result)) return result;
            if(!amount) return E_IO_ERROR;
        }
        return ok;
    }

    RV NativePakStream::seek(i64 offset, SeekMode mode)
    {
        u64 base;
        switch(mode)
        {
        case SeekMode::begin: base = 0; break;
        case SeekMode::current: base = m_position; break;
        case SeekMode::end: base = m_size; break;
        default: return E_BAD_ARGUMENTS;
        }
        if(base > I64_MAX || offset < -(i64)base || (offset > 0 && base > (u64)(I64_MAX - offset))) return E_OUT_OF_RANGE;
        m_position = (u64)((i64)base + offset);
        return ok;
    }

    RV NativePakStream::set_size(u64 size)
    {
        if(!m_writable) return E_ACCESS_DENIED;
        if(size > I64_MAX) return E_OUT_OF_RANGE;
        auto ready = ensure_file();
        if(failed(ready)) return ready;
        auto result = m_file->set_size(size);
        m_size = m_file->get_size();
        return result;
    }

    R<Ref<ISeekableStream>> NativePakStorage::open_source()
    {
        Ref<ISeekableStream> result;
        lutry
        {
            lulet(attr, Luna::get_file_attribute(m_path.c_str()));
            if(test_flags(attr.attributes, FileAttributeFlag::directory)) return E_IS_DIRECTORY;
            lulet(file, Luna::open_file(m_path.c_str(), FileOpenFlag::read, FileCreationMode::open_existing));
            auto stream = new_object<NativePakStream>();
            stream->m_owner = this;
            stream->m_path = m_path;
            stream->m_size = file->get_size();
            stream->m_file = move(file);
            result = move(stream);
        }
        lucatchret;
        return result;
    }

    R<Ref<ISeekableStream>> NativePakStorage::create_output()
    {
        for(u32 attempt = 0; attempt < 16; ++attempt)
        {
            c8 guid[GUID_STRING_LENGTH + 1]{};
            lupanic_if_failed(encode_guid(random_guid(), guid, GUID_STRING_LENGTH));
            String filename(".luna-pak-");
            filename.append(guid);
            filename.append(".tmp");
            Path path(m_path);
            path.pop_back();
            path.push_back(Name(filename));
            String encoded = path.encode();
            auto opened = Luna::open_file(encoded.c_str(), FileOpenFlag::read | FileOpenFlag::write, FileCreationMode::create_new);
            if(failed(opened))
            {
                if(unwrap_errcode(opened.errcode()) == E_ALREADY_EXISTS) continue;
                return opened.errcode();
            }
            auto stream = new_object<NativePakStream>();
            stream->m_owner = this;
            stream->m_path = move(encoded);
            stream->m_file = move(opened.get());
            stream->m_writable = true;
            stream->m_delete_on_close = true;
            return Ref<ISeekableStream>(stream);
        }
        return E_ALREADY_EXISTS;
    }

    RV NativePakStorage::publish(ISeekableStream* output)
    {
        Ref<ISeekableStream> reference(output);
        auto stream = reference.as<NativePakStream>();
        if(!stream || stream->m_owner != this) return E_BAD_ARGUMENTS;
        // Pak retains this logical stream. Close its native handle while keeping its
        // cursor/size so Windows can rename it; reads reopen the selected path on demand.
        stream->m_delete_on_close = false;
        stream->close_file();
        stream->m_writable = false;
        auto published = Luna::move_file(stream->m_path.c_str(), m_path.c_str(),
            FileMoveFlag::allow_overwrite | FileMoveFlag::no_copy);
        if(failed(published))
        {
            return set_error(unwrap_errcode(published.errcode()), "Failed to publish Pak %s; complete output retained at %s: %s",
                m_path.c_str(), stream->m_path.c_str(), explain(published.errcode()));
        }
        stream->m_path = m_path;
        // Publication has committed. A later failure to reopen is a read error, not
        // a failed publication that could safely retry renaming the old temporary path.
        return ok;
    }
}
