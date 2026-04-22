using System.Text.RegularExpressions;
using CPPSL.Core.Frontend;

namespace CPPSL.Core.Semantics;

public sealed partial class CppslAttributeParser
{
    private readonly Dictionary<string, string[]> _sourceLines = new(StringComparer.Ordinal);

    public IReadOnlyList<CppslAttribute> GetAttributes(CppslAstNode node, bool includeLeadingLines = true)
    {
        if (node.File is null || node.Line is null)
        {
            return Array.Empty<CppslAttribute>();
        }

        var lines = GetSourceLines(node.File);
        var lineIndex = node.Line.Value - 1;
        if (lineIndex < 0 || lineIndex >= lines.Length)
        {
            return Array.Empty<CppslAttribute>();
        }

        var chunks = new List<(string Text, int Line, int ColumnBase)>();

        var currentLine = lines[lineIndex];
        var prefixLength = node.Column is null
            ? currentLine.Length
            : Math.Clamp(node.Column.Value - 1, 0, currentLine.Length);
        var currentPrefix = currentLine[..prefixLength];
        if (currentPrefix.Contains("[[", StringComparison.Ordinal))
        {
            chunks.Add((currentPrefix, lineIndex + 1, 1));
        }

        if (!includeLeadingLines)
        {
            return chunks.SelectMany(chunk => ParseChunk(chunk.Text, node.File, chunk.Line, chunk.ColumnBase)).ToArray();
        }

        for (var i = lineIndex - 1; i >= 0; --i)
        {
            var trimmed = lines[i].Trim();
            if (trimmed.Length == 0)
            {
                continue;
            }
            if (!IsStandaloneAttributeLine(trimmed))
            {
                break;
            }

            chunks.Add((lines[i], i + 1, 1));
        }

        chunks.Reverse();
        return chunks.SelectMany(chunk => ParseChunk(chunk.Text, node.File, chunk.Line, chunk.ColumnBase)).ToArray();
    }

    private string[] GetSourceLines(string path)
    {
        path = Path.GetFullPath(path);
        if (!_sourceLines.TryGetValue(path, out var lines))
        {
            lines = File.ReadAllLines(path);
            _sourceLines.Add(path, lines);
        }
        return lines;
    }

    private static IEnumerable<CppslAttribute> ParseChunk(string text, string file, int line, int columnBase)
    {
        foreach (Match blockMatch in AttributeBlockRegex().Matches(text))
        {
            var blockText = blockMatch.Groups["body"].Value;
            foreach (Match attrMatch in CppslAttributeRegex().Matches(blockText))
            {
                var argsText = attrMatch.Groups["args"].Success ? attrMatch.Groups["args"].Value : string.Empty;
                var args = argsText.Length == 0
                    ? Array.Empty<string>()
                    : argsText.Split(',').Select(static arg => arg.Trim()).Where(static arg => arg.Length != 0).ToArray();

                yield return new CppslAttribute(
                    attrMatch.Groups["name"].Value,
                    args,
                    file,
                    line,
                    columnBase + blockMatch.Index + attrMatch.Index);
            }
        }
    }

    private static bool IsStandaloneAttributeLine(string trimmedLine)
    {
        if (!trimmedLine.StartsWith("[[", StringComparison.Ordinal))
        {
            return false;
        }

        var withoutBlocks = AttributeBlockRegex().Replace(trimmedLine, string.Empty).Trim();
        return withoutBlocks.Length == 0;
    }

    [GeneratedRegex("\\[\\[(?<body>.*?)\\]\\]")]
    private static partial Regex AttributeBlockRegex();

    [GeneratedRegex("cppsl::(?<name>[A-Za-z_][A-Za-z0-9_]*)(?:\\((?<args>[^)]*)\\))?")]
    private static partial Regex CppslAttributeRegex();
}
