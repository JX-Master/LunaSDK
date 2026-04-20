using System.Runtime.InteropServices;

namespace Luna.RHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeDepthStencilAttachment
{
    public readonly System.IntPtr Texture;
    public readonly int ReadOnly;
    public readonly uint DepthLoadOp;
    public readonly uint DepthStoreOp;
    public readonly float DepthClearValue;
    public readonly uint StencilLoadOp;
    public readonly uint StencilStoreOp;
    public readonly byte StencilClearValue;
    public readonly uint ViewType;
    public readonly uint Format;
    public readonly uint MipSlice;
    public readonly uint ArraySlice;

    private NativeDepthStencilAttachment(DepthStencilAttachment attachment)
    {
        Texture = RhiTexture.GetNativeTexturePointer(attachment.Texture);
        ReadOnly = attachment.ReadOnly ? 1 : 0;
        DepthLoadOp = (uint)attachment.DepthLoadOp;
        DepthStoreOp = (uint)attachment.DepthStoreOp;
        DepthClearValue = attachment.DepthClearValue;
        StencilLoadOp = (uint)attachment.StencilLoadOp;
        StencilStoreOp = (uint)attachment.StencilStoreOp;
        StencilClearValue = attachment.StencilClearValue;
        ViewType = (uint)attachment.ViewType;
        Format = (uint)attachment.Format;
        MipSlice = attachment.MipSlice;
        ArraySlice = attachment.ArraySlice;
    }

    internal static NativeDepthStencilAttachment FromPublic(DepthStencilAttachment attachment)
    {
        return new NativeDepthStencilAttachment(attachment);
    }
}
