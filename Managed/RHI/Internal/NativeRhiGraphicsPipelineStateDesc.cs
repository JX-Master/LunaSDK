using System;
using System.Runtime.InteropServices;

namespace Luna.RHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeInputBindingDesc
{
    public readonly uint BindingSlot;
    public readonly uint ElementSize;
    public readonly uint InputRate;

    internal NativeInputBindingDesc(InputBindingDesc desc)
    {
        BindingSlot = desc.BindingSlot;
        ElementSize = desc.ElementSize;
        InputRate = (uint)desc.InputRate;
    }
}

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeInputAttributeDesc
{
    public readonly uint Location;
    public readonly uint BindingSlot;
    public readonly uint Offset;
    public readonly uint Format;

    internal NativeInputAttributeDesc(InputAttributeDesc desc)
    {
        Location = desc.Location;
        BindingSlot = desc.BindingSlot;
        Offset = desc.Offset;
        Format = (uint)desc.Format;
    }
}

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeShaderData
{
    public readonly IntPtr Data;
    public readonly ulong Size;
    public readonly IntPtr EntryPoint;
    public readonly uint Format;

    internal NativeShaderData(IntPtr data, ulong size, IntPtr entryPoint, ShaderDataFormat format)
    {
        Data = data;
        Size = size;
        EntryPoint = entryPoint;
        Format = (uint)format;
    }
}

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeRasterizerDesc
{
    public readonly int DepthBias;
    public readonly float SlopeScaledDepthBias;
    public readonly float DepthBiasClamp;
    public readonly uint FillMode;
    public readonly uint CullMode;
    public readonly int FrontCounterClockwise;
    public readonly int DepthClampEnable;

    internal NativeRasterizerDesc(RasterizerDesc desc)
    {
        DepthBias = desc.DepthBias;
        SlopeScaledDepthBias = desc.SlopeScaledDepthBias;
        DepthBiasClamp = desc.DepthBiasClamp;
        FillMode = (uint)desc.FillMode;
        CullMode = (uint)desc.CullMode;
        FrontCounterClockwise = desc.FrontCounterClockwise ? 1 : 0;
        DepthClampEnable = desc.DepthClampEnable ? 1 : 0;
    }
}

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeDepthStencilDesc
{
    public readonly int DepthTestEnable;
    public readonly int DepthWriteEnable;
    public readonly uint DepthFunction;
    public readonly int StencilEnable;
    public readonly byte StencilReadMask;
    public readonly byte StencilWriteMask;
    public readonly uint FrontStencilFailOp;
    public readonly uint FrontStencilDepthFailOp;
    public readonly uint FrontStencilPassOp;
    public readonly uint FrontStencilFunction;
    public readonly uint BackStencilFailOp;
    public readonly uint BackStencilDepthFailOp;
    public readonly uint BackStencilPassOp;
    public readonly uint BackStencilFunction;

    internal NativeDepthStencilDesc(DepthStencilDesc desc)
    {
        DepthTestEnable = desc.DepthTestEnable ? 1 : 0;
        DepthWriteEnable = desc.DepthWriteEnable ? 1 : 0;
        DepthFunction = (uint)desc.DepthFunction;
        StencilEnable = desc.StencilEnable ? 1 : 0;
        StencilReadMask = desc.StencilReadMask;
        StencilWriteMask = desc.StencilWriteMask;
        FrontStencilFailOp = (uint)desc.FrontFace.StencilFailOp;
        FrontStencilDepthFailOp = (uint)desc.FrontFace.StencilDepthFailOp;
        FrontStencilPassOp = (uint)desc.FrontFace.StencilPassOp;
        FrontStencilFunction = (uint)desc.FrontFace.StencilFunction;
        BackStencilFailOp = (uint)desc.BackFace.StencilFailOp;
        BackStencilDepthFailOp = (uint)desc.BackFace.StencilDepthFailOp;
        BackStencilPassOp = (uint)desc.BackFace.StencilPassOp;
        BackStencilFunction = (uint)desc.BackFace.StencilFunction;
    }
}

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeAttachmentBlendDesc
{
    public readonly int BlendEnable;
    public readonly uint SourceBlendColor;
    public readonly uint DestinationBlendColor;
    public readonly uint BlendOpColor;
    public readonly uint SourceBlendAlpha;
    public readonly uint DestinationBlendAlpha;
    public readonly uint BlendOpAlpha;
    public readonly uint ColorWriteMask;

    internal NativeAttachmentBlendDesc(AttachmentBlendDesc desc)
    {
        BlendEnable = desc.BlendEnable ? 1 : 0;
        SourceBlendColor = (uint)desc.SourceBlendColor;
        DestinationBlendColor = (uint)desc.DestinationBlendColor;
        BlendOpColor = (uint)desc.BlendOpColor;
        SourceBlendAlpha = (uint)desc.SourceBlendAlpha;
        DestinationBlendAlpha = (uint)desc.DestinationBlendAlpha;
        BlendOpAlpha = (uint)desc.BlendOpAlpha;
        ColorWriteMask = (uint)desc.ColorWriteMask;
    }
}

[StructLayout(LayoutKind.Sequential)]
internal struct NativeBlendDesc
{
    public int AlphaToCoverageEnable;
    public int IndependentBlendEnable;
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
    public NativeAttachmentBlendDesc[] Attachments = new NativeAttachmentBlendDesc[8];

    internal NativeBlendDesc(BlendDesc? desc)
    {
        AlphaToCoverageEnable = desc?.AlphaToCoverageEnable == true ? 1 : 0;
        IndependentBlendEnable = desc?.IndependentBlendEnable == true ? 1 : 0;
        var attachments = desc?.Attachments ?? Array.Empty<AttachmentBlendDesc>();
        if (attachments.Length > 8)
        {
            throw new ArgumentOutOfRangeException(nameof(desc), "A graphics pipeline can use at most 8 blend attachments.");
        }
        for (var i = 0; i < Attachments.Length; ++i)
        {
            Attachments[i] = new NativeAttachmentBlendDesc(i < attachments.Length ? attachments[i] : AttachmentBlendDesc.Default);
        }
    }
}

[StructLayout(LayoutKind.Sequential)]
internal sealed class NativeGraphicsPipelineStateDesc
{
    public IntPtr InputBindings;
    public ulong InputBindingCount;
    public IntPtr InputAttributes;
    public ulong InputAttributeCount;
    public IntPtr PipelineLayout;
    public NativeShaderData VertexShader;
    public NativeShaderData PixelShader;
    public NativeRasterizerDesc RasterizerState;
    public NativeDepthStencilDesc DepthStencilState;
    public NativeBlendDesc BlendState = new(null);
    public uint IndexBufferStripCutValue;
    public uint PrimitiveTopology;
    public byte NumColorAttachments;
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
    public uint[] ColorFormats = new uint[8];
    public uint DepthStencilFormat;
    public uint SampleCount;
}
