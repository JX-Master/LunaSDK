using System;
using System.Runtime.InteropServices;

namespace Luna.RHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeDescriptorSetLayoutBinding
{
    public readonly uint BindingSlot;
    public readonly uint NumDescriptors;
    public readonly uint Type;
    public readonly uint TextureViewType;
    public readonly uint ShaderVisibilityFlags;

    internal NativeDescriptorSetLayoutBinding(DescriptorSetLayoutBinding binding)
    {
        BindingSlot = binding.BindingSlot;
        NumDescriptors = binding.Count;
        Type = (uint)binding.Type;
        TextureViewType = (uint)binding.TextureViewType;
        ShaderVisibilityFlags = (uint)binding.Visibility;
    }
}

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeBufferViewDesc
{
    public readonly ulong FirstElement;
    public readonly IntPtr Buffer;
    public readonly uint ElementCount;
    public readonly uint ElementSize;

    private NativeBufferViewDesc(BufferViewDesc desc)
    {
        FirstElement = desc.FirstElement;
        Buffer = RhiBuffer.GetNativeBufferPointer(desc.Buffer);
        ElementCount = desc.ElementCount;
        ElementSize = desc.ElementSize;
    }

    internal static NativeBufferViewDesc FromPublic(BufferViewDesc desc)
    {
        return new NativeBufferViewDesc(desc);
    }
}

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeTextureViewDesc
{
    public readonly IntPtr Texture;
    public readonly uint Type;
    public readonly uint Format;
    public readonly uint MipSlice;
    public readonly uint MipSize;
    public readonly uint ArraySlice;
    public readonly uint ArraySize;

    private NativeTextureViewDesc(TextureViewDesc desc)
    {
        Texture = RhiTexture.GetNativeTexturePointer(desc.Texture);
        Type = (uint)desc.Type;
        Format = (uint)desc.Format;
        MipSlice = desc.MipSlice;
        MipSize = desc.MipSize;
        ArraySlice = desc.ArraySlice;
        ArraySize = desc.ArraySize;
    }

    internal static NativeTextureViewDesc FromPublic(TextureViewDesc desc)
    {
        return new NativeTextureViewDesc(desc);
    }
}

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeSamplerDesc
{
    public readonly uint MinFilter;
    public readonly uint MagFilter;
    public readonly uint MipFilter;
    public readonly uint AddressU;
    public readonly uint AddressV;
    public readonly uint AddressW;
    public readonly int AnisotropyEnable;
    public readonly int CompareEnable;
    public readonly uint CompareFunction;
    public readonly uint BorderColor;
    public readonly uint MaxAnisotropy;
    public readonly float MinLod;
    public readonly float MaxLod;

    internal NativeSamplerDesc(SamplerDesc desc)
    {
        MinFilter = (uint)desc.MinFilter;
        MagFilter = (uint)desc.MagFilter;
        MipFilter = (uint)desc.MipFilter;
        AddressU = (uint)desc.AddressU;
        AddressV = (uint)desc.AddressV;
        AddressW = (uint)desc.AddressW;
        AnisotropyEnable = desc.AnisotropyEnable ? 1 : 0;
        CompareEnable = desc.CompareEnable ? 1 : 0;
        CompareFunction = (uint)desc.CompareFunction;
        BorderColor = (uint)desc.BorderColor;
        MaxAnisotropy = desc.MaxAnisotropy;
        MinLod = desc.MinLod;
        MaxLod = desc.MaxLod;
    }
}
