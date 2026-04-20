using System.Runtime.InteropServices;

namespace Luna.RHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeBufferBarrier
{
    public readonly System.IntPtr Buffer;
    public readonly uint Before;
    public readonly uint After;
    public readonly uint Flags;

    private NativeBufferBarrier(BufferBarrier barrier)
    {
        Buffer = RhiBuffer.GetNativeBufferPointer(barrier.Buffer);
        Before = (uint)barrier.Before;
        After = (uint)barrier.After;
        Flags = (uint)barrier.Flags;
    }

    internal static NativeBufferBarrier FromPublic(BufferBarrier barrier)
    {
        return new NativeBufferBarrier(barrier);
    }
}
