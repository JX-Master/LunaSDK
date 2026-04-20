using System;
using Luna.RHI;
using Luna.ShaderCompiler;

internal static class RhiTestShaders
{
    public const string TriangleVertexShader = """
struct VS_INPUT
{
    [[vk::location(0)]]
    float2 pos : POSITION;
    [[vk::location(1)]]
    float4 col : COLOR0;
};
struct PS_INPUT
{
    [[vk::location(0)]]
    float4 pos : SV_POSITION;
    [[vk::location(1)]]
    float4 col : COLOR0;
};
PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = float4(input.pos.x, input.pos.y, 0.0f, 1.0f);
    output.col = input.col;
    return output;
}
""";

    public const string TrianglePixelShader = """
struct PS_INPUT
{
    [[vk::location(0)]]
    float4 pos : SV_POSITION;
    [[vk::location(1)]]
    float4 col : COLOR0;
};
[[vk::location(0)]]
float4 main(PS_INPUT input) : SV_Target
{
    return input.col;
}
""";

    public const string TextureVertexShader = """
struct VS_INPUT
{
    [[vk::location(0)]]
    float2 pos : POSITION;
    [[vk::location(1)]]
    float2 uv : TEXCOORD0;
};
struct PS_INPUT
{
    [[vk::location(0)]]
    float4 pos : SV_POSITION;
    [[vk::location(1)]]
    float2 uv : TEXCOORD0;
};
PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = float4(input.pos.x, input.pos.y, 0.0f, 1.0f);
    output.uv = input.uv;
    return output;
}
""";

    public const string TexturePixelShader = """
struct PS_INPUT
{
    [[vk::location(0)]]
    float4 pos : SV_POSITION;
    [[vk::location(1)]]
    float2 uv : TEXCOORD0;
};
Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s1);
[[vk::location(0)]]
float4 main(PS_INPUT input) : SV_Target
{
    return clamp(texture0.Sample(sampler0, input.uv), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(1.0f, 1.0f, 1.0f, 1.0f));
}
""";

    public const string BoxVertexShader = """
cbuffer vertexBuffer : register(b0)
{
    row_major float4x4 world_to_proj;
};
Texture2D tex : register(t1);
SamplerState tex_sampler : register(s2);
struct VS_INPUT
{
    [[vk::location(0)]]
    float3 position : POSITION;
    [[vk::location(1)]]
    float2 texcoord : TEXCOORD;
};
struct PS_INPUT
{
    [[vk::location(0)]]
    float4 position : SV_POSITION;
    [[vk::location(1)]]
    float2 texcoord : TEXCOORD;
};
PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    output.position = mul(world_to_proj, float4(input.position, 1.0f));
    output.texcoord = input.texcoord;
    return output;
}
""";

    public const string BoxPixelShader = """
cbuffer vertexBuffer : register(b0)
{
    row_major float4x4 world_to_proj;
};
Texture2D tex : register(t1);
SamplerState tex_sampler : register(s2);
struct PS_INPUT
{
    [[vk::location(0)]]
    float4 position : SV_POSITION;
    [[vk::location(1)]]
    float2 texcoord : TEXCOORD;
};
[[vk::location(0)]]
float4 main(PS_INPUT input) : SV_Target
{
    return tex.Sample(tex_sampler, input.texcoord);
}
""";

    public static ShaderData Compile(string source, string sourceName, ShaderCompilerShaderType shaderType)
    {
        var result = ShaderCompilerModule.Compile(new ShaderCompileParameters
        {
            Source = source,
            SourceName = sourceName,
            EntryPoint = "main",
            TargetFormat = GetCurrentShaderTargetFormat(),
            ShaderType = shaderType,
            ShaderModelMajor = 6,
            ShaderModelMinor = 0,
            MatrixPackMode = ShaderCompilerMatrixPackMode.RowMajor
        });
        return new ShaderData(result.Data, result.EntryPoint, ToShaderDataFormat(result.Format));
    }

    private static ShaderCompilerTargetFormat GetCurrentShaderTargetFormat()
    {
        return Module.BackendType switch
        {
            BackendType.D3D12 => ShaderCompilerTargetFormat.Dxil,
            BackendType.Vulkan => ShaderCompilerTargetFormat.Spirv,
            BackendType.Metal => ShaderCompilerTargetFormat.Msl,
            _ => throw new NotSupportedException($"Unsupported RHI backend {Module.BackendType}.")
        };
    }

    private static ShaderDataFormat ToShaderDataFormat(ShaderCompilerTargetFormat format)
    {
        return format switch
        {
            ShaderCompilerTargetFormat.Dxil => ShaderDataFormat.Dxil,
            ShaderCompilerTargetFormat.Spirv => ShaderDataFormat.Spirv,
            ShaderCompilerTargetFormat.Msl => ShaderDataFormat.Msl,
            _ => ShaderDataFormat.None
        };
    }
}
