using System.Text;

namespace LunaBuild.Core.MakeSystem;

public sealed class CppslShaderActionExecutor : KnownActionExecutor
{
    private readonly TimeSpan _actionTimeout;

    public CppslShaderActionExecutor(TimeSpan? actionTimeout = null)
        : base("cppsl.shader")
    {
        _actionTimeout = actionTimeout ?? TimeSpan.FromMinutes(5);
    }

    public override async Task ExecuteAsync(MakeActionContext context, CancellationToken cancellationToken)
    {
        var payload = ActionPayload.Parse(context.ActionPayload);
        var format = payload.Required("format");
        if(!format.Equals("dxil", StringComparison.OrdinalIgnoreCase))
        {
            throw new MakeSystemException($"CPPSL shader executor currently supports D3D12/DXIL only. Requested format: {format}.");
        }

        var source = context.Workspace.ResolveRepositoryPath(payload.Required("source"));
        var header = context.Workspace.ResolveRepositoryPath(payload.Required("header"));
        var intermediateDirectory = context.Workspace.ResolveRepositoryPath(payload.Required("intermediate_dir"));
        var stage = NormalizeStage(payload.Required("stage"));
        var entry = payload.Required("entry");
        var sourceName = Path.GetFileNameWithoutExtension(source);

        Directory.CreateDirectory(intermediateDirectory);
        Directory.CreateDirectory(Path.GetDirectoryName(header)!);

        var nativeExtractor = LocateNativeExtractor(context.Workspace);
        await RunCppslAsync(context.Workspace, source, intermediateDirectory, stage, entry, nativeExtractor, cancellationToken);

        var hlsl = Path.Combine(intermediateDirectory, sourceName + ".hlsl");
        if(!File.Exists(hlsl))
        {
            throw new MakeSystemException($"CPPSL did not produce expected HLSL output: {hlsl}");
        }

        var dxil = Path.Combine(intermediateDirectory, sourceName + ".dxil");
        await RunDxcAsync(context.Workspace, hlsl, dxil, stage, entry, cancellationToken);
        WriteShaderHeader(header, sourceName, dxil, "dxil", entry);
    }

    private async Task RunCppslAsync(
        BuildWorkspace workspace,
        string source,
        string outputDirectory,
        string stage,
        string entry,
        string nativeExtractor,
        CancellationToken cancellationToken)
    {
        var includeStd = Path.Combine(workspace.RootDirectory, "Tools", "CPPSL", "std");
        var cppslc = LocateCppslc(workspace);
        var args = new List<string>
        {
            "compile",
            source,
            "--stage",
            stage,
            "--entry",
            entry,
            "--include",
            includeStd,
            "--include",
            Path.GetDirectoryName(source)!,
            "--out",
            outputDirectory,
            "--target",
            "hlsl",
            "--native-extractor",
            nativeExtractor,
        };

        var result = await ProcessRunner.RunAsync(cppslc, args, workspace.RootDirectory, _actionTimeout, cancellationToken);
        if(result.ExitCode != 0)
        {
            throw new MakeSystemException($"CPPSL compile failed for {source}:{Environment.NewLine}{result.Output}");
        }
    }

    private async Task RunDxcAsync(
        BuildWorkspace workspace,
        string hlsl,
        string dxil,
        string stage,
        string entry,
        CancellationToken cancellationToken)
    {
        var dxc = LocateDxc(workspace);
        var args = new List<string>
        {
            "-E",
            entry,
            "-T",
            DxcProfile(stage),
            "-Od",
            "-Zi",
            "-Qembed_debug",
            "-Qsource_in_debug_module",
            "-Zpc",
            "-Fo",
            dxil,
            hlsl,
        };
        var result = await ProcessRunner.RunAsync(dxc, args, workspace.RootDirectory, _actionTimeout, cancellationToken);
        if(result.ExitCode != 0)
        {
            throw new MakeSystemException($"DXC compile failed for {hlsl}:{Environment.NewLine}{result.Output}");
        }
    }

    private static string LocateNativeExtractor(BuildWorkspace workspace)
    {
        var executable = OperatingSystem.IsWindows() ? "cppsl-native-extractor.exe" : "cppsl-native-extractor";
        var candidates = new[]
        {
            Path.Combine(workspace.RootDirectory, "SDKs", "CPPSL", "windows", "x64", "bin", executable),
            Path.Combine(workspace.RootDirectory, "Tools", "CPPSL", "native", "bin", executable),
        };
        foreach(var candidate in candidates)
        {
            if(File.Exists(candidate))
            {
                return candidate;
            }
        }
        throw new MakeSystemException(
            "CPPSL native extractor is missing. Expected one of:" +
            string.Concat(candidates.Select(path => $"{Environment.NewLine}  {path}")));
    }

    private static string LocateCppslc(BuildWorkspace workspace)
    {
        var executable = OperatingSystem.IsWindows() ? "cppslc.exe" : "cppslc";
        var candidates = new[]
        {
            Path.Combine(workspace.RootDirectory, "SDKs", "CPPSL", "windows", "x64", "bin", executable),
            Path.Combine(workspace.RootDirectory, "Tools", "CPPSL", "src", "CPPSL.Cli", "bin", "Debug", "net9.0", executable),
        };
        foreach(var candidate in candidates)
        {
            if(File.Exists(candidate))
            {
                return candidate;
            }
        }
        throw new MakeSystemException(
            "cppslc is missing. Expected one of:" +
            string.Concat(candidates.Select(path => $"{Environment.NewLine}  {path}")));
    }

    private static string LocateDxc(BuildWorkspace workspace)
    {
        var candidates = new List<string>
        {
            Path.Combine(workspace.RootDirectory, "SDKs", "vulkan-tools", "windows", "x64", "bin", "dxc.exe"),
        };
        var vulkanSdk = Environment.GetEnvironmentVariable("VULKAN_SDK");
        if(!string.IsNullOrWhiteSpace(vulkanSdk))
        {
            candidates.Add(Path.Combine(vulkanSdk, "Bin", OperatingSystem.IsWindows() ? "dxc.exe" : "dxc"));
        }

        var pathValue = Environment.GetEnvironmentVariable("PATH") ?? string.Empty;
        foreach(var pathEntry in pathValue.Split(Path.PathSeparator, StringSplitOptions.RemoveEmptyEntries))
        {
            candidates.Add(Path.Combine(pathEntry, OperatingSystem.IsWindows() ? "dxc.exe" : "dxc"));
        }

        foreach(var candidate in candidates)
        {
            if(File.Exists(candidate))
            {
                return candidate;
            }
        }
        throw new MakeSystemException("DXC was not found. Install Vulkan SDK or provide SDKs/vulkan-tools/windows/x64/bin/dxc.exe.");
    }

    private static string NormalizeStage(string stage)
    {
        return stage.Equals("pixel", StringComparison.OrdinalIgnoreCase)
            ? "fragment"
            : stage;
    }

    private static string DxcProfile(string stage)
    {
        return stage.ToLowerInvariant() switch
        {
            "vertex" => "vs_6_0",
            "fragment" or "pixel" => "ps_6_0",
            "compute" => "cs_6_0",
            _ => throw new MakeSystemException($"Unsupported DXIL shader stage: {stage}"),
        };
    }

    private static void WriteShaderHeader(string header, string sourceName, string binaryPath, string dataFormat, string entry)
    {
        var bytes = File.ReadAllBytes(binaryPath);
        var builder = new StringBuilder();
        builder.AppendLine("// Autogenerated by cppslc, do not modify.");
        builder.AppendLine("#pragma once");
        builder.AppendLine("#include <Luna/RHI/CppslShaderHelper.hpp>");
        builder.AppendLine();
        builder.AppendLine("namespace Luna");
        builder.AppendLine("{");
        builder.Append("    constexpr u8 SHADER_DATA_").Append(sourceName).Append("[] = {");
        for(var i = 0; i < bytes.Length; ++i)
        {
            if(i != 0)
            {
                builder.Append(',');
            }
            builder.Append(bytes[i]);
        }
        builder.AppendLine("};");
        builder.Append("    constexpr usize SHADER_DATA_SIZE_").Append(sourceName).Append(" = sizeof(SHADER_DATA_").Append(sourceName).AppendLine(");");
        builder.Append("    constexpr RHI::ShaderDataFormat SHADER_DATA_FORMAT_").Append(sourceName).Append(" = RHI::ShaderDataFormat::").Append(dataFormat).AppendLine(";");
        builder.Append("    constexpr c8 SHADER_ENTRY_POINT_").Append(sourceName).Append("[] = \"").Append(entry).AppendLine("\";");
        builder.Append("    constexpr u32 SHADER_METAL_NUMTHREADS_X_").Append(sourceName).AppendLine(" = 0;");
        builder.Append("    constexpr u32 SHADER_METAL_NUMTHREADS_Y_").Append(sourceName).AppendLine(" = 0;");
        builder.Append("    constexpr u32 SHADER_METAL_NUMTHREADS_Z_").Append(sourceName).AppendLine(" = 0;");
        builder.AppendLine("}");
        File.WriteAllText(header, builder.ToString());
    }
}
