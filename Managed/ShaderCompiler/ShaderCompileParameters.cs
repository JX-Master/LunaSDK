namespace Luna.ShaderCompiler;

public sealed class ShaderCompileParameters
{
    public string Source { get; init; } = string.Empty;

    public string SourceName { get; init; } = string.Empty;

    public string SourceFilePath { get; init; } = string.Empty;

    public string EntryPoint { get; init; } = "main";

    public ShaderCompilerTargetFormat TargetFormat { get; init; }

    public ShaderCompilerShaderType ShaderType { get; init; } = ShaderCompilerShaderType.Vertex;

    public uint ShaderModelMajor { get; init; } = 6;

    public uint ShaderModelMinor { get; init; }

    public ShaderCompilerOptimizationLevel OptimizationLevel { get; init; } = ShaderCompilerOptimizationLevel.Full;

    public bool Debug { get; init; }

    public bool SkipValidation { get; init; }

    public ShaderCompilerMatrixPackMode MatrixPackMode { get; init; } = ShaderCompilerMatrixPackMode.ColumnMajor;

    public ShaderCompilerMetalPlatform MetalPlatform { get; init; } = ShaderCompilerMetalPlatform.Macos;
}
