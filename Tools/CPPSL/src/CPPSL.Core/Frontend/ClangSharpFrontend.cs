using ClangSharp.Interop;
using CPPSL.Core.Diagnostics;

namespace CPPSL.Core.Frontend;

public sealed class ClangSharpFrontend
{
    public ClangFrontendResult Parse(string sourcePath, IReadOnlyList<string> includeRoots)
    {
        var diagnostics = new List<CppslDiagnostic>();
        var declarations = new List<ClangDeclaration>();
        var commandLineArgs = BuildCommandLineArgs(includeRoots);

        var index = CXIndex.Create(excludeDeclarationsFromPch: false, displayDiagnostics: false);
        try
        {
            var translationUnit = CXTranslationUnit.Parse(
                index,
                sourcePath,
                commandLineArgs,
                Array.Empty<CXUnsavedFile>(),
                CXTranslationUnit_Flags.CXTranslationUnit_None);

            try
            {
                CollectDiagnostics(translationUnit, diagnostics);
                CollectTopLevelDeclarations(translationUnit.Cursor, declarations);
            }
            finally
            {
                translationUnit.Dispose();
            }
        }
        catch (Exception ex)
        {
            diagnostics.Add(CppslDiagnostic.Error($"ClangSharp failed to parse CPPSL source: {ex.Message}", sourcePath));
        }
        finally
        {
            index.Dispose();
        }

        var succeeded = !diagnostics.Any(static d => d.Severity == DiagnosticSeverity.Error);
        return new ClangFrontendResult(succeeded, diagnostics, declarations);
    }

    private static string[] BuildCommandLineArgs(IReadOnlyList<string> includeRoots)
    {
        var args = new List<string>
        {
            "-x",
            "c++",
            "-std=c++20",
            "-fsyntax-only",
            "-Wno-unknown-attributes",
            "-Wno-ignored-attributes",
            "-D__CPPSL__=1"
        };
        foreach (var includeRoot in includeRoots)
        {
            args.Add("-I" + includeRoot);
        }
        return args.ToArray();
    }

    private static void CollectDiagnostics(CXTranslationUnit translationUnit, List<CppslDiagnostic> diagnostics)
    {
        foreach (var clangDiagnostic in translationUnit.DiagnosticSet)
        {
            var severity = clangDiagnostic.Severity switch
            {
                CXDiagnosticSeverity.CXDiagnostic_Error or CXDiagnosticSeverity.CXDiagnostic_Fatal => DiagnosticSeverity.Error,
                CXDiagnosticSeverity.CXDiagnostic_Warning => DiagnosticSeverity.Warning,
                _ => DiagnosticSeverity.Info
            };

            var location = clangDiagnostic.Location;
            location.GetSpellingLocation(out var file, out var line, out var column, out _);
            var fileName = file.ToString();
            diagnostics.Add(new CppslDiagnostic(
                severity,
                clangDiagnostic.Spelling.ToString(),
                string.IsNullOrEmpty(fileName) ? null : fileName,
                checked((int)line),
                checked((int)column)));
        }
    }

    private static unsafe void CollectTopLevelDeclarations(CXCursor root, List<ClangDeclaration> declarations)
    {
        root.VisitChildren((cursor, _, _) =>
        {
            if (!IsInterestingDeclaration(cursor.kind))
            {
                return CXChildVisitResult.CXChildVisit_Continue;
            }

            cursor.Location.GetSpellingLocation(out var file, out var line, out var column, out _);
            var fileName = file.ToString();
            declarations.Add(new ClangDeclaration(
                cursor.kind.ToString(),
                cursor.Spelling.ToString(),
                cursor.DisplayName.ToString(),
                string.IsNullOrEmpty(fileName) ? null : fileName,
                checked((int)line),
                checked((int)column)));
            return CXChildVisitResult.CXChildVisit_Continue;
        }, default);
    }

    private static bool IsInterestingDeclaration(CXCursorKind kind)
    {
        return kind is
            CXCursorKind.CXCursor_Namespace or
            CXCursorKind.CXCursor_StructDecl or
            CXCursorKind.CXCursor_ClassDecl or
            CXCursorKind.CXCursor_EnumDecl or
            CXCursorKind.CXCursor_FunctionDecl or
            CXCursorKind.CXCursor_FunctionTemplate or
            CXCursorKind.CXCursor_ClassTemplate or
            CXCursorKind.CXCursor_VarDecl;
    }
}
