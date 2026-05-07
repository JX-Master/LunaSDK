using System;
using System.Runtime.InteropServices;
using Luna.Runtime;
using Luna.Runtime.Internal;

namespace Luna.VFS.Internal;

internal static class VfsNative
{
    private const string LibraryName = "LunaVFSC";

    [DllImport(LibraryName, EntryPoint = "luna_vfs_init_module")]
    internal static extern UIntPtr InitModule();

    [DllImport(LibraryName, EntryPoint = "luna_vfs_mount")]
    internal static extern UIntPtr Mount(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string driver,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string driverPath,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string mountPath);

    [DllImport(LibraryName, EntryPoint = "luna_vfs_unmount")]
    internal static extern UIntPtr Unmount([MarshalAs(UnmanagedType.LPUTF8Str)] string mountPath);

    [DllImport(LibraryName, EntryPoint = "luna_vfs_remount")]
    internal static extern UIntPtr Remount(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string fromPath,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string toPath);

    [DllImport(LibraryName, EntryPoint = "luna_vfs_open_file")]
    internal static extern UIntPtr OpenFile(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string path,
        uint flags,
        uint creation,
        out NativeFileHandle outFile);

    [DllImport(LibraryName, EntryPoint = "luna_vfs_get_file_attribute")]
    internal static extern UIntPtr GetFileAttribute(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string path,
        out FileAttribute outAttribute);

    [DllImport(LibraryName, EntryPoint = "luna_vfs_copy_file")]
    internal static extern UIntPtr CopyFile(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string fromPath,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string toPath);

    [DllImport(LibraryName, EntryPoint = "luna_vfs_move_file")]
    internal static extern UIntPtr MoveFile(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string fromPath,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string toPath);

    [DllImport(LibraryName, EntryPoint = "luna_vfs_delete_file")]
    internal static extern UIntPtr DeleteFile([MarshalAs(UnmanagedType.LPUTF8Str)] string path);

    [DllImport(LibraryName, EntryPoint = "luna_vfs_open_dir")]
    internal static extern UIntPtr OpenDirectory(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string path,
        out NativeFileIteratorHandle outIterator);

    [DllImport(LibraryName, EntryPoint = "luna_vfs_create_dir")]
    internal static extern UIntPtr CreateDirectory([MarshalAs(UnmanagedType.LPUTF8Str)] string path);

    [DllImport(LibraryName, EntryPoint = "luna_vfs_get_native_path")]
    internal static extern UIntPtr GetNativePath(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string vfsPath,
        out IntPtr outPath);

    [DllImport(LibraryName, EntryPoint = "luna_vfs_get_platform_filesystem_driver")]
    internal static extern UIntPtr GetPlatformFilesystemDriver(out IntPtr outName);
}
