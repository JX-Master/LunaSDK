/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Archive.cpp
* @author JXMaster
* @date 2026/9/4
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_ZIP_API LUNA_EXPORT
#include "Archive.hpp"
#include <Luna/Runtime/Memory.hpp>
#include <cerrno>
#include <cstdio>
#include <zlib.h>
#ifdef _MSC_VER
// libzip's Windows random source uses CryptoAPI. Keep this requirement with
// the object that uses libzip, including when Zip itself is a static archive.
#pragma comment(lib, "advapi32.lib")
#endif

namespace Luna::Zip
{
    static ResultCode zip_failure(zip_error_t* error)
    {
        ResultCode code = E_IO_ERROR;
        switch(zip_error_code_zip(error))
        {
        case ZIP_ER_MEMORY: code = E_OUT_OF_MEMORY; break;
        case ZIP_ER_ZLIB:
            code = zip_error_code_system(error) == Z_DATA_ERROR ? E_BAD_DATA : E_IO_ERROR;
            break;
        case ZIP_ER_NOENT:
        case ZIP_ER_DELETED: code = E_NOT_FOUND; break;
        case ZIP_ER_EXISTS: code = E_ALREADY_EXISTS; break;
        case ZIP_ER_INVAL: code = E_BAD_ARGUMENTS; break;
        case ZIP_ER_RDONLY: code = E_ACCESS_DENIED; break;
        case ZIP_ER_INUSE: code = E_BUSY; break;
        case ZIP_ER_ZIPCLOSED: code = E_BAD_CALLING_TIME; break;
        case ZIP_ER_MULTIDISK:
        case ZIP_ER_COMPNOTSUPP:
        case ZIP_ER_ENCRNOTSUPP:
        case ZIP_ER_NOPASSWD:
        case ZIP_ER_WRONGPASSWD:
        case ZIP_ER_OPNOTSUPP:
        case ZIP_ER_NOT_ALLOWED: code = E_NOT_SUPPORTED; break;
        case ZIP_ER_CRC:
        case ZIP_ER_EOF:
        case ZIP_ER_NOZIP:
        case ZIP_ER_INCONS:
        case ZIP_ER_COMPRESSED_DATA:
        case ZIP_ER_DATA_LENGTH:
        case ZIP_ER_TRUNCATED_ZIP: code = E_BAD_DATA; break;
        default: break;
        }
        return set_error(code, "Zip: %s", zip_error_strerror(error));
    }

    struct StreamSource
    {
        Ref<ISeekableStream> input;
        Ref<ISeekableStream> output;
        zip_error_t error;
        zip_source_t* native = nullptr;
        u64 size = 0;
        u64 read_position = 0;
        u64 write_position = 0;
        u64 write_size = 0;
        bool writable = false;
        bool accept_empty = false;
        bool reading = false;

        StreamSource() { zip_error_init(&error); }
        ~StreamSource() { zip_error_fini(&error); }

        zip_int64_t fail(int code)
        {
            zip_error_set(&error, code, code == ZIP_ER_READ || code == ZIP_ER_WRITE || code == ZIP_ER_SEEK ? EIO : 0);
            return -1;
        }

        zip_int64_t read(void* buffer, u64 length)
        {
            length = min(length, size - read_position);
            if(!length) return 0;
            if(!input || failed(input->seek((i64)read_position, SeekMode::begin))) return fail(ZIP_ER_SEEK);
            u64 done = 0;
            while(done < length)
            {
                usize count = 0;
                usize request = (usize)min<u64>(length - done, 1024 * 1024);
                auto result = input->read((byte_t*)buffer + (usize)done, request, &count);
                if(failed(result) || count > request) return fail(ZIP_ER_READ);
                if(!count) return fail(ZIP_ER_EOF);
                done += count;
                read_position += count;
            }
            return (zip_int64_t)done;
        }

        zip_int64_t write(const void* buffer, u64 length)
        {
            if(!output || length > ZIP_INT64_MAX - write_position) return fail(ZIP_ER_WRITE);
            if(failed(output->seek((i64)write_position, SeekMode::begin))) return fail(ZIP_ER_SEEK);
            u64 done = 0;
            while(done < length)
            {
                usize count = 0;
                usize request = (usize)min<u64>(length - done, 1024 * 1024);
                auto result = output->write((const byte_t*)buffer + (usize)done, request, &count);
                if(failed(result) || !count || count > request) return fail(ZIP_ER_WRITE);
                done += count;
                write_position += count;
                write_size = max(write_size, write_position);
            }
            return (zip_int64_t)done;
        }
    };

    static zip_int64_t stream_callback(void* userdata, void* data, zip_uint64_t length, zip_source_cmd_t command)
    {
        auto source = (StreamSource*)userdata;
        switch(command)
        {
        case ZIP_SOURCE_SUPPORTS:
            return (source->writable ? ZIP_SOURCE_SUPPORTS_WRITABLE : ZIP_SOURCE_SUPPORTS_SEEKABLE)
                | ZIP_SOURCE_MAKE_COMMAND_BITMASK(ZIP_SOURCE_ACCEPT_EMPTY)
                | ZIP_SOURCE_MAKE_COMMAND_BITMASK(ZIP_SOURCE_SUPPORTS_REOPEN);
        case ZIP_SOURCE_ACCEPT_EMPTY: return source->accept_empty ? 1 : 0;
        case ZIP_SOURCE_SUPPORTS_REOPEN: return 1;
        case ZIP_SOURCE_OPEN:
            if(source->input && source->input->get_size() != source->size) return source->fail(ZIP_ER_CHANGED);
            source->read_position = 0;
            source->reading = true;
            zip_error_set(&source->error, ZIP_ER_OK, 0);
            return 0;
        case ZIP_SOURCE_READ: return source->read(data, length);
        case ZIP_SOURCE_CLOSE: source->reading = false; return 0;
        case ZIP_SOURCE_STAT:
        {
            if(length < sizeof(zip_stat_t)) return source->fail(ZIP_ER_INVAL);
            auto stat = (zip_stat_t*)data;
            zip_stat_init(stat);
            stat->size = source->size;
            stat->comp_size = source->size;
            stat->comp_method = ZIP_CM_STORE;
            stat->encryption_method = ZIP_EM_NONE;
            stat->valid = ZIP_STAT_SIZE | ZIP_STAT_COMP_SIZE | ZIP_STAT_COMP_METHOD | ZIP_STAT_ENCRYPTION_METHOD;
            return sizeof(zip_stat_t);
        }
        case ZIP_SOURCE_ERROR: return zip_error_to_data(&source->error, data, length);
        case ZIP_SOURCE_FREE: memdelete(source); return 0;
        case ZIP_SOURCE_TELL: return (zip_int64_t)source->read_position;
        case ZIP_SOURCE_SEEK:
        {
            auto offset = zip_source_seek_compute_offset(source->read_position, source->size, data, length, &source->error);
            if(offset < 0) return -1;
            source->read_position = (u64)offset;
            return 0;
        }
        case ZIP_SOURCE_BEGIN_WRITE:
            if(!source->output) return source->fail(ZIP_ER_WRITE);
            if(failed(source->output->set_size(0))) return source->fail(ZIP_ER_WRITE);
            source->write_position = 0;
            source->write_size = 0;
            zip_error_set(&source->error, ZIP_ER_OK, 0);
            return 0;
        case ZIP_SOURCE_WRITE: return source->write(data, length);
        case ZIP_SOURCE_TELL_WRITE: return (zip_int64_t)source->write_position;
        case ZIP_SOURCE_SEEK_WRITE:
        {
            // libzip rewrites headers inside data already emitted by this source.
            auto offset = zip_source_seek_compute_offset(source->write_position, source->write_size, data, length, &source->error);
            if(offset < 0) return -1;
            source->write_position = (u64)offset;
            return 0;
        }
        case ZIP_SOURCE_COMMIT_WRITE:
            if(!source->output || failed(source->output->set_size(source->write_size))) return source->fail(ZIP_ER_WRITE);
            source->input = source->output;
            source->size = source->write_size;
            source->read_position = 0;
            source->output = nullptr;
            return 0;
        case ZIP_SOURCE_ROLLBACK_WRITE:
            // The separate destination is disposable; the original source stays intact.
            source->write_position = 0;
            source->write_size = 0;
            return 0;
        case ZIP_SOURCE_REMOVE:
            // Empty archives are always kept, so removal is never a save operation.
            return source->fail(ZIP_ER_OPNOTSUPP);
        default: return source->fail(ZIP_ER_OPNOTSUPP);
        }
    }

    static R<zip_source_t*> make_source(ISeekableStream* input, bool writable, bool create, StreamSource** context = nullptr)
    {
        if(!input && !create) return E_BAD_ARGUMENTS;
        if(input && input->get_size() > ZIP_INT64_MAX) return E_FILE_TOO_BIG;
        auto source = memnew<StreamSource>();
        source->input = input;
        source->size = input ? input->get_size() : 0;
        source->writable = writable;
        source->accept_empty = create;
        auto native = zip_source_function_create(stream_callback, source, &source->error);
        if(!native)
        {
            auto error = zip_failure(&source->error);
            memdelete(source);
            return error;
        }
        if(context) *context = source;
        source->native = native;
        return native;
    }

    static RV validate_compression(CompressionMethod method, u32 level)
    {
        if(method != CompressionMethod::store && method != CompressionMethod::deflate) return E_NOT_SUPPORTED;
        if(level > 9 || (method == CompressionMethod::store && level)) return E_BAD_ARGUMENTS;
        return ok;
    }

    Archive::~Archive()
    {
        if(m_archive) zip_discard(m_archive);
    }

    RV Archive::init(ISeekableStream* input, OpenMode mode, bool create)
    {
        if(mode != OpenMode::read && mode != OpenMode::read_write) return E_BAD_ARGUMENTS;
        m_writable = mode == OpenMode::read_write;
        auto source = make_source(input, m_writable, create, &m_source);
        if(failed(source)) return source.errcode();
        zip_error_t error;
        zip_error_init(&error);
        m_archive = zip_open_from_source(source.get(), create ? ZIP_CREATE | ZIP_TRUNCATE : (m_writable ? 0 : ZIP_RDONLY), &error);
        if(!m_archive)
        {
            auto result = zip_failure(&error);
            zip_error_fini(&error);
            zip_source_free(source.get());
            m_source = nullptr;
            return result;
        }
        zip_error_fini(&error);
        m_content_changed = create;
        if(m_writable && zip_set_archive_flag(m_archive, ZIP_AFL_CREATE_OR_KEEP_FILE_FOR_EMPTY_ARCHIVE, 1) < 0)
            return zip_failure(zip_get_error(m_archive));
        return ok;
    }

    RV Archive::check_edit()
    {
        if(!m_archive) return E_BAD_CALLING_TIME;
        if(!m_writable) return E_ACCESS_DENIED;
        if(m_readers) return E_BUSY;
        return ok;
    }

    bool Archive::is_open() { return m_archive != nullptr; }

    R<EntryInfo> Archive::get_entry(u64 index)
    {
        if(!m_archive) return E_BAD_CALLING_TIME;
        if(index >= (u64)zip_get_num_entries(m_archive, 0)) return E_NOT_FOUND;
        zip_stat_t stat;
        if(zip_stat_index(m_archive, index, 0, &stat) < 0) return zip_failure(zip_get_error(m_archive));
        EntryInfo result;
        result.index = index;
        result.name = stat.name;
        result.size = stat.size;
        result.has_compressed_size = (stat.valid & ZIP_STAT_COMP_SIZE) != 0;
        result.compressed_size = result.has_compressed_size ? stat.comp_size : 0;
        result.has_crc32 = (stat.valid & ZIP_STAT_CRC) != 0;
        result.crc32 = result.has_crc32 ? stat.crc : 0;
        result.compression = (CompressionMethod)stat.comp_method;
        result.directory = !result.name.empty() && result.name.back() == '/';
        result.encrypted = stat.encryption_method != ZIP_EM_NONE;
        return result;
    }

    R<Vector<EntryInfo>> Archive::get_entries()
    {
        if(!m_archive) return E_BAD_CALLING_TIME;
        Vector<EntryInfo> result;
        auto count = zip_get_num_entries(m_archive, 0);
        if(count < 0) return zip_failure(zip_get_error(m_archive));
        for(u64 i = 0; i < (u64)count; ++i)
        {
            // Avoid constructing an error object for ordinary tombstones.
            if(!zip_get_name(m_archive, i, 0))
            {
                if(zip_error_code_zip(zip_get_error(m_archive)) == ZIP_ER_DELETED) continue;
                return zip_failure(zip_get_error(m_archive));
            }
            auto entry = get_entry(i);
            if(failed(entry)) return entry.errcode();
            result.push_back(move(entry.get()));
        }
        return result;
    }

    R<u64> Archive::find_entry(const c8* name)
    {
        if(!m_archive) return E_BAD_CALLING_TIME;
        if(!name || !name[0]) return E_BAD_ARGUMENTS;
        auto index = zip_name_locate(m_archive, name, ZIP_FL_ENC_UTF_8);
        if(index < 0) return zip_failure(zip_get_error(m_archive));
        return (u64)index;
    }

    R<Ref<IStream>> Archive::open_entry(u64 index)
    {
        auto entry = get_entry(index);
        if(failed(entry)) return entry.errcode();
        if(entry.get().directory) return E_IS_DIRECTORY;
        if(entry.get().encrypted) return E_NOT_SUPPORTED;
        auto compression = validate_compression(entry.get().compression, 0);
        if(failed(compression)) return compression.errcode();
        auto reader = new_object<EntryStream>();
        auto staged = m_staged_entries.find(index);
        if(staged != m_staged_entries.end())
        {
            // libzip's changed-entry reader uses the destination compression method
            // in zip_stat_index, although this source still contains raw bytes.
            reader->m_staged_source = memnew<StreamSource>();
            reader->m_staged_source->input = staged->second;
            reader->m_staged_source->size = entry.get().size;
        }
        else
        {
            reader->m_file = zip_fopen_index(m_archive, index, 0);
            if(!reader->m_file) return zip_failure(zip_get_error(m_archive));
        }
        reader->m_owner = this;
        ++m_readers;
        return Ref<IStream>(reader);
    }

    R<u64> Archive::add_file(const c8* name, ISeekableStream* input, CompressionMethod method, u32 level)
    {
        auto editable = check_edit();
        if(failed(editable)) return editable.errcode();
        if(!name || !name[0] || name[strlen(name) - 1] == '/') return E_BAD_ARGUMENTS;
        auto compression = validate_compression(method, level);
        if(failed(compression)) return compression.errcode();
        auto source = make_source(input, false, false);
        if(failed(source)) return source.errcode();
        auto index = zip_file_add(m_archive, name, source.get(), ZIP_FL_ENC_UTF_8);
        if(index < 0)
        {
            zip_source_free(source.get());
            return zip_failure(zip_get_error(m_archive));
        }
        m_content_changed = true;
        if(zip_set_file_compression(m_archive, (u64)index, (zip_int32_t)method, level) < 0)
        {
            auto error = zip_failure(zip_get_error(m_archive));
            zip_delete(m_archive, (u64)index);
            return error;
        }
        m_entry_sources.push_back(input);
        m_staged_entries.insert_or_assign((u64)index, Ref<ISeekableStream>(input));
        return (u64)index;
    }

    R<u64> Archive::add_directory(const c8* name)
    {
        auto editable = check_edit();
        if(failed(editable)) return editable.errcode();
        if(!name || !name[0]) return E_BAD_ARGUMENTS;
        auto index = zip_dir_add(m_archive, name, ZIP_FL_ENC_UTF_8);
        if(index < 0) return zip_failure(zip_get_error(m_archive));
        m_content_changed = true;
        return (u64)index;
    }

    RV Archive::replace_file(u64 index, ISeekableStream* input)
    {
        auto editable = check_edit();
        if(failed(editable)) return editable.errcode();
        auto entry = get_entry(index);
        if(failed(entry)) return entry.errcode();
        if(entry.get().directory) return E_IS_DIRECTORY;
        if(entry.get().encrypted) return E_NOT_SUPPORTED;
        auto compression = validate_compression(entry.get().compression, 0);
        if(failed(compression)) return compression.errcode();
        auto source = make_source(input, false, false);
        if(failed(source)) return source.errcode();
        if(zip_file_replace(m_archive, index, source.get(), 0) < 0)
        {
            zip_source_free(source.get());
            return zip_failure(zip_get_error(m_archive));
        }
        m_content_changed = true;
        m_entry_sources.push_back(input);
        m_staged_entries.insert_or_assign(index, Ref<ISeekableStream>(input));
        if(zip_set_file_compression(m_archive, index, (zip_int32_t)entry.get().compression, 0) < 0)
            return zip_failure(zip_get_error(m_archive));
        return ok;
    }

    RV Archive::rename_entry(u64 index, const c8* name)
    {
        auto editable = check_edit();
        if(failed(editable)) return editable.errcode();
        if(!name || !name[0]) return E_BAD_ARGUMENTS;
        auto entry = get_entry(index);
        if(failed(entry)) return entry.errcode();
        if((name[strlen(name) - 1] == '/') != entry.get().directory) return E_BAD_ARGUMENTS;
        if(zip_file_rename(m_archive, index, name, ZIP_FL_ENC_UTF_8) < 0) return zip_failure(zip_get_error(m_archive));
        return ok;
    }

    RV Archive::delete_entry(u64 index)
    {
        auto editable = check_edit();
        if(failed(editable)) return editable.errcode();
        auto entry = get_entry(index);
        if(failed(entry)) return entry.errcode();
        if(zip_delete(m_archive, index) < 0) return zip_failure(zip_get_error(m_archive));
        m_staged_entries.erase(index);
        m_content_changed = true;
        return ok;
    }

    RV Archive::set_compression(u64 index, CompressionMethod method, u32 level)
    {
        auto editable = check_edit();
        if(failed(editable)) return editable.errcode();
        auto compression = validate_compression(method, level);
        if(failed(compression)) return compression.errcode();
        auto entry = get_entry(index);
        if(failed(entry)) return entry.errcode();
        if(entry.get().directory) return E_IS_DIRECTORY;
        if(entry.get().encrypted) return E_NOT_SUPPORTED;
        if(zip_set_file_compression(m_archive, index, (zip_int32_t)method, level) < 0) return zip_failure(zip_get_error(m_archive));
        m_content_changed = true;
        return ok;
    }

    RV Archive::save(ISeekableStream* destination)
    {
        auto editable = check_edit();
        if(failed(editable)) return editable.errcode();
        if(!destination || m_source->input == destination) return E_BAD_ARGUMENTS;
        for(auto& source : m_entry_sources)
        {
            if(destination == source.get()) return E_BAD_ARGUMENTS;
        }
        bool changed = m_content_changed;
        if(!changed)
        {
            auto count = zip_get_num_entries(m_archive, 0);
            for(u64 i = 0; i < (u64)count; ++i)
            {
                auto current = zip_get_name(m_archive, i, 0);
                auto original = zip_get_name(m_archive, i, ZIP_FL_UNCHANGED);
                if(!current || !original) return zip_failure(zip_get_error(m_archive));
                if(strcmp(current, original)) { changed = true; break; }
            }
        }
        m_source->output = destination;
        if(!changed)
        {
            // zip_close skips unchanged archives. Copy while the handle is still live,
            // so an output failure also preserves this session for retry.
            if(stream_callback(m_source, nullptr, 0, ZIP_SOURCE_BEGIN_WRITE) < 0)
            {
                m_source->output = nullptr;
                return zip_failure(&m_source->error);
            }
            m_source->read_position = 0;
            byte_t buffer[64 * 1024];
            while(m_source->read_position < m_source->size)
            {
                auto count = m_source->read(buffer, sizeof(buffer));
                if(count < 0 || m_source->write(buffer, (u64)count) < 0)
                {
                    m_source->output = nullptr;
                    return zip_failure(&m_source->error);
                }
            }
            if(stream_callback(m_source, nullptr, 0, ZIP_SOURCE_COMMIT_WRITE) < 0)
            {
                m_source->output = nullptr;
                return zip_failure(&m_source->error);
            }
            zip_discard(m_archive);
        }
        else if(zip_close(m_archive) < 0)
        {
            m_source->output = nullptr;
            // libzip closes its input before invoking COMMIT_WRITE. Restore the
            // read session after rollback, and clear any source EOF/error state.
            if(m_source->reading) zip_source_close(m_source->native);
            if(zip_source_open(m_source->native) < 0) return zip_failure(zip_source_error(m_source->native));
            return zip_failure(zip_get_error(m_archive));
        }
        m_archive = nullptr;
        m_source = nullptr;
        m_entry_sources.clear();
        m_staged_entries.clear();
        return ok;
    }

    RV Archive::discard()
    {
        if(m_readers) return E_BUSY;
        if(m_archive) zip_discard(m_archive);
        m_archive = nullptr;
        m_source = nullptr;
        m_entry_sources.clear();
        m_staged_entries.clear();
        return ok;
    }

    EntryStream::~EntryStream()
    {
        if(m_file) zip_fclose(m_file);
        if(m_staged_source) memdelete(m_staged_source);
        if(m_owner) --m_owner->m_readers;
    }

    RV EntryStream::read(void* buffer, usize size, usize* read_bytes)
    {
        if(read_bytes) *read_bytes = 0;
        if(size && !buffer) return E_BAD_ARGUMENTS;
        if(!size) return ok;
        usize done = 0;
        while(done < size)
        {
            usize request = min<usize>(size - done, 1024 * 1024);
            auto count = m_staged_source ? m_staged_source->read((byte_t*)buffer + done, request)
                : zip_fread(m_file, (byte_t*)buffer + done, request);
            if(count < 0) return zip_failure(m_staged_source ? &m_staged_source->error : zip_file_get_error(m_file));
            if(!count) break;
            done += (usize)count;
            if(read_bytes) *read_bytes = done;
        }
        return ok;
    }

    RV EntryStream::write(const void*, usize, usize* write_bytes)
    {
        if(write_bytes) *write_bytes = 0;
        return E_ACCESS_DENIED;
    }

    LUNA_ZIP_API R<Ref<IArchive>> open_archive(ISeekableStream* source, OpenMode mode)
    {
        auto archive = new_object<Archive>();
        auto result = archive->init(source, mode, false);
        if(failed(result)) return result.errcode();
        return Ref<IArchive>(archive);
    }

    LUNA_ZIP_API R<Ref<IArchive>> new_archive()
    {
        auto archive = new_object<Archive>();
        auto result = archive->init(nullptr, OpenMode::read_write, true);
        if(failed(result)) return result.errcode();
        return Ref<IArchive>(archive);
    }
}
