using CPPSL.Core.Compiler;
using CPPSL.Core.Diagnostics;
using CPPSL.Core.IR;

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

        var compiler = new CppslCompiler();
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
        var includeRoots = new List<string>();
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
            stage);

        return new ParseResult(options, diagnostics);
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
        cppslc - CPPSL compiler prototype

        Usage:
          cppslc compile <shader.cxx> --stage <stage> --entry <name> --include <dir> --out <dir>

        Stages:
          vertex, fragment, pixel, compute, raygen, miss, closesthit, anyhit, intersection, callable
        """);
    }

    private sealed record ParseResult(CppslCompileOptions Options, List<CppslDiagnostic> Diagnostics);
}
