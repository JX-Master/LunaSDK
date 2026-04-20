using System;
using System.Runtime.InteropServices;

namespace Luna.ShaderCompiler.Internal;

[StructLayout(LayoutKind.Sequential)]
internal struct NativeShaderCompileResult
{
    public IntPtr Data;
    public ulong DataSize;
    public uint Format;
    public IntPtr EntryPoint;
    public uint MetalNumThreadsX;
    public uint MetalNumThreadsY;
    public uint MetalNumThreadsZ;
}
