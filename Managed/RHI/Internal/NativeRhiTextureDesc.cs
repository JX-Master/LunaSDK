using System.Runtime.InteropServices;

namespace Luna.RHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeTextureDesc
{
    public readonly uint Type;
    public readonly uint Format;
    public readonly uint Width;
    public readonly uint Height;
    public readonly uint Depth;
    public readonly uint ArraySize;
    public readonly uint MipLevels;
    public readonly uint SampleCount;
    public readonly uint Usages;
    public readonly uint Flags;

    private NativeTextureDesc(
        uint type,
        uint format,
        uint width,
        uint height,
        uint depth,
        uint arraySize,
        uint mipLevels,
        uint sampleCount,
        uint usages,
        uint flags)
    {
        Type = type;
        Format = format;
        Width = width;
        Height = height;
        Depth = depth;
        ArraySize = arraySize;
        MipLevels = mipLevels;
        SampleCount = sampleCount;
        Usages = usages;
        Flags = flags;
    }

    internal static NativeTextureDesc FromPublic(TextureDesc desc)
    {
        return new NativeTextureDesc(
            (uint)desc.Type,
            (uint)desc.Format,
            desc.Width,
            desc.Height,
            desc.Depth,
            desc.ArraySize,
            desc.MipLevels,
            desc.SampleCount,
            (uint)desc.Usages,
            (uint)desc.Flags);
    }

    internal TextureDesc ToPublic()
    {
        return new TextureDesc(
            (TextureType)Type,
            (Format)Format,
            Width,
            Height,
            Depth,
            ArraySize,
            MipLevels,
            SampleCount,
            (TextureUsageFlags)Usages,
            (ResourceFlags)Flags);
    }
}
