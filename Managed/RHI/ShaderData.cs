using System;

namespace Luna.RHI;

public readonly struct ShaderData
{
    public ShaderData(byte[] data, string entryPoint, ShaderDataFormat format)
    {
        Data = data ?? throw new ArgumentNullException(nameof(data));
        EntryPoint = entryPoint ?? throw new ArgumentNullException(nameof(entryPoint));
        Format = format;
    }

    public byte[] Data { get; }

    public string EntryPoint { get; }

    public ShaderDataFormat Format { get; }
}
