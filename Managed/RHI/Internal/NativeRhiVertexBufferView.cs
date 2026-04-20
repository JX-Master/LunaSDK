using System;
using System.Runtime.InteropServices;

namespace Luna.RHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeVertexBufferView
{
    public readonly IntPtr Buffer;
    public readonly ulong Offset;
    public readonly uint Size;
    public readonly uint ElementSize;

    private NativeVertexBufferView(VertexBufferView view)
    {
        Buffer = RhiBuffer.GetNativeBufferPointer(view.Buffer);
        Offset = view.Offset;
        Size = view.Size;
        ElementSize = view.ElementSize;
    }

    internal static NativeVertexBufferView FromPublic(VertexBufferView view)
    {
        return new NativeVertexBufferView(view);
    }
}
