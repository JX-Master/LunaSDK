namespace LunaBuild.Core;

public static class BuildActionKind
{
    public static string Extract(string? command)
    {
        if(string.IsNullOrWhiteSpace(command))
        {
            return "opaque";
        }

        using var reader = new StringReader(command);
        string? line;
        while((line = reader.ReadLine()) is not null)
        {
            line = line.Trim();
            if(line.Length == 0)
            {
                continue;
            }

            const string prefix = "kind=";
            if(line.StartsWith(prefix, StringComparison.Ordinal))
            {
                var kind = line[prefix.Length..].Trim();
                return kind.Length == 0 ? "opaque" : kind;
            }
            break;
        }
        return "opaque";
    }
}
