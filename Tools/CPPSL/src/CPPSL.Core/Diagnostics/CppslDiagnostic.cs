namespace CPPSL.Core.Diagnostics;

public sealed record CppslDiagnostic(
    DiagnosticSeverity Severity,
    string Message,
    string? File = null,
    int? Line = null,
    int? Column = null)
{
    public static CppslDiagnostic Info(string message, string? file = null, int? line = null, int? column = null)
    {
        return new CppslDiagnostic(DiagnosticSeverity.Info, message, file, line, column);
    }

    public static CppslDiagnostic Error(string message, string? file = null, int? line = null, int? column = null)
    {
        return new CppslDiagnostic(DiagnosticSeverity.Error, message, file, line, column);
    }

    public string ToDisplayString()
    {
        var location = File is null ? string.Empty : File;
        if (Line is not null)
        {
            location += $":{Line}";
            if (Column is not null)
            {
                location += $":{Column}";
            }
        }
        if (!string.IsNullOrEmpty(location))
        {
            location += ": ";
        }
        return $"{location}{Severity.ToString().ToLowerInvariant()}: {Message}";
    }
}
