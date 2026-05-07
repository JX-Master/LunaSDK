using System;
using System.Runtime.InteropServices;
using Luna.Runtime;
using Luna.Runtime.Internal;
using Luna.VFS.Internal;

namespace Luna.VFS;

public static class VfsModule
{
    public static void Init()
    {
        if (!global::Luna.Runtime.Runtime.IsInitialized)
        {
            throw new InvalidOperationException("Luna runtime must be initialized before initializing the VFS module.");
        }
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VfsNative.InitModule()));
    }

    public static string PlatformFilesystemDriver
    {
        get
        {
            RuntimeErrors.ThrowIfFailed(new ErrorCode(VfsNative.GetPlatformFilesystemDriver(out var name)));
            return PtrToManagedAndFree(name);
        }
    }

    public static void Mount(string driver, string driverPath, string mountPath)
    {
        ArgumentException.ThrowIfNullOrEmpty(driver);
        ArgumentException.ThrowIfNullOrEmpty(driverPath);
        ArgumentException.ThrowIfNullOrEmpty(mountPath);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VfsNative.Mount(driver, driverPath, mountPath)));
    }

    public static void Unmount(string mountPath)
    {
        ArgumentException.ThrowIfNullOrEmpty(mountPath);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VfsNative.Unmount(mountPath)));
    }

    public static void Remount(string fromPath, string toPath)
    {
        ArgumentException.ThrowIfNullOrEmpty(fromPath);
        ArgumentException.ThrowIfNullOrEmpty(toPath);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VfsNative.Remount(fromPath, toPath)));
    }

    public static IFile OpenFile(string path, FileOpenFlags flags, FileCreationMode creation)
    {
        ArgumentException.ThrowIfNullOrEmpty(path);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VfsNative.OpenFile(path, (uint)flags, (uint)creation, out var file)));
        return new NativeFile(file, retain: false);
    }

    public static FileAttribute GetFileAttribute(string path)
    {
        ArgumentException.ThrowIfNullOrEmpty(path);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VfsNative.GetFileAttribute(path, out var attribute)));
        return attribute;
    }

    public static void CopyFile(string fromPath, string toPath)
    {
        ArgumentException.ThrowIfNullOrEmpty(fromPath);
        ArgumentException.ThrowIfNullOrEmpty(toPath);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VfsNative.CopyFile(fromPath, toPath)));
    }

    public static void MoveFile(string fromPath, string toPath)
    {
        ArgumentException.ThrowIfNullOrEmpty(fromPath);
        ArgumentException.ThrowIfNullOrEmpty(toPath);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VfsNative.MoveFile(fromPath, toPath)));
    }

    public static void DeleteFile(string path)
    {
        ArgumentException.ThrowIfNullOrEmpty(path);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VfsNative.DeleteFile(path)));
    }

    public static IFileIterator OpenDirectory(string path)
    {
        ArgumentException.ThrowIfNullOrEmpty(path);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VfsNative.OpenDirectory(path, out var iterator)));
        return new NativeFileIterator(iterator, retain: false);
    }

    public static void CreateDirectory(string path)
    {
        ArgumentException.ThrowIfNullOrEmpty(path);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VfsNative.CreateDirectory(path)));
    }

    public static string GetNativePath(string vfsPath)
    {
        ArgumentException.ThrowIfNullOrEmpty(vfsPath);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VfsNative.GetNativePath(vfsPath, out var path)));
        return PtrToManagedAndFree(path);
    }

    private static string PtrToManagedAndFree(IntPtr value)
    {
        if (value == IntPtr.Zero)
        {
            return string.Empty;
        }
        try
        {
            return Marshal.PtrToStringUTF8(value) ?? string.Empty;
        }
        finally
        {
            RuntimeNative.FreeBuffer(value);
        }
    }
}
