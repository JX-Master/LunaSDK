using System;
using System.Runtime.InteropServices;

namespace Luna.Runtime.Internal;

internal static class RuntimeNative
{
    private const string LibraryName = "LunaRuntimeC";

    [DllImport(LibraryName, EntryPoint = "luna_runtime_init")]
    internal static extern int Init();

    [DllImport(LibraryName, EntryPoint = "luna_runtime_is_initialized")]
    internal static extern int IsInitialized();

    [DllImport(LibraryName, EntryPoint = "luna_runtime_close")]
    internal static extern void Close();

    [DllImport(LibraryName, EntryPoint = "luna_runtime_error_get_code_by_name")]
    internal static extern UIntPtr ErrorGetCodeByName(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string categoryName,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string codeName);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_error_get_category_by_name")]
    internal static extern UIntPtr ErrorGetCategoryByName(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string categoryName);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_error_get_code_name")]
    internal static extern IntPtr ErrorGetCodeName(UIntPtr code);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_error_get_category_name")]
    internal static extern IntPtr ErrorGetCategoryName(UIntPtr category);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_error_get_code_category")]
    internal static extern UIntPtr ErrorGetCodeCategory(UIntPtr code);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_error_explain")]
    internal static extern IntPtr ErrorExplain(UIntPtr code);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_error_unwrap")]
    internal static extern UIntPtr ErrorUnwrap(UIntPtr code);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_error_get_current_code")]
    internal static extern UIntPtr ErrorGetCurrentCode();

    [DllImport(LibraryName, EntryPoint = "luna_runtime_error_get_current_message")]
    internal static extern IntPtr ErrorGetCurrentMessage();

    [DllImport(LibraryName, EntryPoint = "luna_runtime_object_retain")]
    internal static extern int ObjectRetain(IntPtr nativeObject);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_object_release")]
    internal static extern int ObjectRelease(IntPtr nativeObject);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_object_ref_count")]
    internal static extern int ObjectRefCount(IntPtr nativeObject);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_object_retain_weak")]
    internal static extern int ObjectRetainWeak(IntPtr nativeObject);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_object_release_weak")]
    internal static extern int ObjectReleaseWeak(IntPtr nativeObject);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_object_weak_ref_count")]
    internal static extern int ObjectWeakRefCount(IntPtr nativeObject);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_object_expired")]
    internal static extern int ObjectExpired(IntPtr nativeObject);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_object_retain_if_not_expired")]
    internal static extern int ObjectRetainIfNotExpired(IntPtr nativeObject);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_object_query_interface")]
    internal static extern IntPtr ObjectQueryInterface(IntPtr nativeObject, in Guid interfaceId);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_type_get_by_guid")]
    internal static extern IntPtr TypeGetByGuid(in Guid guid);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_type_get_object_type")]
    internal static extern IntPtr TypeGetObjectType(IntPtr nativeObject);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_type_get_base")]
    internal static extern IntPtr TypeGetBase(IntPtr type);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_type_get_guid")]
    internal static extern void TypeGetGuid(IntPtr type, out Guid guid);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_type_get_name")]
    internal static extern IntPtr TypeGetName(IntPtr type);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_type_get_alias")]
    internal static extern IntPtr TypeGetAlias(IntPtr type);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_type_get_size")]
    internal static extern ulong TypeGetSize(IntPtr type);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_type_get_alignment")]
    internal static extern ulong TypeGetAlignment(IntPtr type);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_type_is_type")]
    internal static extern int TypeIsType(IntPtr type, IntPtr targetType);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_object_is_type")]
    internal static extern int ObjectIsType(IntPtr nativeObject, IntPtr targetType);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_time_get_ticks")]
    internal static extern ulong TimeGetTicks();

    [DllImport(LibraryName, EntryPoint = "luna_runtime_time_get_ticks_per_second")]
    internal static extern double TimeGetTicksPerSecond();

    [DllImport(LibraryName, EntryPoint = "luna_runtime_time_get_utc_timestamp")]
    internal static extern long TimeGetUtcTimestamp();

    [DllImport(LibraryName, EntryPoint = "luna_runtime_time_get_local_timestamp")]
    internal static extern long TimeGetLocalTimestamp();

    [DllImport(LibraryName, EntryPoint = "luna_runtime_time_local_to_utc")]
    internal static extern long TimeLocalToUtc(long localTimestamp);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_time_utc_to_local")]
    internal static extern long TimeUtcToLocal(long utcTimestamp);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_time_timestamp_to_datetime")]
    internal static extern void TimeTimestampToDateTime(long timestamp, out DateTime dateTime);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_time_datetime_to_timestamp")]
    internal static extern long TimeDateTimeToTimestamp(in DateTime dateTime);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_log")]
    internal static extern void Log(
        uint verbosity,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string tag,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string message);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_log_set_platform_enabled")]
    internal static extern void LogSetPlatformEnabled(int enabled);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_log_set_platform_verbosity")]
    internal static extern void LogSetPlatformVerbosity(uint verbosity);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_log_set_file_enabled")]
    internal static extern void LogSetFileEnabled(int enabled);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_log_set_file")]
    internal static extern void LogSetFile([MarshalAs(UnmanagedType.LPUTF8Str)] string file);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_log_set_file_verbosity")]
    internal static extern void LogSetFileVerbosity(uint verbosity);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_log_flush_file")]
    internal static extern void LogFlushFile();

    [DllImport(LibraryName, EntryPoint = "luna_runtime_get_current_dir")]
    internal static extern IntPtr GetCurrentDir();

    [DllImport(LibraryName, EntryPoint = "luna_runtime_release_current_dir")]
    internal static extern void ReleaseCurrentDir(IntPtr path);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_set_current_dir")]
    internal static extern UIntPtr SetCurrentDir([MarshalAs(UnmanagedType.LPUTF8Str)] string path);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_get_process_path")]
    internal static extern IntPtr GetProcessPath();

    [DllImport(LibraryName, EntryPoint = "luna_runtime_release_process_path")]
    internal static extern void ReleaseProcessPath(IntPtr path);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_free_buffer")]
    internal static extern void FreeBuffer(IntPtr buffer);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_file_open")]
    internal static extern UIntPtr FileOpen(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string path,
        uint flags,
        uint creation,
        out NativeFileHandle outFile);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_file_load_data")]
    internal static extern UIntPtr FileLoadData(
        IntPtr self,
        out IntPtr data,
        out ulong size);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_file_get_attribute")]
    internal static extern UIntPtr FileGetAttribute(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string path,
        out FileAttribute outAttribute);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_file_copy")]
    internal static extern UIntPtr FileCopy(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string fromPath,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string toPath);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_file_move")]
    internal static extern UIntPtr FileMove(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string fromPath,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string toPath);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_file_delete")]
    internal static extern UIntPtr FileDelete([MarshalAs(UnmanagedType.LPUTF8Str)] string path);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_file_create_dir")]
    internal static extern UIntPtr FileCreateDirectory([MarshalAs(UnmanagedType.LPUTF8Str)] string path);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_file_open_dir")]
    internal static extern UIntPtr FileOpenDirectory(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string path,
        out NativeFileIteratorHandle outIterator);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_stream_read")]
    internal static extern UIntPtr StreamRead(
        IntPtr self,
        [Out] byte[] buffer,
        ulong size,
        out ulong readBytes);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_stream_write")]
    internal static extern UIntPtr StreamWrite(
        IntPtr self,
        [In] byte[] buffer,
        ulong size,
        out ulong writeBytes);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_seekable_stream_tell")]
    internal static extern UIntPtr SeekableStreamTell(IntPtr self, out ulong position);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_seekable_stream_seek")]
    internal static extern UIntPtr SeekableStreamSeek(IntPtr self, long offset, uint mode);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_seekable_stream_get_size")]
    internal static extern ulong SeekableStreamGetSize(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_seekable_stream_set_size")]
    internal static extern UIntPtr SeekableStreamSetSize(IntPtr self, ulong size);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_file_flush")]
    internal static extern void FileFlush(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_file_iterator_is_valid")]
    internal static extern int FileIteratorIsValid(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_file_iterator_get_filename")]
    internal static extern IntPtr FileIteratorGetFilename(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_file_iterator_get_attributes")]
    internal static extern uint FileIteratorGetAttributes(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_runtime_file_iterator_move_next")]
    internal static extern int FileIteratorMoveNext(IntPtr self);
}
