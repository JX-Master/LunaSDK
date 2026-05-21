namespace LunaBuild.Core;

public enum BuildPropertyKind
{
    Boolean,
    String,
}

public sealed record BuildPropertyDefinition(
    string Name,
    BuildPropertyKind Kind,
    string DefaultValue,
    string Description,
    IReadOnlyList<string> CommandLineNames);

public sealed record BuildPropertyValue(
    string Name,
    string Value,
    string DefaultValue,
    BuildPropertyKind Kind)
{
    public bool IsDefault => string.Equals(Value, DefaultValue, StringComparison.Ordinal);
}

public sealed class BuildProperties
{
    public static BuildProperties Empty { get; } = new(Array.Empty<BuildPropertyValue>());

    private readonly IReadOnlyDictionary<string, BuildPropertyValue> _values;

    public BuildProperties(IReadOnlyList<BuildPropertyValue> values)
    {
        Values = values
            .OrderBy(value => value.Name, StringComparer.OrdinalIgnoreCase)
            .ToArray();
        _values = Values.ToDictionary(value => value.Name, StringComparer.OrdinalIgnoreCase);
    }

    public IReadOnlyList<BuildPropertyValue> Values { get; }

    public bool GetBoolean(string name)
    {
        if(!_values.TryGetValue(name, out var value))
        {
            throw new KeyNotFoundException($"Unknown build property: {name}");
        }
        if(value.Kind != BuildPropertyKind.Boolean)
        {
            throw new InvalidOperationException($"Build property `{name}` is not boolean.");
        }
        return ParseBoolean(value.Value, name);
    }

    public string GetString(string name)
    {
        if(!_values.TryGetValue(name, out var value))
        {
            throw new KeyNotFoundException($"Unknown build property: {name}");
        }
        return value.Value;
    }

    public bool TryGetBoolean(string name, out bool result)
    {
        if(!_values.TryGetValue(name, out var value) || value.Kind != BuildPropertyKind.Boolean)
        {
            result = false;
            return false;
        }
        result = ParseBoolean(value.Value, name);
        return true;
    }

    public IEnumerable<BuildPropertyValue> NonDefaultValues()
    {
        return Values.Where(value => !value.IsDefault);
    }

    internal static bool ParseBoolean(string value, string propertyName)
    {
        return value.ToLowerInvariant() switch
        {
            "true" or "1" or "yes" or "y" or "on" => true,
            "false" or "0" or "no" or "n" or "off" => false,
            _ => throw new ArgumentException($"Build property `{propertyName}` expects a boolean value, got `{value}`."),
        };
    }
}

