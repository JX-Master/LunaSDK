using ClangSharp.Interop;
using CPPSL.Core.Diagnostics;

namespace CPPSL.Core.Frontend;

public sealed class ClangSharpFrontend : ICppslFrontend
{
    public const string ProviderName = "ClangSharp";

    public CppslFrontendResult Parse(CppslFrontendOptions options)
    {
        var diagnostics = new List<CppslDiagnostic>();
        var declarations = new List<CppslDeclaration>();
        var astNodes = new List<CppslAstNode>();
        var commandLineArgs = BuildCommandLineArgs(options.IncludeRoots);

        var index = CXIndex.Create(excludeDeclarationsFromPch: false, displayDiagnostics: false);
        try
        {
            var translationUnit = CXTranslationUnit.Parse(
                index,
                options.SourcePath,
                commandLineArgs,
                Array.Empty<CXUnsavedFile>(),
                CXTranslationUnit_Flags.CXTranslationUnit_None);

            try
            {
                CollectDiagnostics(translationUnit, diagnostics);
                CollectAstNodes(translationUnit.Cursor, astNodes);
                CollectTopLevelDeclarations(astNodes, declarations);
            }
            finally
            {
                translationUnit.Dispose();
            }
        }
        catch (Exception ex)
        {
            diagnostics.Add(CppslDiagnostic.Error($"ClangSharp failed to parse CPPSL source: {ex.Message}", options.SourcePath));
        }
        finally
        {
            index.Dispose();
        }

        var succeeded = !diagnostics.Any(static d => d.Severity == DiagnosticSeverity.Error);
        return new CppslFrontendResult(succeeded, ProviderName, diagnostics, declarations, astNodes);
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

    private static void CollectTopLevelDeclarations(IReadOnlyList<CppslAstNode> astNodes, List<CppslDeclaration> declarations)
    {
        foreach (var node in astNodes)
        {
            if (!IsInterestingDeclaration(node.Kind))
            {
                continue;
            }

            declarations.Add(new CppslDeclaration(
                node.Kind,
                node.Spelling,
                node.DisplayName,
                node.File,
                node.Line,
                node.Column));
        }
    }

    private static unsafe void CollectAstNodes(CXCursor root, List<CppslAstNode> nodes)
    {
        root.VisitChildren((cursor, _, _) =>
        {
            if (!ShouldRecordNode(cursor.kind))
            {
                return CXChildVisitResult.CXChildVisit_Recurse;
            }

            nodes.Add(CreateNode(cursor));
            return CXChildVisitResult.CXChildVisit_Continue;
        }, default);
    }

    private static unsafe CppslAstNode CreateNode(CXCursor cursor)
    {
        var children = new List<CppslAstNode>();
        cursor.VisitChildren((child, _, _) =>
        {
            if (!ShouldRecordNode(child.kind))
            {
                return CXChildVisitResult.CXChildVisit_Recurse;
            }

            children.Add(CreateNode(child));
            return CXChildVisitResult.CXChildVisit_Continue;
        }, default);

        cursor.Location.GetSpellingLocation(out var file, out var line, out var column, out _);
        var fileName = file.ToString();
        return new CppslAstNode(
            cursor.kind.ToString(),
            cursor.Spelling.ToString(),
            cursor.DisplayName.ToString(),
            GetTypeSpelling(cursor),
            GetResultTypeSpelling(cursor),
            string.IsNullOrEmpty(fileName) ? null : fileName,
            checked((int)line),
            checked((int)column),
            CollectAttributes(cursor),
            children);
    }

    private static IReadOnlyList<CppslFrontendAttribute> CollectAttributes(CXCursor cursor)
    {
        if (!cursor.HasAttrs)
        {
            return Array.Empty<CppslFrontendAttribute>();
        }

        var attributes = new List<CppslFrontendAttribute>();
        for (var i = 0; i < cursor.NumAttrs; ++i)
        {
            var attr = cursor.GetAttr((uint)i);
            attr.Location.GetSpellingLocation(out var file, out var line, out var column, out _);
            var fileName = file.ToString();
            attributes.Add(new CppslFrontendAttribute(
                attr.kind.ToString(),
                attr.Spelling.ToString(),
                attr.DisplayName.ToString(),
                string.IsNullOrEmpty(fileName) ? null : fileName,
                checked((int)line),
                checked((int)column)));
        }
        return attributes;
    }

    private static bool IsInterestingDeclaration(string kind)
    {
        return kind is
            "CXCursor_Namespace" or
            "CXCursor_StructDecl" or
            "CXCursor_ClassDecl" or
            "CXCursor_EnumDecl" or
            "CXCursor_FunctionDecl" or
            "CXCursor_FunctionTemplate" or
            "CXCursor_ClassTemplate" or
            "CXCursor_VarDecl";
    }

    private static bool ShouldRecordNode(CXCursorKind kind)
    {
        return kind is
            CXCursorKind.CXCursor_Namespace or
            CXCursorKind.CXCursor_StructDecl or
            CXCursorKind.CXCursor_ClassDecl or
            CXCursorKind.CXCursor_EnumDecl or
            CXCursorKind.CXCursor_FieldDecl or
            CXCursorKind.CXCursor_FunctionDecl or
            CXCursorKind.CXCursor_FunctionTemplate or
            CXCursorKind.CXCursor_ClassTemplate or
            CXCursorKind.CXCursor_VarDecl or
            CXCursorKind.CXCursor_ParmDecl or
            CXCursorKind.CXCursor_Constructor or
            CXCursorKind.CXCursor_CXXMethod;
    }

    private static string? GetTypeSpelling(CXCursor cursor)
    {
        var spelling = cursor.Type.Spelling.ToString();
        return string.IsNullOrEmpty(spelling) ? null : spelling;
    }

    private static string? GetResultTypeSpelling(CXCursor cursor)
    {
        if (cursor.kind is not (CXCursorKind.CXCursor_FunctionDecl or CXCursorKind.CXCursor_FunctionTemplate or CXCursorKind.CXCursor_CXXMethod))
        {
            return null;
        }

        var spelling = cursor.ResultType.Spelling.ToString();
        return string.IsNullOrEmpty(spelling) ? null : spelling;
    }
}
