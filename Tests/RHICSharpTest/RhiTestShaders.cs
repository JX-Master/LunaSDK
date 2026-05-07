using System;
using System.IO;
using Luna.RHI;
using Luna.Runtime;

internal static class RhiTestShaders
{
    public static ShaderData LoadTriangleVertexShader() => Load("TestTriangleVS", "vs_main");

    public static ShaderData LoadTrianglePixelShader() => Load("TestTrianglePS", "ps_main");

    public static ShaderData LoadTextureVertexShader() => Load("TestTextureVS", "vs_main");

    public static ShaderData LoadTexturePixelShader() => Load("TestTexturePS", "ps_main");

    public static ShaderData LoadBoxVertexShader() => Load("TestBoxVS", "vs_main");

    public static ShaderData LoadBoxPixelShader() => Load("TestBoxPS", "ps_main");

    private static ShaderData Load(string shaderName, string entryPoint)
    {
        var asset = GetCurrentShaderAsset(shaderName, entryPoint);
        var path = Path.Combine(AppContext.BaseDirectory, asset.FileName);
        if (!File.Exists(path))
        {
            throw new FileNotFoundException($"The precompiled shader asset '{asset.FileName}' was not found for backend {Module.BackendType}.", path);
        }
        return new ShaderData(RuntimeFile.LoadData(path), asset.EntryPoint, asset.Format);
    }

    private static ShaderAsset GetCurrentShaderAsset(string shaderName, string entryPoint)
    {
        return Module.BackendType switch
        {
            BackendType.D3D12 => new ShaderAsset($"{shaderName}.dxil", entryPoint, ShaderDataFormat.Dxil),
            BackendType.Vulkan => new ShaderAsset($"{shaderName}.spv", entryPoint, ShaderDataFormat.Spirv),
            BackendType.Metal => new ShaderAsset($"{shaderName}.metallib", entryPoint, ShaderDataFormat.MetalLib),
            _ => throw new NotSupportedException($"Unsupported RHI backend {Module.BackendType}.")
        };
    }

    private readonly record struct ShaderAsset(string FileName, string EntryPoint, ShaderDataFormat Format);
}
