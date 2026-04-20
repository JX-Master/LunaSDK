namespace Luna.ShaderCompiler;

public sealed class ShaderCompileResult
{
    public ShaderCompileResult(
        byte[] data,
        ShaderCompilerTargetFormat format,
        string entryPoint,
        uint metalNumThreadsX,
        uint metalNumThreadsY,
        uint metalNumThreadsZ)
    {
        Data = data;
        Format = format;
        EntryPoint = entryPoint;
        MetalNumThreadsX = metalNumThreadsX;
        MetalNumThreadsY = metalNumThreadsY;
        MetalNumThreadsZ = metalNumThreadsZ;
    }

    public byte[] Data { get; }

    public ShaderCompilerTargetFormat Format { get; }

    public string EntryPoint { get; }

    public uint MetalNumThreadsX { get; }

    public uint MetalNumThreadsY { get; }

    public uint MetalNumThreadsZ { get; }
}
