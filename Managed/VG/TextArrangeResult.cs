using System;
using Luna.VG.Internal;

namespace Luna.VG;

public sealed class TextArrangeResult : IDisposable
{
    private readonly NativeTextArrangeResultHandle _handle;

    internal TextArrangeResult(NativeTextArrangeResultHandle handle, RectF boundingRect, bool overflow, TextLineArrangeResult[] lines)
    {
        _handle = handle;
        BoundingRect = boundingRect;
        Overflow = overflow;
        Lines = lines;
    }

    public RectF BoundingRect { get; }
    public bool Overflow { get; }
    public TextLineArrangeResult[] Lines { get; }
    public bool IsDisposed => _handle.IsClosed || _handle.IsInvalid;

    internal IntPtr GetNativeHandle()
    {
        if (IsDisposed)
        {
            throw new ObjectDisposedException(nameof(TextArrangeResult));
        }
        return _handle.DangerousGetHandle();
    }

    public void Dispose()
    {
        _handle.Dispose();
        GC.SuppressFinalize(this);
    }
}
