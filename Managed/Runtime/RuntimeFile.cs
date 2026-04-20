using System;
using System.Runtime.InteropServices;
using Luna.Runtime.Internal;

namespace Luna.Runtime;

public static class RuntimeFile
{
    public static IFile Open(string path, FileOpenFlags flags, FileCreationMode creation)
    {
        ArgumentException.ThrowIfNullOrEmpty(path);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RuntimeNative.FileOpen(path, (uint)flags, (uint)creation, out var file)));
        return new NativeFile(file, retain: false);
    }

    public static byte[] LoadData(string path)
    {
        using var file = Open(path, FileOpenFlags.Read | FileOpenFlags.UserBuffering, FileCreationMode.OpenExisting);
        return LoadData(file);
    }

    public static byte[] LoadData(IFile file)
    {
        ArgumentNullException.ThrowIfNull(file);
        var nativeFile = file as NativeFile
            ?? throw new ArgumentException("The file was not created by the Luna.Runtime binding.", nameof(file));

        var data = IntPtr.Zero;
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RuntimeNative.FileLoadData(nativeFile.NativeFilePointer, out data, out var size)));
        try
        {
            if (size > int.MaxValue)
            {
                throw new InvalidOperationException("The file is too large to load into a managed byte array.");
            }
            var result = new byte[(int)size];
            if (size > 0)
            {
                Marshal.Copy(data, result, 0, result.Length);
            }
            return result;
        }
        finally
        {
            RuntimeNative.FreeBuffer(data);
        }
    }

    public static FileAttribute GetAttribute(string path)
    {
        ArgumentException.ThrowIfNullOrEmpty(path);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RuntimeNative.FileGetAttribute(path, out var attribute)));
        return attribute;
    }

    public static void Copy(string fromPath, string toPath)
    {
        ArgumentException.ThrowIfNullOrEmpty(fromPath);
        ArgumentException.ThrowIfNullOrEmpty(toPath);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RuntimeNative.FileCopy(fromPath, toPath)));
    }

    public static void Move(string fromPath, string toPath)
    {
        ArgumentException.ThrowIfNullOrEmpty(fromPath);
        ArgumentException.ThrowIfNullOrEmpty(toPath);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RuntimeNative.FileMove(fromPath, toPath)));
    }

    public static void Delete(string path)
    {
        ArgumentException.ThrowIfNullOrEmpty(path);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RuntimeNative.FileDelete(path)));
    }

    public static void CreateDirectory(string path)
    {
        ArgumentException.ThrowIfNullOrEmpty(path);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RuntimeNative.FileCreateDirectory(path)));
    }

    public static IFileIterator OpenDirectory(string path)
    {
        ArgumentException.ThrowIfNullOrEmpty(path);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RuntimeNative.FileOpenDirectory(path, out var iterator)));
        return new NativeFileIterator(iterator, retain: false);
    }
}
