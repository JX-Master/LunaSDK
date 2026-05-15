using System.Text;
using System.Text.Json;

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
        var source = context.Workspace.ResolveRepositoryPath(payload.Required("source"));
        var header = context.Workspace.ResolveRepositoryPath(payload.Required("header"));
        var intermediateDirectory = context.Workspace.ResolveRepositoryPath(payload.Required("intermediate_dir"));
        var stage = NormalizeStage(payload.Required("stage"));
        var entry = payload.Required("entry");
        var sourceName = Path.GetFileNameWithoutExtension(source);

        Directory.CreateDirectory(intermediateDirectory);
        Directory.CreateDirectory(Path.GetDirectoryName(header)!);

        var nativeExtractor = LocateNativeExtractor(context.Workspace);
        if(format.Equals("dxil", StringComparison.OrdinalIgnoreCase))
        {
            await RunCppslAsync(context.Workspace, source, intermediateDirectory, stage, entry, nativeExtractor, "hlsl", cancellationToken);

            var hlsl = Path.Combine(intermediateDirectory, sourceName + ".hlsl");
            if(!File.Exists(hlsl))
            {
                throw new MakeSystemException($"CPPSL did not produce expected HLSL output: {hlsl}");
            }

            var dxil = Path.Combine(intermediateDirectory, sourceName + ".dxil");
            await RunDxcAsync(context.Workspace, hlsl, dxil, stage, entry, cancellationToken);
            WriteShaderHeader(header, sourceName, dxil, "dxil", entry, 0, 0, 0);
            return;
        }

        if(format.Equals("msl", StringComparison.OrdinalIgnoreCase))
        {
            await RunCppslAsync(context.Workspace, source, intermediateDirectory, stage, entry, nativeExtractor, "msl,reflection", cancellationToken);

            var metal = Path.Combine(intermediateDirectory, sourceName + ".metal");
            if(!File.Exists(metal))
            {
                throw new MakeSystemException($"CPPSL did not produce expected Metal output: {metal}");
            }

            var metallib = await RunMetalAsync(context.Workspace, metal, intermediateDirectory, sourceName, IsDebug(payload.Required("mode")), cancellationToken);
            var (x, y, z) = ReadMetalNumthreads(Path.Combine(intermediateDirectory, sourceName + ".reflection.json"));
            WriteShaderHeader(header, sourceName, metallib, "metallib", entry, x, y, z);
            return;
        }

        throw new MakeSystemException($"CPPSL shader executor does not support requested format: {format}.");
    }

    private async Task RunCppslAsync(
        BuildWorkspace workspace,
        string source,
        string outputDirectory,
        string stage,
        string entry,
        string nativeExtractor,
        string target,
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
            target,
            "--native-extractor",
            nativeExtractor,
        };

        var result = await ProcessRunner.RunAsync(cppslc, args, workspace.RootDirectory, _actionTimeout, cancellationToken);
        if(result.ExitCode != 0)
        {
            throw new MakeSystemException(FormatToolFailure(
                $"CPPSL compile failed for {source}",
                workspace.RootDirectory,
                cppslc,
                args,
                result.Output));
        }
    }

    private async Task<string> RunMetalAsync(
        BuildWorkspace workspace,
        string metal,
        string intermediateDirectory,
        string sourceName,
        bool debug,
        CancellationToken cancellationToken)
    {
        var moduleCache = Path.Combine(workspace.BuildDirectory, ".metal-module-cache");
        Directory.CreateDirectory(moduleCache);

        var air = Path.Combine(intermediateDirectory, sourceName + ".air");
        var metallib = Path.Combine(intermediateDirectory, sourceName + ".metallib");
        var metalArgs = new List<string>
        {
            "metal",
            "-std=metal3.2",
            "-fmodules-cache-path=" + moduleCache,
            "-x",
            "metal",
            "-c",
            metal,
            "-o",
            air,
        };
        if(debug)
        {
            metalArgs.Add("-gline-tables-only");
            metalArgs.Add("-frecord-sources");
        }

        var metalResult = await ProcessRunner.RunAsync("xcrun", metalArgs, workspace.RootDirectory, _actionTimeout, cancellationToken);
        if(metalResult.ExitCode != 0)
        {
            throw new MakeSystemException(FormatToolFailure(
                $"Metal compile failed for {metal}",
                workspace.RootDirectory,
                "xcrun",
                metalArgs,
                metalResult.Output));
        }

        var metallibResult = await ProcessRunner.RunAsync(
            "xcrun",
            new[] { "metallib", air, "-o", metallib },
            workspace.RootDirectory,
            _actionTimeout,
            cancellationToken);
        if(metallibResult.ExitCode != 0)
        {
            throw new MakeSystemException(FormatToolFailure(
                $"Metal library link failed for {air}",
                workspace.RootDirectory,
                "xcrun",
                new[] { "metallib", air, "-o", metallib },
                metallibResult.Output));
        }
        return metallib;
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
            throw new MakeSystemException(FormatToolFailure(
                $"DXC compile failed for {hlsl}",
                workspace.RootDirectory,
                dxc,
                args,
                result.Output));
        }
    }

    private static string FormatToolFailure(string title, string workingDirectory, string tool, IReadOnlyList<string> arguments, string output)
    {
        var builder = new StringBuilder();
        builder.AppendLine(title);
        builder.AppendLine($"  working directory: {workingDirectory}");
        builder.AppendLine($"  tool: {tool}");
        builder.AppendLine("  arguments:");
        foreach(var argument in arguments.Take(256))
        {
            builder.AppendLine("    " + argument);
        }
        if(arguments.Count > 256)
        {
            builder.AppendLine($"    ... {arguments.Count - 256} more");
        }
        builder.AppendLine("  command line:");
        builder.AppendLine("    " + tool + " " + string.Join(' ', arguments.Select(QuoteForLog)));
        builder.AppendLine("  process output:");
        builder.Append((output.Length == 0 ? "<empty>" : output).TrimEnd());
        return builder.ToString();
    }

    private static string QuoteForLog(string value)
    {
        return value.Contains(' ') || value.Contains('\t') || value.Contains('"')
            ? "\"" + value.Replace("\"", "\\\"") + "\""
            : value;
    }

    private static string LocateNativeExtractor(BuildWorkspace workspace)
    {
        var executable = OperatingSystem.IsWindows() ? "cppsl-native-extractor.exe" : "cppsl-native-extractor";
        var candidates = new[]
        {
            Path.Combine(workspace.RootDirectory, "SDKs", "CPPSL", "macosx", "arm64", "bin", executable),
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
            Path.Combine(workspace.RootDirectory, "SDKs", "CPPSL", "macosx", "arm64", "bin", executable),
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
            Path.Combine(workspace.RootDirectory, "SDKs", "vulkan-tools", "macosx", "arm64", "bin", "dxc"),
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

    private static bool IsDebug(string mode)
    {
        return mode.Equals("Debug", StringComparison.OrdinalIgnoreCase);
    }

    private static (uint X, uint Y, uint Z) ReadMetalNumthreads(string reflectionPath)
    {
        if(!File.Exists(reflectionPath))
        {
            return (0, 0, 0);
        }

        using var document = JsonDocument.Parse(File.ReadAllText(reflectionPath));
        if(!TryFindWorkgroupSize(document.RootElement, out var values))
        {
            return (0, 0, 0);
        }
        return (
            values.Count > 0 ? values[0] : 0,
            values.Count > 1 ? values[1] : 0,
            values.Count > 2 ? values[2] : 0);
    }

    private static bool TryFindWorkgroupSize(JsonElement element, out List<uint> values)
    {
        if(element.ValueKind == JsonValueKind.Object)
        {
            foreach(var property in element.EnumerateObject())
            {
                if(property.NameEquals("WorkgroupSize") && property.Value.ValueKind == JsonValueKind.Array)
                {
                    values = property.Value.EnumerateArray()
                        .Where(item => item.ValueKind == JsonValueKind.Number)
                        .Select(item => item.GetUInt32())
                        .ToList();
                    return true;
                }
                if(TryFindWorkgroupSize(property.Value, out values))
                {
                    return true;
                }
            }
        }
        else if(element.ValueKind == JsonValueKind.Array)
        {
            foreach(var item in element.EnumerateArray())
            {
                if(TryFindWorkgroupSize(item, out values))
                {
                    return true;
                }
            }
        }

        values = new List<uint>();
        return false;
    }

    private static void WriteShaderHeader(
        string header,
        string sourceName,
        string binaryPath,
        string dataFormat,
        string entry,
        uint metalNumthreadsX,
        uint metalNumthreadsY,
        uint metalNumthreadsZ)
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
        builder.Append("    constexpr u32 SHADER_METAL_NUMTHREADS_X_").Append(sourceName).Append(" = ").Append(metalNumthreadsX).AppendLine(";");
        builder.Append("    constexpr u32 SHADER_METAL_NUMTHREADS_Y_").Append(sourceName).Append(" = ").Append(metalNumthreadsY).AppendLine(";");
        builder.Append("    constexpr u32 SHADER_METAL_NUMTHREADS_Z_").Append(sourceName).Append(" = ").Append(metalNumthreadsZ).AppendLine(";");
        builder.AppendLine("}");
        File.WriteAllText(header, builder.ToString());
    }
}
