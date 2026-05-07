using CPPSL.Core.Compiler;
using CPPSL.Core.Diagnostics;
using CPPSL.Core.Frontend;
using CPPSL.Core.ShaderModel;
using CPPSL.Core.Artifacts;

namespace CPPSL.Cli;

public static class CppslCli
{
    public static int Run(string[] args)
    {
        if (args.Length == 0 || args[0] is "-h" or "--help" or "help")
        {
            PrintUsage();
            return 0;
        }
        if (args[0] != "compile")
        {
            Console.Error.WriteLine($"error: unknown command '{args[0]}'");
            PrintUsage();
            return 2;
        }

        var parsed = ParseCompile(args[1..]);
        if (parsed.Diagnostics.Count != 0)
        {
            WriteDiagnostics(parsed.Diagnostics);
            return 2;
        }

        var compiler = new CppslCompiler(CreateFrontend(parsed.NativeExtractorPath));
        var result = compiler.Compile(parsed.Options);
        WriteDiagnostics(result.Diagnostics);
        return result.Succeeded ? 0 : 1;
    }

    private static ParseResult ParseCompile(string[] args)
    {
        string? source = null;
        string? outputDirectory = null;
        string entryPoint = "main";
        ShaderStage stage = ShaderStage.Vertex;
        string? nativeExtractorPath = null;
        var includeRoots = new List<string>();
        var targets = new List<CppslOutputTarget>();
        var diagnostics = new List<CppslDiagnostic>();

        for (var i = 0; i < args.Length; ++i)
        {
            var arg = args[i];
            switch (arg)
            {
                case "--entry":
                    entryPoint = ReadValue(args, ref i, "--entry", diagnostics) ?? entryPoint;
                    break;
                case "--stage":
                    var stageValue = ReadValue(args, ref i, "--stage", diagnostics);
                    if (stageValue is not null && !Enum.TryParse(stageValue, ignoreCase: true, out stage))
                    {
                        diagnostics.Add(CppslDiagnostic.Error($"Unknown shader stage '{stageValue}'."));
                    }
                    break;
                case "--include":
                case "-I":
                    var include = ReadValue(args, ref i, arg, diagnostics);
                    if (include is not null)
                    {
                        includeRoots.Add(include);
                    }
                    break;
                case "--out":
                case "-o":
                    outputDirectory = ReadValue(args, ref i, arg, diagnostics);
                    break;
                case "--target":
                case "-t":
                    var targetValue = ReadValue(args, ref i, arg, diagnostics);
                    if (targetValue is not null)
                    {
                        AddTargets(targetValue, targets, diagnostics);
                    }
                    break;
                case "--native-extractor":
                    nativeExtractorPath = ReadValue(args, ref i, "--native-extractor", diagnostics) ?? nativeExtractorPath;
                    break;
                default:
                    if (arg.StartsWith('-'))
                    {
                        diagnostics.Add(CppslDiagnostic.Error($"Unknown option '{arg}'."));
                    }
                    else if (source is null)
                    {
                        source = arg;
                    }
                    else
                    {
                        diagnostics.Add(CppslDiagnostic.Error($"Unexpected positional argument '{arg}'."));
                    }
                    break;
            }
        }

        if (source is null)
        {
            diagnostics.Add(CppslDiagnostic.Error("Missing source `.cxx` file."));
        }
        if (outputDirectory is null)
        {
            diagnostics.Add(CppslDiagnostic.Error("Missing output directory. Use `--out <dir>`."));
        }
        if (includeRoots.Count == 0)
        {
            diagnostics.Add(CppslDiagnostic.Error("Missing CPPSL include root. Use `--include <dir>`."));
        }

        var options = new CppslCompileOptions(
            source ?? string.Empty,
            outputDirectory ?? string.Empty,
            includeRoots,
            entryPoint,
            stage,
            targets);

        return new ParseResult(options, diagnostics, nativeExtractorPath);
    }

    private static ICppslFrontend CreateFrontend(string? nativeExtractorPath)
    {
        return nativeExtractorPath is null
            ? new NativeExtractorFrontend()
            : new NativeExtractorFrontend(nativeExtractorPath);
    }

    private static string? ReadValue(string[] args, ref int index, string option, List<CppslDiagnostic> diagnostics)
    {
        if (index + 1 >= args.Length)
        {
            diagnostics.Add(CppslDiagnostic.Error($"Missing value for `{option}`."));
            return null;
        }
        return args[++index];
    }

    private static void AddTargets(string value, List<CppslOutputTarget> targets, List<CppslDiagnostic> diagnostics)
    {
        foreach (var part in value.Split(',', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries))
        {
            if (TryParseTarget(part, out var target))
            {
                targets.Add(target);
            }
            else
            {
                diagnostics.Add(CppslDiagnostic.Error($"Unknown output target '{part}'."));
            }
        }
    }

    private static bool TryParseTarget(string value, out CppslOutputTarget target)
    {
        if (value.Equals("reflect", StringComparison.OrdinalIgnoreCase))
        {
            target = CppslOutputTarget.Reflection;
            return true;
        }
        return Enum.TryParse(value, ignoreCase: true, out target);
    }

    private static void WriteDiagnostics(IEnumerable<CppslDiagnostic> diagnostics)
    {
        foreach (var diagnostic in diagnostics)
        {
            Console.Error.WriteLine(diagnostic.ToDisplayString());
        }
    }

    private static void PrintUsage()
    {
        Console.WriteLine("""
        cppslc - CPPSL shader compiler

        Usage:
          cppslc compile <shader.cxx> --stage <stage> --entry <name> --include <dir> --out <dir> [--target <target>] [--native-extractor <path>]

        Stages:
          vertex, fragment, pixel, compute, raygen, miss, closesthit, anyhit, intersection, callable

        Targets:
          hlsl, glsl, msl, reflection. Pass --target multiple times or use comma-separated values.
          If omitted, all targets are emitted from one frontend parse.

        Native extractor:
          Use --native-extractor <path> to override the default executable.
        """);
    }

    private sealed record ParseResult(
        CppslCompileOptions Options,
        List<CppslDiagnostic> Diagnostics,
        string? NativeExtractorPath);
}
