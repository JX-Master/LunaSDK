using System;
using System.Runtime.InteropServices;

namespace Luna.Runtime.Internal;

internal sealed class NativeFileIterator : ObjectBase, IFileIterator
{
    private readonly IntPtr _ifileIterator;

    internal NativeFileIterator(NativeFileIteratorHandle handle, bool retain)
        : base(handle.Object, retain)
    {
        if (handle.IfileIterator == IntPtr.Zero)
        {
            throw new ArgumentException("Native file iterator handle is incomplete.", nameof(handle));
        }
        _ifileIterator = handle.IfileIterator;
    }

    public bool IsValid
    {
        get
        {
            EnsureNotDisposed();
            return RuntimeNativeGenerated.FileIteratorIsValid(_ifileIterator) != 0;
        }
    }

    public string? FileName
    {
        get
        {
            EnsureNotDisposed();
            var value = RuntimeNativeGenerated.FileIteratorGetFilename(_ifileIterator);
            return value == IntPtr.Zero ? null : Marshal.PtrToStringUTF8(value);
        }
    }

    public FileAttributeFlags Attributes
    {
        get
        {
            EnsureNotDisposed();
            return (FileAttributeFlags)RuntimeNativeGenerated.FileIteratorGetAttributes(_ifileIterator);
        }
    }

    public bool MoveNext()
    {
        EnsureNotDisposed();
        return RuntimeNativeGenerated.FileIteratorMoveNext(_ifileIterator) != 0;
    }
}
