#include "Runtime.h"

#include <Luna/Runtime/Error.hpp>
#include <Luna/Runtime/File.hpp>
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/Runtime/Memory.hpp>
#include <Luna/Runtime/Object.hpp>
#include <Luna/Runtime/Reflection.hpp>
#include <Luna/Runtime/Ref.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Stream.hpp>
#include <Luna/Runtime/Time.hpp>

#include <cstring>

namespace
{
Luna::ErrCode to_errcode(luna_errcode_t code)
{
    return Luna::ErrCode(static_cast<Luna::usize>(code));
}

luna_errcode_t from_errcode(Luna::ErrCode code)
{
    return static_cast<luna_errcode_t>(code.code);
}

luna_errcode_t from_result(const Luna::RV& result)
{
    return from_errcode(result.errcode());
}

Luna::Guid to_guid(const LunaGuid& guid)
{
    return Luna::Guid(guid.high, guid.low);
}

LunaGuid from_guid(const Luna::Guid& guid)
{
    return LunaGuid{guid.high, guid.low};
}

Luna::typeinfo_t to_type(luna_type_t type)
{
    return Luna::typeinfo_t(type);
}

luna_type_t from_type(Luna::typeinfo_t type)
{
    return type.handle;
}

LunaDateTime from_datetime(const Luna::DateTime& datetime)
{
    return LunaDateTime
    {
        datetime.year,
        datetime.month,
        datetime.day,
        datetime.hour,
        datetime.minute,
        datetime.second,
        datetime.day_of_week
    };
}

Luna::DateTime to_datetime(const LunaDateTime& datetime)
{
    return Luna::DateTime
    {
        datetime.year,
        datetime.month,
        datetime.day,
        datetime.hour,
        datetime.minute,
        datetime.second,
        datetime.day_of_week
    };
}

Luna::LogVerbosity to_log_verbosity(uint32_t verbosity)
{
    if (verbosity > static_cast<uint32_t>(Luna::LogVerbosity::verbose))
    {
        verbosity = static_cast<uint32_t>(Luna::LogVerbosity::verbose);
    }
    return static_cast<Luna::LogVerbosity>(verbosity);
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

Luna::IStream* as_stream(void* self)
{
    return reinterpret_cast<Luna::IStream*>(self);
}

Luna::ISeekableStream* as_seekable_stream(void* self)
{
    return reinterpret_cast<Luna::ISeekableStream*>(self);
}

Luna::IFile* as_file(void* self)
{
    return reinterpret_cast<Luna::IFile*>(self);
}

Luna::IFileIterator* as_file_iterator(void* self)
{
    return reinterpret_cast<Luna::IFileIterator*>(self);
}

template <typename T>
T* object_as(luna_handle_t object)
{
    return object ? Luna::query_interface<T>(object) : nullptr;
}
}

extern "C"
{
LUNA_RUNTIME_C_API int32_t luna_runtime_init(void)
{
    return Luna::init() ? 1 : 0;
}

LUNA_RUNTIME_C_API int32_t luna_runtime_is_initialized(void)
{
    return Luna::is_initialized() ? 1 : 0;
}

LUNA_RUNTIME_C_API void luna_runtime_close(void)
{
    Luna::close();
}

LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_error_get_code_by_name(const char* category_name, const char* code_name)
{
    return from_errcode(Luna::get_error_code_by_name(category_name, code_name));
}

LUNA_RUNTIME_C_API luna_errcat_t luna_runtime_error_get_category_by_name(const char* category_name)
{
    return static_cast<luna_errcat_t>(Luna::get_error_category_by_name(category_name));
}

LUNA_RUNTIME_C_API const char* luna_runtime_error_get_code_name(luna_errcode_t code)
{
    return Luna::get_error_code_name(to_errcode(code));
}

LUNA_RUNTIME_C_API const char* luna_runtime_error_get_category_name(luna_errcat_t category)
{
    return Luna::get_error_category_name(static_cast<Luna::errcat_t>(category));
}

LUNA_RUNTIME_C_API luna_errcat_t luna_runtime_error_get_code_category(luna_errcode_t code)
{
    return static_cast<luna_errcat_t>(Luna::get_error_code_category(to_errcode(code)));
}

LUNA_RUNTIME_C_API const char* luna_runtime_error_explain(luna_errcode_t code)
{
    return Luna::explain(to_errcode(code));
}

LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_error_unwrap(luna_errcode_t code)
{
    return from_errcode(Luna::unwrap_errcode(to_errcode(code)));
}

LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_error_get_current_code(void)
{
    return from_errcode(Luna::get_error().code);
}

LUNA_RUNTIME_C_API const char* luna_runtime_error_get_current_message(void)
{
    return Luna::get_error().explain();
}

LUNA_RUNTIME_C_API int32_t luna_runtime_object_retain(luna_handle_t object)
{
    return object ? Luna::object_retain(object) : 0;
}

LUNA_RUNTIME_C_API int32_t luna_runtime_object_release(luna_handle_t object)
{
    return object ? Luna::object_release(object) : 0;
}

LUNA_RUNTIME_C_API int32_t luna_runtime_object_ref_count(luna_handle_t object)
{
    return object ? Luna::object_ref_count(object) : 0;
}

LUNA_RUNTIME_C_API int32_t luna_runtime_object_retain_weak(luna_handle_t object)
{
    return object ? Luna::object_retain_weak(object) : 0;
}

LUNA_RUNTIME_C_API int32_t luna_runtime_object_release_weak(luna_handle_t object)
{
    return object ? Luna::object_release_weak(object) : 0;
}

LUNA_RUNTIME_C_API int32_t luna_runtime_object_weak_ref_count(luna_handle_t object)
{
    return object ? Luna::object_weak_ref_count(object) : 0;
}

LUNA_RUNTIME_C_API int32_t luna_runtime_object_expired(luna_handle_t object)
{
    return object ? (Luna::object_expired(object) ? 1 : 0) : 1;
}

LUNA_RUNTIME_C_API int32_t luna_runtime_object_retain_if_not_expired(luna_handle_t object)
{
    return object && Luna::object_retain_if_not_expired(object) ? 1 : 0;
}

LUNA_RUNTIME_C_API void* luna_runtime_object_query_interface(luna_handle_t object, const LunaGuid* iid)
{
    if (!object || !iid)
    {
        return nullptr;
    }
    Luna::Guid guid = to_guid(*iid);
    return Luna::query_interface(object, guid);
}

LUNA_RUNTIME_C_API luna_type_t luna_runtime_type_get_by_guid(const LunaGuid* guid)
{
    return guid ? from_type(Luna::get_type_by_guid(to_guid(*guid))) : nullptr;
}

LUNA_RUNTIME_C_API luna_type_t luna_runtime_type_get_object_type(luna_handle_t object)
{
    return object ? from_type(Luna::get_object_type(object)) : nullptr;
}

LUNA_RUNTIME_C_API luna_type_t luna_runtime_type_get_base(luna_type_t type)
{
    return type ? from_type(Luna::get_base_type(to_type(type))) : nullptr;
}

LUNA_RUNTIME_C_API void luna_runtime_type_get_guid(luna_type_t type, LunaGuid* out_guid)
{
    if (!out_guid)
    {
        return;
    }
    *out_guid = type ? from_guid(Luna::get_type_guid(to_type(type))) : LunaGuid{0, 0};
}

LUNA_RUNTIME_C_API const char* luna_runtime_type_get_name(luna_type_t type)
{
    return type ? Luna::get_type_name(to_type(type)).c_str() : "";
}

LUNA_RUNTIME_C_API const char* luna_runtime_type_get_alias(luna_type_t type)
{
    if (!type)
    {
        return "";
    }
    Luna::Name alias;
    Luna::get_type_name(to_type(type), &alias);
    return alias.c_str();
}

LUNA_RUNTIME_C_API uint64_t luna_runtime_type_get_size(luna_type_t type)
{
    return type ? static_cast<uint64_t>(Luna::get_type_size(to_type(type))) : 0;
}

LUNA_RUNTIME_C_API uint64_t luna_runtime_type_get_alignment(luna_type_t type)
{
    return type ? static_cast<uint64_t>(Luna::get_type_alignment(to_type(type))) : 0;
}

LUNA_RUNTIME_C_API int32_t luna_runtime_type_is_type(luna_type_t type, luna_type_t target_type)
{
    if (!type || !target_type)
    {
        return 0;
    }
    Luna::typeinfo_t current = to_type(type);
    Luna::typeinfo_t target = to_type(target_type);
    while (current)
    {
        if (current == target)
        {
            return 1;
        }
        current = Luna::get_base_type(current);
    }
    return 0;
}

LUNA_RUNTIME_C_API int32_t luna_runtime_object_is_type(luna_handle_t object, luna_type_t target_type)
{
    return object && target_type && Luna::object_is_type(object, to_type(target_type)) ? 1 : 0;
}

LUNA_RUNTIME_C_API uint64_t luna_runtime_time_get_ticks(void)
{
    return static_cast<uint64_t>(Luna::get_ticks());
}

LUNA_RUNTIME_C_API double luna_runtime_time_get_ticks_per_second(void)
{
    return Luna::get_ticks_per_second();
}

LUNA_RUNTIME_C_API int64_t luna_runtime_time_get_utc_timestamp(void)
{
    return static_cast<int64_t>(Luna::get_utc_timestamp());
}

LUNA_RUNTIME_C_API int64_t luna_runtime_time_get_local_timestamp(void)
{
    return static_cast<int64_t>(Luna::get_local_timestamp());
}

LUNA_RUNTIME_C_API int64_t luna_runtime_time_local_to_utc(int64_t local_timestamp)
{
    return static_cast<int64_t>(Luna::local_timestamp_to_utc_timestamp(static_cast<Luna::i64>(local_timestamp)));
}

LUNA_RUNTIME_C_API int64_t luna_runtime_time_utc_to_local(int64_t utc_timestamp)
{
    return static_cast<int64_t>(Luna::utc_timestamp_to_local_timestamp(static_cast<Luna::i64>(utc_timestamp)));
}

LUNA_RUNTIME_C_API void luna_runtime_time_timestamp_to_datetime(int64_t timestamp, LunaDateTime* out_datetime)
{
    if (!out_datetime)
    {
        return;
    }
    *out_datetime = from_datetime(Luna::timestamp_to_datetime(static_cast<Luna::i64>(timestamp)));
}

LUNA_RUNTIME_C_API int64_t luna_runtime_time_datetime_to_timestamp(const LunaDateTime* datetime)
{
    return datetime ? static_cast<int64_t>(Luna::datetime_to_timestamp(to_datetime(*datetime))) : 0;
}

LUNA_RUNTIME_C_API void luna_runtime_log(uint32_t verbosity, const char* tag, const char* message)
{
    tag = tag ? tag : "";
    message = message ? message : "";
    Luna::log_unformatted(
        to_log_verbosity(verbosity),
        tag,
        std::strlen(tag),
        message,
        std::strlen(message));
}

LUNA_RUNTIME_C_API void luna_runtime_log_set_platform_enabled(int32_t enabled)
{
    Luna::set_log_to_platform_enabled(enabled != 0);
}

LUNA_RUNTIME_C_API void luna_runtime_log_set_platform_verbosity(uint32_t verbosity)
{
    Luna::set_log_to_platform_verbosity(to_log_verbosity(verbosity));
}

LUNA_RUNTIME_C_API void luna_runtime_log_set_file_enabled(int32_t enabled)
{
    Luna::set_log_to_file_enabled(enabled != 0);
}

LUNA_RUNTIME_C_API void luna_runtime_log_set_file(const char* file)
{
    Luna::set_log_file(file ? file : "");
}

LUNA_RUNTIME_C_API void luna_runtime_log_set_file_verbosity(uint32_t verbosity)
{
    Luna::set_log_to_file_verbosity(to_log_verbosity(verbosity));
}

LUNA_RUNTIME_C_API void luna_runtime_log_flush_file(void)
{
    Luna::flush_log_to_file();
}

LUNA_RUNTIME_C_API const char* luna_runtime_get_current_dir(void)
{
    return Luna::get_current_dir();
}

LUNA_RUNTIME_C_API void luna_runtime_release_current_dir(const char* path)
{
    if (path)
    {
        Luna::release_current_dir(path);
    }
}

LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_set_current_dir(const char* path)
{
    return from_result(Luna::set_current_dir(path ? path : ""));
}

LUNA_RUNTIME_C_API const char* luna_runtime_get_process_path(void)
{
    return Luna::get_process_path();
}

LUNA_RUNTIME_C_API void luna_runtime_release_process_path(const char* path)
{
    if (path)
    {
        Luna::release_process_path(path);
    }
}

LUNA_RUNTIME_C_API void luna_runtime_free_buffer(void* buffer)
{
    Luna::memfree(buffer);
}

LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_file_open(const char* path, uint32_t flags, uint32_t creation, LunaFileHandle* out_file)
{
    if (!path || !out_file)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_file->object = nullptr;
    out_file->ifile = nullptr;
    out_file->iseekable_stream = nullptr;
    out_file->istream = nullptr;

    auto result = Luna::open_file(
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

LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_file_load_data(void* self, void** out_data, uint64_t* out_size)
{
    if (!self || !out_data || !out_size)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    *out_data = nullptr;
    *out_size = 0;
    auto result = Luna::load_file_data(as_file(self));
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }

    Luna::Blob data = Luna::move(result.get());
    *out_size = static_cast<uint64_t>(data.size());
    *out_data = data.detach();
    return 0;
}

LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_file_get_attribute(const char* path, LunaFileAttribute* out_attribute)
{
    if (!path || !out_attribute)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    auto result = Luna::get_file_attribute(path);
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }
    *out_attribute = from_file_attribute(result.get());
    return 0;
}

LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_file_copy(const char* from_path, const char* to_path)
{
    if (!from_path || !to_path)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::copy_file(from_path, to_path));
}

LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_file_move(const char* from_path, const char* to_path)
{
    if (!from_path || !to_path)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::move_file(from_path, to_path));
}

LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_file_delete(const char* path)
{
    if (!path)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::delete_file(path));
}

LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_file_create_dir(const char* path)
{
    if (!path)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::create_dir(path));
}

LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_file_open_dir(const char* path, LunaFileIteratorHandle* out_iterator)
{
    if (!path || !out_iterator)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_iterator->object = nullptr;
    out_iterator->ifile_iterator = nullptr;

    auto result = Luna::open_dir(path);
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

LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_stream_read(void* self, void* buffer, uint64_t size, uint64_t* out_read_bytes)
{
    if (!self || (!buffer && size))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::usize read_bytes = 0;
    auto result = as_stream(self)->read(buffer, static_cast<Luna::usize>(size), &read_bytes);
    if (out_read_bytes)
    {
        *out_read_bytes = static_cast<uint64_t>(read_bytes);
    }
    return from_result(result);
}

LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_stream_write(void* self, const void* buffer, uint64_t size, uint64_t* out_write_bytes)
{
    if (!self || (!buffer && size))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::usize write_bytes = 0;
    auto result = as_stream(self)->write(buffer, static_cast<Luna::usize>(size), &write_bytes);
    if (out_write_bytes)
    {
        *out_write_bytes = static_cast<uint64_t>(write_bytes);
    }
    return from_result(result);
}

LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_seekable_stream_tell(void* self, uint64_t* out_position)
{
    if (!self || !out_position)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    auto result = as_seekable_stream(self)->tell();
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }
    *out_position = static_cast<uint64_t>(result.get());
    return 0;
}

LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_seekable_stream_seek(void* self, int64_t offset, uint32_t mode)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(as_seekable_stream(self)->seek(
        static_cast<Luna::i64>(offset),
        static_cast<Luna::SeekMode>(mode)));
}

LUNA_RUNTIME_C_API uint64_t luna_runtime_seekable_stream_get_size(void* self)
{
    return self ? static_cast<uint64_t>(as_seekable_stream(self)->get_size()) : 0;
}

LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_seekable_stream_set_size(void* self, uint64_t size)
{
    if (!self)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(as_seekable_stream(self)->set_size(static_cast<Luna::u64>(size)));
}

LUNA_RUNTIME_C_API void luna_runtime_file_flush(void* self)
{
    if (self)
    {
        as_file(self)->flush();
    }
}

LUNA_RUNTIME_C_API int32_t luna_runtime_file_iterator_is_valid(void* self)
{
    return self && as_file_iterator(self)->is_valid() ? 1 : 0;
}

LUNA_RUNTIME_C_API const char* luna_runtime_file_iterator_get_filename(void* self)
{
    return self ? as_file_iterator(self)->get_filename() : nullptr;
}

LUNA_RUNTIME_C_API uint32_t luna_runtime_file_iterator_get_attributes(void* self)
{
    return self ? static_cast<uint32_t>(as_file_iterator(self)->get_attributes()) : 0;
}

LUNA_RUNTIME_C_API int32_t luna_runtime_file_iterator_move_next(void* self)
{
    return self && as_file_iterator(self)->move_next() ? 1 : 0;
}
}
