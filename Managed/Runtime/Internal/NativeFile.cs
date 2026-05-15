using System;
using System.Runtime.InteropServices;

namespace Luna.Runtime.Internal;

internal sealed class NativeFile : ObjectBase, IFile
{
    private readonly IntPtr _ifile;
    private readonly IntPtr _iseekableStream;
    private readonly IntPtr _istream;

    internal NativeFile(NativeFileHandle handle, bool retain)
        : base(handle.Object, retain)
    {
        if (handle.Ifile == IntPtr.Zero ||
            handle.IseekableStream == IntPtr.Zero ||
            handle.Istream == IntPtr.Zero)
        {
            throw new ArgumentException("Native file handle is incomplete.", nameof(handle));
        }
        _ifile = handle.Ifile;
        _iseekableStream = handle.IseekableStream;
        _istream = handle.Istream;
    }

    internal IntPtr NativeFilePointer => _ifile;

    public ulong Position
    {
        get
        {
            EnsureNotDisposed();
            RuntimeErrors.ThrowIfFailed(new ErrorCode(RuntimeNativeGenerated.SeekableStreamTell(_iseekableStream, out var position)));
            return position;
        }
    }

    public ulong Size
    {
        get
        {
            EnsureNotDisposed();
            return RuntimeNativeGenerated.SeekableStreamGetSize(_iseekableStream);
        }
        set
        {
            EnsureNotDisposed();
            RuntimeErrors.ThrowIfFailed(new ErrorCode(RuntimeNativeGenerated.SeekableStreamSetSize(_iseekableStream, value)));
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
        using var pinnedTemp = PinnedByteArray.Create(temp);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RuntimeNativeGenerated.StreamRead(_istream, pinnedTemp.Pointer, (ulong)count, out var readBytes)));
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
        using var pinnedTemp = PinnedByteArray.Create(temp);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RuntimeNativeGenerated.StreamWrite(_istream, pinnedTemp.Pointer, (ulong)count, out var writeBytes)));
        return writeBytes;
    }

    public void Seek(long offset, SeekMode mode)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RuntimeNativeGenerated.SeekableStreamSeek(_iseekableStream, offset, (uint)mode)));
    }

    public void Flush()
    {
        EnsureNotDisposed();
        RuntimeNativeGenerated.FileFlush(_ifile);
    }

    private static void ValidateRange(byte[] buffer, int offset, int count)
    {
        ArgumentNullException.ThrowIfNull(buffer);
        if (offset < 0 || count < 0 || offset > buffer.Length - count)
        {
            throw new ArgumentOutOfRangeException(nameof(offset));
        }
    }

    private readonly struct PinnedByteArray : IDisposable
    {
        private readonly GCHandle m_handle;

        public IntPtr Pointer { get; }

        private PinnedByteArray(GCHandle handle, IntPtr pointer)
        {
            m_handle = handle;
            Pointer = pointer;
        }

        public static PinnedByteArray Create(byte[] data)
        {
            if (data.Length == 0)
            {
                return new PinnedByteArray(default, IntPtr.Zero);
            }
            var handle = GCHandle.Alloc(data, GCHandleType.Pinned);
            return new PinnedByteArray(handle, handle.AddrOfPinnedObject());
        }

        public void Dispose()
        {
            if (m_handle.IsAllocated)
            {
                m_handle.Free();
            }
        }
    }
}
