using System.Runtime.InteropServices;

namespace Luna.RHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeColorAttachment
{
    public readonly System.IntPtr Texture;
    public readonly uint LoadOp;
    public readonly uint StoreOp;
    public readonly float ClearRed;
    public readonly float ClearGreen;
    public readonly float ClearBlue;
    public readonly float ClearAlpha;
    public readonly uint ViewType;
    public readonly uint Format;
    public readonly uint MipSlice;
    public readonly uint ArraySlice;

    private NativeColorAttachment(ColorAttachment attachment)
    {
        Texture = RhiTexture.GetNativeTexturePointer(attachment.Texture);
        LoadOp = (uint)attachment.LoadOp;
        StoreOp = (uint)attachment.StoreOp;
        ClearRed = attachment.ClearValue.Red;
        ClearGreen = attachment.ClearValue.Green;
        ClearBlue = attachment.ClearValue.Blue;
        ClearAlpha = attachment.ClearValue.Alpha;
        ViewType = (uint)attachment.ViewType;
        Format = (uint)attachment.Format;
        MipSlice = attachment.MipSlice;
        ArraySlice = attachment.ArraySlice;
    }

    internal static NativeColorAttachment FromPublic(ColorAttachment attachment)
    {
        return new NativeColorAttachment(attachment);
    }
}
