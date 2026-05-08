using System;
using System.Numerics;
using System.Runtime.InteropServices;
using Luna.RHI;

namespace Luna.ImGui.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeSampledImageHandle
{
    public readonly IntPtr Object;
    public readonly IntPtr ISampledImage;
}

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeGlyphRange
{
    public readonly ushort Start;
    public readonly ushort End;

    public GlyphRange ToPublic() => new(Start, End);
}

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeFloat2
{
    public readonly float X;
    public readonly float Y;

    public NativeFloat2(float x, float y)
    {
        X = x;
        Y = y;
    }

    public static NativeFloat2 FromPublic(Vector2 value) => new(value.X, value.Y);
}

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeFloat4
{
    public readonly float X;
    public readonly float Y;
    public readonly float Z;
    public readonly float W;

    public NativeFloat4(float x, float y, float z, float w)
    {
        X = x;
        Y = y;
        Z = z;
        W = w;
    }

    public static NativeFloat4 FromPublic(Vector4 value) => new(value.X, value.Y, value.Z, value.W);
}

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeMatrix4x4
{
    public readonly NativeFloat4 Row0;
    public readonly NativeFloat4 Row1;
    public readonly NativeFloat4 Row2;
    public readonly NativeFloat4 Row3;

    public NativeMatrix4x4(NativeFloat4 row0, NativeFloat4 row1, NativeFloat4 row2, NativeFloat4 row3)
    {
        Row0 = row0;
        Row1 = row1;
        Row2 = row2;
        Row3 = row3;
    }

    public static NativeMatrix4x4 FromPublic(Matrix4x4 value) => new(
        new NativeFloat4(value.M11, value.M12, value.M13, value.M14),
        new NativeFloat4(value.M21, value.M22, value.M23, value.M24),
        new NativeFloat4(value.M31, value.M32, value.M33, value.M34),
        new NativeFloat4(value.M41, value.M42, value.M43, value.M44));

    public Matrix4x4 ToPublic() => new(
        Row0.X, Row0.Y, Row0.Z, Row0.W,
        Row1.X, Row1.Y, Row1.Z, Row1.W,
        Row2.X, Row2.Y, Row2.Z, Row2.W,
        Row3.X, Row3.Y, Row3.Z, Row3.W);
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

    public NativeSamplerDesc(SamplerDesc value)
    {
        MinFilter = (uint)value.MinFilter;
        MagFilter = (uint)value.MagFilter;
        MipFilter = (uint)value.MipFilter;
        AddressU = (uint)value.AddressU;
        AddressV = (uint)value.AddressV;
        AddressW = (uint)value.AddressW;
        AnisotropyEnable = value.AnisotropyEnable ? 1 : 0;
        CompareEnable = value.CompareEnable ? 1 : 0;
        CompareFunction = (uint)value.CompareFunction;
        BorderColor = (uint)value.BorderColor;
        MaxAnisotropy = value.MaxAnisotropy;
        MinLod = value.MinLod;
        MaxLod = value.MaxLod;
    }

    public SamplerDesc ToPublic() => new(
        (Filter)MinFilter,
        (Filter)MagFilter,
        (Filter)MipFilter,
        (TextureAddressMode)AddressU,
        (TextureAddressMode)AddressV,
        (TextureAddressMode)AddressW,
        AnisotropyEnable != 0,
        MaxAnisotropy,
        (BorderColor)BorderColor,
        MinLod,
        MaxLod,
        CompareEnable != 0,
        (CompareFunction)CompareFunction);
}
