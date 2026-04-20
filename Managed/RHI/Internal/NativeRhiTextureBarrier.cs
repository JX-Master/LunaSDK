using System.Runtime.InteropServices;

namespace Luna.RHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeTextureBarrier
{
    public readonly System.IntPtr Texture;
    public readonly NativeSubresourceIndex Subresource;
    public readonly uint Before;
    public readonly uint After;
    public readonly uint Flags;

    private NativeTextureBarrier(TextureBarrier barrier)
    {
        Texture = RhiTexture.GetNativeTexturePointer(barrier.Texture);
        Subresource = new NativeSubresourceIndex(barrier.Subresource);
        Before = (uint)barrier.Before;
        After = (uint)barrier.After;
        Flags = (uint)barrier.Flags;
    }

    internal static NativeTextureBarrier FromPublic(TextureBarrier barrier)
    {
        return new NativeTextureBarrier(barrier);
    }
}
