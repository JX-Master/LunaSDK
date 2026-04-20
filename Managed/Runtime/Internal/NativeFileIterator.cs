using System;
using System.Runtime.InteropServices;

namespace Luna.Runtime.Internal;

internal sealed class NativeFileIterator : ObjectBase, IFileIterator
{
    private readonly IntPtr _ifileIterator;

    internal NativeFileIterator(NativeFileIteratorHandle handle, bool retain)
        : base(handle.Object, retain)
    {
        if (handle.IFileIterator == IntPtr.Zero)
        {
            throw new ArgumentException("Native file iterator handle is incomplete.", nameof(handle));
        }
        _ifileIterator = handle.IFileIterator;
    }

    public bool IsValid
    {
        get
        {
            EnsureNotDisposed();
            return RuntimeNative.FileIteratorIsValid(_ifileIterator) != 0;
        }
    }

    public string? FileName
    {
        get
        {
            EnsureNotDisposed();
            var value = RuntimeNative.FileIteratorGetFilename(_ifileIterator);
            return value == IntPtr.Zero ? null : Marshal.PtrToStringUTF8(value);
        }
    }

    public FileAttributeFlags Attributes
    {
        get
        {
            EnsureNotDisposed();
            return (FileAttributeFlags)RuntimeNative.FileIteratorGetAttributes(_ifileIterator);
        }
    }

    public bool MoveNext()
    {
        EnsureNotDisposed();
        return RuntimeNative.FileIteratorMoveNext(_ifileIterator) != 0;
    }
}
