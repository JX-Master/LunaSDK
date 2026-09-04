/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file File.cpp
* @author JXMaster
* @date 2026/9/4
*/
#include "Package.hpp"

namespace Luna::Pak
{
    R<u64> seek_position(u64 position, u64 size, i64 offset, SeekMode mode)
    {
        u64 base;
        switch(mode)
        {
        case SeekMode::begin: base = 0; break;
        case SeekMode::current: base = position; break;
        case SeekMode::end: base = size; break;
        default: return E_BAD_ARGUMENTS;
        }
        if(base > I64_MAX || offset < -(i64)base || (offset > 0 && base > (u64)(I64_MAX - offset))) return E_OUT_OF_RANGE;
        return (u64)((i64)base + offset);
    }

    RV write_all(IStream* stream, const void* buffer, usize size, usize* written)
    {
        usize done = 0;
        if(written) *written = 0;
        while(done < size)
        {
            usize count = 0;
            auto result = stream->write((const byte_t*)buffer + done, size - done, &count);
            if(count > size - done) return E_IO_ERROR;
            done += count;
            if(written) *written = done;
            if(failed(result)) return result;
            if(!count) return E_IO_ERROR;
        }
        return ok;
    }

    RV MemoryStream::read(void* buffer, usize size, usize* read_bytes)
    {
        if(read_bytes) *read_bytes = 0;
        if(size && !buffer) return E_BAD_ARGUMENTS;
        usize amount = m_position >= m_bytes.size() ? 0 : (usize)min<u64>(size, m_bytes.size() - m_position);
        if(amount) memcpy(buffer, m_bytes.data() + (usize)m_position, amount);
        m_position += amount;
        if(read_bytes) *read_bytes = amount;
        return ok;
    }

    RV MemoryStream::write(const void* buffer, usize size, usize* write_bytes)
    {
        if(write_bytes) *write_bytes = 0;
        if(size && !buffer) return E_BAD_ARGUMENTS;
        if(!size) return ok;
        if(size > (u64)I64_MAX - m_position) return E_OUT_OF_RANGE;
        if(m_position + size > m_bytes.size())
        {
            auto result = set_size(m_position + size);
            if(failed(result)) return result;
        }
        memcpy(m_bytes.data() + (usize)m_position, buffer, size);
        m_position += size;
        if(write_bytes) *write_bytes = size;
        return ok;
    }

    RV MemoryStream::seek(i64 offset, SeekMode mode)
    {
        auto result = seek_position(m_position, m_bytes.size(), offset, mode);
        if(failed(result)) return result.errcode();
        m_position = result.get();
        return ok;
    }

    RV MemoryStream::set_size(u64 size)
    {
        if(size > m_limit || size > I64_MAX || size > USIZE_MAX) return E_OUT_OF_RANGE;
        usize old_size = m_bytes.size();
        if(size > m_bytes.capacity())
        {
            u64 capacity = min<u64>(m_limit, max<u64>(size, (u64)m_bytes.capacity() * 2));
            m_bytes.reserve((usize)capacity);
        }
        m_bytes.resize((usize)size);
        if(size > old_size) memzero(m_bytes.data() + old_size, (usize)size - old_size);
        return ok;
    }

    PakFile::~PakFile()
    {
        if(!m_owner || !m_node) return;
        MutexGuard guard(m_owner->m_mutex);
        m_reader = nullptr;
        if(test_flags(m_flags, FileOpenFlag::write)) m_node->writer = false;
        else --m_node->readers;
        --m_owner->m_handles;
    }

    RV PakFile::prepare_reader()
    {
        if(!m_reader || m_decode_position > m_position)
        {
            auto opened = m_owner->m_archive->open_entry(m_node->index);
            if(failed(opened)) return opened.errcode();
            m_reader = move(opened.get());
            m_decode_position = 0;
        }
        byte_t buffer[65536];
        while(m_decode_position < m_position)
        {
            usize count = 0;
            usize amount = (usize)min<u64>(sizeof(buffer), m_position - m_decode_position);
            auto result = m_reader->read(buffer, amount, &count);
            m_decode_position += count;
            if(failed(result) || !count || count > amount)
            {
                m_reader = nullptr;
                return failed(result) ? result : RV(E_BAD_DATA);
            }
        }
        return ok;
    }

    RV PakFile::read(void* buffer, usize size, usize* read_bytes)
    {
        MutexGuard guard(m_owner->m_mutex);
        if(read_bytes) *read_bytes = 0;
        if(!test_flags(m_flags, FileOpenFlag::read)) return E_ACCESS_DENIED;
        if(size && !buffer) return E_BAD_ARGUMENTS;
        if(!size) return ok;
        lutry
        {
            if(m_position < m_node->size)
            {
                Ref<IStream> input;
                if(m_node->data)
                {
                    luexp(m_node->data->seek((i64)m_position, SeekMode::begin));
                    input = m_node->data;
                }
                else
                {
                    luexp(prepare_reader());
                    input = m_reader;
                }
                usize remaining = (usize)min<u64>(size, m_node->size - m_position);
                usize done = 0;
                while(remaining)
                {
                    usize count = 0;
                    auto result = input->read((byte_t*)buffer + done, remaining, &count);
                    if(count > remaining) return E_IO_ERROR;
                    done += count;
                    remaining -= count;
                    m_position += count;
                    if(!m_node->data) m_decode_position += count;
                    if(read_bytes) *read_bytes = done;
                    if(failed(result))
                    {
                        m_reader = nullptr;
                        return result;
                    }
                    if(!count) return E_BAD_DATA;
                }
            }
            // A sequential read reaching EOF validates CRC now, even if the caller
            // requested exactly the file length. Seeking directly past data does not.
            if(!m_node->data && m_position == m_node->size &&
                (!m_node->size || (m_reader && m_decode_position == m_node->size)))
            {
                if(!m_reader) luexp(prepare_reader());
                byte_t extra;
                usize count = 0;
                auto result = m_reader->read(&extra, 1, &count);
                if(failed(result))
                {
                    m_reader = nullptr;
                    return result;
                }
                if(count) return E_BAD_DATA;
            }
        }
        lucatchret;
        return ok;
    }

    RV PakFile::resize(u64 size)
    {
        if(size > I64_MAX || (!m_owner->m_options.create_staging_stream &&
            (size > m_owner->m_options.max_memory_file_size || size > USIZE_MAX))) return E_OUT_OF_RANGE;
        if(size == m_node->size) return ok;
        if(size < m_node->size || !m_owner->m_options.create_staging_stream)
        {
            auto result = m_node->data->set_size(size);
            u64 new_size = m_node->data->get_size();
            if(new_size != m_node->size)
            {
                m_owner->m_dirty = true;
                m_node->data_changed = true;
                m_node->size = new_size;
            }
            return result;
        }
        auto seek = m_node->data->seek((i64)m_node->size, SeekMode::begin);
        if(failed(seek)) return seek;
        byte_t zeros[65536]{};
        m_owner->m_dirty = true;
        m_node->data_changed = true;
        while(m_node->size < size)
        {
            usize amount = (usize)min<u64>(sizeof(zeros), size - m_node->size);
            u64 previous_size = m_node->size;
            auto result = write_all(m_node->data, zeros, amount);
            m_node->size = m_node->data->get_size();
            if(failed(result)) return result;
            if(m_node->size != previous_size + amount) return E_IO_ERROR;
        }
        return ok;
    }

    RV PakFile::write(const void* buffer, usize size, usize* write_bytes)
    {
        MutexGuard guard(m_owner->m_mutex);
        if(write_bytes) *write_bytes = 0;
        if(!test_flags(m_flags, FileOpenFlag::write)) return E_ACCESS_DENIED;
        if(size && !buffer) return E_BAD_ARGUMENTS;
        if(!size) return ok;
        if(size > (u64)I64_MAX - m_position) return E_OUT_OF_RANGE;
        if(!m_owner->m_options.create_staging_stream &&
            m_position + size > min<u64>(m_owner->m_options.max_memory_file_size, USIZE_MAX)) return E_OUT_OF_RANGE;
        lutry
        {
            if(m_position > m_node->size) luexp(resize(m_position));
            luexp(m_node->data->seek((i64)m_position, SeekMode::begin));
            m_owner->m_dirty = true;
            m_node->data_changed = true;
            usize count = 0;
            auto result = write_all(m_node->data, buffer, size, &count);
            m_position += count;
            m_node->size = m_node->data->get_size();
            if(write_bytes) *write_bytes = count;
            return result;
        }
        lucatchret;
        return ok;
    }

    R<u64> PakFile::tell() { MutexGuard guard(m_owner->m_mutex); return m_position; }
    u64 PakFile::get_size() { MutexGuard guard(m_owner->m_mutex); return m_node->size; }

    RV PakFile::seek(i64 offset, SeekMode mode)
    {
        MutexGuard guard(m_owner->m_mutex);
        auto result = seek_position(m_position, m_node->size, offset, mode);
        if(failed(result)) return result.errcode();
        m_position = result.get();
        return ok;
    }

    RV PakFile::set_size(u64 size)
    {
        MutexGuard guard(m_owner->m_mutex);
        if(!test_flags(m_flags, FileOpenFlag::write)) return E_ACCESS_DENIED;
        return resize(size);
    }

    void PakFile::flush()
    {
        MutexGuard guard(m_owner->m_mutex);
        if(m_node->data)
        {
            auto file = m_node->data.as<IFile>();
            if(file) file->flush();
        }
    }
}
