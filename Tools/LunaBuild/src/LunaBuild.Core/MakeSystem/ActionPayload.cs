namespace LunaBuild.Core.MakeSystem;

internal sealed class ActionPayload
{
    private readonly Dictionary<string, List<string>> _values = new(StringComparer.Ordinal);

    public static ActionPayload Parse(string payload)
    {
        var result = new ActionPayload();
        using var reader = new StringReader(payload);
        string? line;
        while((line = reader.ReadLine()) is not null)
        {
            line = line.Trim();
            if(line.Length == 0)
            {
                continue;
            }

            var separator = line.IndexOf('=');
            if(separator < 0)
            {
                result.Add(line, string.Empty);
                continue;
            }

            result.Add(line[..separator], line[(separator + 1)..]);
        }
        return result;
    }

    public string Required(string name)
    {
        if(_values.TryGetValue(name, out var values) && values.Count > 0)
        {
            return values[^1];
        }
        throw new MakeSystemException($"Action payload is missing required value `{name}`.");
    }

    public IReadOnlyList<string> All(string name)
    {
        return _values.TryGetValue(name, out var values) ? values : Array.Empty<string>();
    }

    public bool Contains(string name)
    {
        return _values.ContainsKey(name);
    }

    private void Add(string name, string value)
    {
        if(!_values.TryGetValue(name, out var values))
        {
            values = new List<string>();
            _values.Add(name, values);
        }
        values.Add(value);
    }
}
