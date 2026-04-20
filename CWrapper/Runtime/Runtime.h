#pragma once

#include <stdint.h>
#include <stddef.h>

#if defined(_WIN32)
#define LUNA_RUNTIME_C_API __declspec(dllexport)
#else
#define LUNA_RUNTIME_C_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef uintptr_t luna_errcode_t;
typedef uintptr_t luna_errcat_t;
typedef void* luna_type_t;
typedef void* luna_handle_t;

typedef struct LunaGuid
{
    uint64_t high;
    uint64_t low;
} LunaGuid;

typedef struct LunaDateTime
{
    int16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t day_of_week;
} LunaDateTime;

typedef struct LunaFileAttribute
{
    uint64_t size;
    int64_t creation_time;
    int64_t last_access_time;
    int64_t last_write_time;
    uint32_t attributes;
} LunaFileAttribute;

typedef struct LunaFileHandle
{
    luna_handle_t object;
    void* ifile;
    void* iseekable_stream;
    void* istream;
} LunaFileHandle;

typedef struct LunaFileIteratorHandle
{
    luna_handle_t object;
    void* ifile_iterator;
} LunaFileIteratorHandle;

LUNA_RUNTIME_C_API int32_t luna_runtime_init(void);
LUNA_RUNTIME_C_API int32_t luna_runtime_is_initialized(void);
LUNA_RUNTIME_C_API void luna_runtime_close(void);

LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_error_get_code_by_name(const char* category_name, const char* code_name);
LUNA_RUNTIME_C_API luna_errcat_t luna_runtime_error_get_category_by_name(const char* category_name);
LUNA_RUNTIME_C_API const char* luna_runtime_error_get_code_name(luna_errcode_t code);
LUNA_RUNTIME_C_API const char* luna_runtime_error_get_category_name(luna_errcat_t category);
LUNA_RUNTIME_C_API luna_errcat_t luna_runtime_error_get_code_category(luna_errcode_t code);
LUNA_RUNTIME_C_API const char* luna_runtime_error_explain(luna_errcode_t code);
LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_error_unwrap(luna_errcode_t code);
LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_error_get_current_code(void);
LUNA_RUNTIME_C_API const char* luna_runtime_error_get_current_message(void);

LUNA_RUNTIME_C_API int32_t luna_runtime_object_retain(luna_handle_t object);
LUNA_RUNTIME_C_API int32_t luna_runtime_object_release(luna_handle_t object);
LUNA_RUNTIME_C_API int32_t luna_runtime_object_ref_count(luna_handle_t object);
LUNA_RUNTIME_C_API int32_t luna_runtime_object_retain_weak(luna_handle_t object);
LUNA_RUNTIME_C_API int32_t luna_runtime_object_release_weak(luna_handle_t object);
LUNA_RUNTIME_C_API int32_t luna_runtime_object_weak_ref_count(luna_handle_t object);
LUNA_RUNTIME_C_API int32_t luna_runtime_object_expired(luna_handle_t object);
LUNA_RUNTIME_C_API int32_t luna_runtime_object_retain_if_not_expired(luna_handle_t object);
LUNA_RUNTIME_C_API void* luna_runtime_object_query_interface(luna_handle_t object, const LunaGuid* iid);

LUNA_RUNTIME_C_API luna_type_t luna_runtime_type_get_by_guid(const LunaGuid* guid);
LUNA_RUNTIME_C_API luna_type_t luna_runtime_type_get_object_type(luna_handle_t object);
LUNA_RUNTIME_C_API luna_type_t luna_runtime_type_get_base(luna_type_t type);
LUNA_RUNTIME_C_API void luna_runtime_type_get_guid(luna_type_t type, LunaGuid* out_guid);
LUNA_RUNTIME_C_API const char* luna_runtime_type_get_name(luna_type_t type);
LUNA_RUNTIME_C_API const char* luna_runtime_type_get_alias(luna_type_t type);
LUNA_RUNTIME_C_API uint64_t luna_runtime_type_get_size(luna_type_t type);
LUNA_RUNTIME_C_API uint64_t luna_runtime_type_get_alignment(luna_type_t type);
LUNA_RUNTIME_C_API int32_t luna_runtime_type_is_type(luna_type_t type, luna_type_t target_type);
LUNA_RUNTIME_C_API int32_t luna_runtime_object_is_type(luna_handle_t object, luna_type_t target_type);

LUNA_RUNTIME_C_API uint64_t luna_runtime_time_get_ticks(void);
LUNA_RUNTIME_C_API double luna_runtime_time_get_ticks_per_second(void);
LUNA_RUNTIME_C_API int64_t luna_runtime_time_get_utc_timestamp(void);
LUNA_RUNTIME_C_API int64_t luna_runtime_time_get_local_timestamp(void);
LUNA_RUNTIME_C_API int64_t luna_runtime_time_local_to_utc(int64_t local_timestamp);
LUNA_RUNTIME_C_API int64_t luna_runtime_time_utc_to_local(int64_t utc_timestamp);
LUNA_RUNTIME_C_API void luna_runtime_time_timestamp_to_datetime(int64_t timestamp, LunaDateTime* out_datetime);
LUNA_RUNTIME_C_API int64_t luna_runtime_time_datetime_to_timestamp(const LunaDateTime* datetime);

LUNA_RUNTIME_C_API void luna_runtime_log(uint32_t verbosity, const char* tag, const char* message);
LUNA_RUNTIME_C_API void luna_runtime_log_set_platform_enabled(int32_t enabled);
LUNA_RUNTIME_C_API void luna_runtime_log_set_platform_verbosity(uint32_t verbosity);
LUNA_RUNTIME_C_API void luna_runtime_log_set_file_enabled(int32_t enabled);
LUNA_RUNTIME_C_API void luna_runtime_log_set_file(const char* file);
LUNA_RUNTIME_C_API void luna_runtime_log_set_file_verbosity(uint32_t verbosity);
LUNA_RUNTIME_C_API void luna_runtime_log_flush_file(void);

LUNA_RUNTIME_C_API const char* luna_runtime_get_current_dir(void);
LUNA_RUNTIME_C_API void luna_runtime_release_current_dir(const char* path);
LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_set_current_dir(const char* path);
LUNA_RUNTIME_C_API const char* luna_runtime_get_process_path(void);
LUNA_RUNTIME_C_API void luna_runtime_release_process_path(const char* path);

LUNA_RUNTIME_C_API void luna_runtime_free_buffer(void* buffer);
LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_file_open(const char* path, uint32_t flags, uint32_t creation, LunaFileHandle* out_file);
LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_file_load_data(void* self, void** out_data, uint64_t* out_size);
LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_file_get_attribute(const char* path, LunaFileAttribute* out_attribute);
LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_file_copy(const char* from_path, const char* to_path);
LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_file_move(const char* from_path, const char* to_path);
LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_file_delete(const char* path);
LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_file_create_dir(const char* path);
LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_file_open_dir(const char* path, LunaFileIteratorHandle* out_iterator);
LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_stream_read(void* self, void* buffer, uint64_t size, uint64_t* out_read_bytes);
LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_stream_write(void* self, const void* buffer, uint64_t size, uint64_t* out_write_bytes);
LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_seekable_stream_tell(void* self, uint64_t* out_position);
LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_seekable_stream_seek(void* self, int64_t offset, uint32_t mode);
LUNA_RUNTIME_C_API uint64_t luna_runtime_seekable_stream_get_size(void* self);
LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_seekable_stream_set_size(void* self, uint64_t size);
LUNA_RUNTIME_C_API void luna_runtime_file_flush(void* self);
LUNA_RUNTIME_C_API int32_t luna_runtime_file_iterator_is_valid(void* self);
LUNA_RUNTIME_C_API const char* luna_runtime_file_iterator_get_filename(void* self);
LUNA_RUNTIME_C_API uint32_t luna_runtime_file_iterator_get_attributes(void* self);
LUNA_RUNTIME_C_API int32_t luna_runtime_file_iterator_move_next(void* self);

#ifdef __cplusplus
}
#endif
