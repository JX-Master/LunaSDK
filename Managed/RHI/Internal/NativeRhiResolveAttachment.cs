using System;
using System.Runtime.InteropServices;

namespace Luna.RHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeResolveAttachment
{
    public readonly IntPtr Texture;
    public readonly uint MipSlice;
    public readonly uint ArraySlice;
    public readonly uint ArraySize;

    private NativeResolveAttachment(ResolveAttachment attachment)
    {
        Texture = RhiTexture.GetNativeTexturePointer(attachment.Texture);
        MipSlice = attachment.MipSlice;
        ArraySlice = attachment.ArraySlice;
        ArraySize = attachment.ArraySize;
    }

    internal static NativeResolveAttachment FromPublic(ResolveAttachment attachment)
    {
        return new NativeResolveAttachment(attachment);
    }
}
