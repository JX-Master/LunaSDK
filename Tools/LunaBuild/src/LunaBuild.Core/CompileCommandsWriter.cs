using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;
using LunaBuild.Core.MakeSystem;

namespace LunaBuild.Core;

public static class CompileCommandsWriter
{
    private static readonly JsonSerializerOptions JsonOptions = new()
    {
        WriteIndented = true,
    };

    public static void Write(BuildWorkspace workspace, BuildGraph graph, string outputPath)
    {
        var directory = Path.GetDirectoryName(Path.GetFullPath(outputPath));
        if(!string.IsNullOrEmpty(directory))
        {
            Directory.CreateDirectory(directory);
        }

        var appleToolchain = LocateAppleToolchain(graph.Options);
        var entries = graph.Nodes
            .Where(node => node.Command is not null && BuildActionKind.Extract(node.Command) == "cpp.compile")
            .Select(node => CreateEntry(workspace, graph, node, appleToolchain))
            .Where(entry => entry is not null)
            .OrderBy(entry => entry!.File, StringComparer.OrdinalIgnoreCase)
            .ToArray();

        File.WriteAllText(outputPath, JsonSerializer.Serialize(entries, JsonOptions), new UTF8Encoding(false));
    }

    private static AppleClangToolchain? LocateAppleToolchain(BuildOptions options)
    {
        if(options.Platform != BuildPlatform.MacOS || !OperatingSystem.IsMacOS())
        {
            return null;
        }
        return AppleClangToolchainLocator.LocateMacOS();
    }

    private static CompileCommandEntry? CreateEntry(BuildWorkspace workspace, BuildGraph graph, BuildGraphNode node, AppleClangToolchain? appleToolchain)
    {
        var payload = ActionPayload.Parse(node.Command!);
        var language = payload.Required("language");
        if(language.Equals("assembler", StringComparison.OrdinalIgnoreCase) ||
            language.Equals("assembler-with-cpp", StringComparison.OrdinalIgnoreCase))
        {
            return null;
        }

        var command = CppCommandLineBuilder.BuildCompileCommand(
            workspace,
            graph.Options,
            node.Command!,
            appleClangPath: appleToolchain?.Clang,
            appleClangCxxPath: appleToolchain?.ClangCxx,
            appleSdkPath: appleToolchain?.SdkPath).ToShellCommand();
        return new CompileCommandEntry(
            Directory: workspace.RootDirectory,
            File: workspace.ResolveRepositoryPath(payload.Required("source")),
            Command: command);
    }

    private sealed record CompileCommandEntry(
        [property: JsonPropertyName("directory")]
        string Directory,
        [property: JsonPropertyName("file")]
        string File,
        [property: JsonPropertyName("command")]
        string Command);
}
