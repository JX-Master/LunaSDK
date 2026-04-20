using System;
using System.Runtime.InteropServices;

namespace Luna.RHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal sealed class NativeComputePipelineStateDesc
{
    public IntPtr PipelineLayout;
    public NativeShaderData ComputeShader;
    public uint MetalNumThreadsX;
    public uint MetalNumThreadsY;
    public uint MetalNumThreadsZ;
}
