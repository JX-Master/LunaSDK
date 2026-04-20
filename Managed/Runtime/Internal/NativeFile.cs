using System;

namespace Luna.Runtime.Internal;

internal sealed class NativeFile : ObjectBase, IFile
{
    private readonly IntPtr _ifile;
    private readonly IntPtr _iseekableStream;
    private readonly IntPtr _istream;

    internal NativeFile(NativeFileHandle handle, bool retain)
        : base(handle.Object, retain)
    {
        if (handle.IFile == IntPtr.Zero ||
            handle.ISeekableStream == IntPtr.Zero ||
            handle.IStream == IntPtr.Zero)
        {
            throw new ArgumentException("Native file handle is incomplete.", nameof(handle));
        }
        _ifile = handle.IFile;
        _iseekableStream = handle.ISeekableStream;
        _istream = handle.IStream;
    }

    internal IntPtr NativeFilePointer => _ifile;

    public ulong Position
    {
        get
        {
            EnsureNotDisposed();
            RuntimeErrors.ThrowIfFailed(new ErrorCode(RuntimeNative.SeekableStreamTell(_iseekableStream, out var position)));
            return position;
        }
    }

    public ulong Size
    {
        get
        {
            EnsureNotDisposed();
            return RuntimeNative.SeekableStreamGetSize(_iseekableStream);
        }
        set
        {
            EnsureNotDisposed();
            RuntimeErrors.ThrowIfFailed(new ErrorCode(RuntimeNative.SeekableStreamSetSize(_iseekableStream, value)));
        }
    }

    public ulong Read(byte[] buffer)
    {
        ArgumentNullException.ThrowIfNull(buffer);
        return Read(buffer, 0, buffer.Length);
    }

    public ulong Read(byte[] buffer, int offset, int count)
    {
        EnsureNotDisposed();
        ValidateRange(buffer, offset, count);
        if (count == 0)
        {
            return 0;
        }

        var temp = offset == 0 && count == buffer.Length ? buffer : new byte[count];
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RuntimeNative.StreamRead(_istream, temp, (ulong)count, out var readBytes)));
        if (!ReferenceEquals(temp, buffer) && readBytes > 0)
        {
            Array.Copy(temp, 0, buffer, offset, checked((int)readBytes));
        }
        return readBytes;
    }

    public ulong Write(byte[] buffer)
    {
        ArgumentNullException.ThrowIfNull(buffer);
        return Write(buffer, 0, buffer.Length);
    }

    public ulong Write(byte[] buffer, int offset, int count)
    {
        EnsureNotDisposed();
        ValidateRange(buffer, offset, count);
        if (count == 0)
        {
            return 0;
        }

        var temp = offset == 0 && count == buffer.Length ? buffer : new byte[count];
        if (!ReferenceEquals(temp, buffer))
        {
            Array.Copy(buffer, offset, temp, 0, count);
        }
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RuntimeNative.StreamWrite(_istream, temp, (ulong)count, out var writeBytes)));
        return writeBytes;
    }

    public void Seek(long offset, SeekMode mode)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RuntimeNative.SeekableStreamSeek(_iseekableStream, offset, (uint)mode)));
    }

    public void Flush()
    {
        EnsureNotDisposed();
        RuntimeNative.FileFlush(_ifile);
    }

    private static void ValidateRange(byte[] buffer, int offset, int count)
    {
        ArgumentNullException.ThrowIfNull(buffer);
        if (offset < 0 || count < 0 || offset > buffer.Length - count)
        {
            throw new ArgumentOutOfRangeException(nameof(offset));
        }
    }
}
