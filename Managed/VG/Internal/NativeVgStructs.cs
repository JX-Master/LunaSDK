using System;
using System.Numerics;
using System.Runtime.InteropServices;
using Luna.RHI;

namespace Luna.VG.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeShapeBufferHandle
{
    public readonly IntPtr Object;
    public readonly IntPtr IShapeBuffer;
}

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeFontAtlasHandle
{
    public readonly IntPtr Object;
    public readonly IntPtr IFontAtlas;
}

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeShapeDrawListHandle
{
    public readonly IntPtr Object;
    public readonly IntPtr IShapeDrawList;
}

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeShapeRendererHandle
{
    public readonly IntPtr Object;
    public readonly IntPtr IShapeRenderer;
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
    public Vector2 ToPublic() => new(X, Y);
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
    public Vector4 ToPublic() => new(X, Y, Z, W);
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
internal readonly struct NativeTextArrangeSection
{
    public readonly IntPtr FontFile;
    public readonly ulong NumChars;
    public readonly uint FontIndex;
    public readonly NativeFloat4 Color;
    public readonly float FontSize;
    public readonly float CharSpan;
    public readonly float LineSpan;

    public NativeTextArrangeSection(IntPtr fontFile, ulong numChars, uint fontIndex, NativeFloat4 color, float fontSize, float charSpan, float lineSpan)
    {
        FontFile = fontFile;
        NumChars = numChars;
        FontIndex = fontIndex;
        Color = color;
        FontSize = fontSize;
        CharSpan = charSpan;
        LineSpan = lineSpan;
    }

    public static NativeTextArrangeSection[] FromPublic(TextArrangeSection[] sections)
    {
        var result = new NativeTextArrangeSection[sections.Length];
        for (var i = 0; i < sections.Length; ++i)
        {
            ArgumentNullException.ThrowIfNull(sections[i]);
            ArgumentNullException.ThrowIfNull(sections[i].FontFile);
            result[i] = new NativeTextArrangeSection(
                Font.Internal.NativeFontFile.GetNativeFontPointer(sections[i].FontFile),
                sections[i].NumChars,
                sections[i].FontIndex,
                NativeFloat4.FromPublic(sections[i].Color),
                sections[i].FontSize,
                sections[i].CharSpan,
                sections[i].LineSpan);
        }
        return result;
    }
}

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeTextGlyphArrangeResult
{
    public readonly RectF BoundingRect;
    public readonly float OriginOffset;
    public readonly float AdvanceLength;
    public readonly uint Character;
    public readonly uint Index;

    public TextGlyphArrangeResult ToPublic() => new(BoundingRect, OriginOffset, AdvanceLength, Character, Index);
}

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeTextLineArrangeResult
{
    public readonly RectF BoundingRect;
    public readonly float BaselineOffset;
    public readonly float Ascent;
    public readonly float Decent;
    public readonly float LineGap;

    public TextLineArrangeResult ToPublic(TextGlyphArrangeResult[] glyphs) => new(BoundingRect, BaselineOffset, Ascent, Decent, LineGap, glyphs);
}

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeVertex
{
    public readonly NativeFloat2 Position;
    public readonly NativeFloat2 ShapeCoord;
    public readonly NativeFloat2 TexCoord;
    public readonly uint BeginCommand;
    public readonly uint NumCommands;
    public readonly NativeFloat4 Color;

    public NativeVertex(NativeFloat2 position, NativeFloat2 shapeCoord, NativeFloat2 texCoord, uint beginCommand, uint numCommands, NativeFloat4 color)
    {
        Position = position;
        ShapeCoord = shapeCoord;
        TexCoord = texCoord;
        BeginCommand = beginCommand;
        NumCommands = numCommands;
        Color = color;
    }

    public static NativeVertex FromPublic(Vertex value) => new(
        NativeFloat2.FromPublic(value.Position),
        NativeFloat2.FromPublic(value.ShapeCoord),
        NativeFloat2.FromPublic(value.TexCoord),
        value.BeginCommand,
        value.NumCommands,
        NativeFloat4.FromPublic(value.Color));

    public Vertex ToPublic() => new(Position.ToPublic(), ShapeCoord.ToPublic(), TexCoord.ToPublic(), BeginCommand, NumCommands, Color.ToPublic());
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

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeShapeDrawCall
{
    public readonly RHI.Internal.NativeRhiBufferHandle ShapeBuffer;
    public readonly RHI.Internal.NativeRhiTextureHandle Texture;
    public readonly NativeSamplerDesc Sampler;
    public readonly RectF ClipRect;
    public readonly uint BaseIndex;
    public readonly uint NumIndices;
    public readonly NativeMatrix4x4 Transform;

    public NativeShapeDrawCall(RHI.Internal.NativeRhiBufferHandle shapeBuffer, RHI.Internal.NativeRhiTextureHandle texture, NativeSamplerDesc sampler, RectF clipRect, uint baseIndex, uint numIndices, NativeMatrix4x4 transform)
    {
        ShapeBuffer = shapeBuffer;
        Texture = texture;
        Sampler = sampler;
        ClipRect = clipRect;
        BaseIndex = baseIndex;
        NumIndices = numIndices;
        Transform = transform;
    }

    public static NativeShapeDrawCall FromPublic(ShapeDrawCall value) => new(
        value.ShapeBuffer is null ? default : new RHI.Internal.NativeRhiBufferHandle(value.ShapeBuffer.GetNativeHandle(), RhiBuffer.GetNativeBufferPointer(value.ShapeBuffer)),
        value.Texture is null ? default : new RHI.Internal.NativeRhiTextureHandle(value.Texture.GetNativeHandle(), RhiTexture.GetNativeTexturePointer(value.Texture)),
        new NativeSamplerDesc(value.Sampler),
        value.ClipRect,
        value.BaseIndex,
        value.NumIndices,
        NativeMatrix4x4.FromPublic(value.Transform));
}
