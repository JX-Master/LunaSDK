namespace LunaBuild.Core;

public abstract class ProjectRules
{
    private readonly List<BuildPropertyDefinition> _properties = new();
    private readonly List<string> _globalDefines = new();
    private readonly List<string> _globalUndefines = new();
    private BuildWorkspace? _currentWorkspace;
    private BuildOptions? _currentOptions;

    protected ProjectRules(string name)
    {
        Name = name;
    }

    public string Name { get; }

    protected BuildWorkspace Workspace => _currentWorkspace ?? throw new InvalidOperationException("ProjectRules workspace is only available during ConfigureProperties/Configure.");

    protected BuildOptions Options => _currentOptions ?? throw new InvalidOperationException("ProjectRules options are only available during Configure.");

    protected void BooleanProperty(string name, bool defaultValue = false, string description = "", params string[] commandLineNames)
    {
        AddProperty(name, BuildPropertyKind.Boolean, defaultValue ? "true" : "false", description, commandLineNames);
    }

    protected void StringProperty(string name, string defaultValue = "", string description = "", params string[] commandLineNames)
    {
        AddProperty(name, BuildPropertyKind.String, defaultValue, description, commandLineNames);
    }

    protected bool GetBoolean(string name) => Options.Properties.GetBoolean(name);

    protected string GetString(string name) => Options.Properties.GetString(name);

    protected void GlobalDefines(params string[] defines)
    {
        _globalDefines.AddRange(defines);
    }

    protected void GlobalUndefines(params string[] undefines)
    {
        _globalUndefines.AddRange(undefines);
    }

    protected virtual void ConfigureProperties(BuildWorkspace workspace)
    {
    }

    protected virtual void Configure(BuildWorkspace workspace, BuildOptions options)
    {
    }

    public BuildProjectDefinition ToDefinition(BuildWorkspace workspace)
    {
        _properties.Clear();
        _currentWorkspace = workspace;
        try
        {
            ConfigureProperties(workspace);
            return new BuildProjectDefinition(Name, _properties.ToArray());
        }
        finally
        {
            _currentWorkspace = null;
        }
    }

    public BuildOptions ConfigureBuildOptions(BuildWorkspace workspace, BuildOptions options)
    {
        _globalDefines.Clear();
        _globalUndefines.Clear();
        _currentWorkspace = workspace;
        _currentOptions = options;
        try
        {
            Configure(workspace, options);
            return options with
            {
                GlobalDefines = options.GlobalDefines
                    .Concat(_globalDefines)
                    .Distinct(StringComparer.Ordinal)
                    .Order(StringComparer.Ordinal)
                    .ToArray(),
                GlobalUndefines = options.GlobalUndefines
                    .Concat(_globalUndefines)
                    .Distinct(StringComparer.Ordinal)
                    .Order(StringComparer.Ordinal)
                    .ToArray(),
            };
        }
        finally
        {
            _currentWorkspace = null;
            _currentOptions = null;
        }
    }

    private void AddProperty(string name, BuildPropertyKind kind, string defaultValue, string description, IReadOnlyList<string> commandLineNames)
    {
        if(string.IsNullOrWhiteSpace(name))
        {
            throw new ArgumentException("Build property name cannot be empty.", nameof(name));
        }
        if(_properties.Any(property => property.Name.Equals(name, StringComparison.OrdinalIgnoreCase)))
        {
            throw new InvalidOperationException($"Project `{Name}` declares build property `{name}` more than once.");
        }

        _properties.Add(new BuildPropertyDefinition(
            name,
            kind,
            NormalizeValue(kind, name, defaultValue),
            description,
            commandLineNames
                .Prepend(name)
                .Select(NormalizeCommandLineName)
                .Distinct(StringComparer.OrdinalIgnoreCase)
                .Order(StringComparer.OrdinalIgnoreCase)
                .ToArray()));
    }

    internal static string NormalizeCommandLineName(string name)
    {
        return name.Trim().TrimStart('-').Replace('_', '-');
    }

    internal static string NormalizeValue(BuildPropertyKind kind, string name, string value)
    {
        return kind switch
        {
            BuildPropertyKind.Boolean => BuildProperties.ParseBoolean(value, name) ? "true" : "false",
            _ => value,
        };
    }
}

public sealed record BuildProjectDefinition(
    string Name,
    IReadOnlyList<BuildPropertyDefinition> Properties)
{
    public static BuildProjectDefinition Empty { get; } = new("Project", Array.Empty<BuildPropertyDefinition>());

    public BuildProperties ResolveProperties(IReadOnlyDictionary<string, string?> overrides)
    {
        var values = Properties.ToDictionary(
            property => property.Name,
            property => property.DefaultValue,
            StringComparer.OrdinalIgnoreCase);

        var commandLineMap = new Dictionary<string, BuildPropertyDefinition>(StringComparer.OrdinalIgnoreCase);
        foreach(var property in Properties)
        {
            foreach(var commandLineName in property.CommandLineNames)
            {
                if(commandLineMap.TryGetValue(commandLineName, out var existing))
                {
                    throw new InvalidOperationException(
                        $"Build properties `{existing.Name}` and `{property.Name}` both use command-line name `--{commandLineName}`.");
                }
                commandLineMap.Add(commandLineName, property);
            }
        }

        foreach(var (rawName, rawValue) in overrides)
        {
            var commandLineName = ProjectRules.NormalizeCommandLineName(rawName);
            if(!commandLineMap.TryGetValue(commandLineName, out var property))
            {
                throw new ArgumentException($"Unknown project build property: --{commandLineName}");
            }

            var value = rawValue ?? (property.Kind == BuildPropertyKind.Boolean
                ? "true"
                : throw new ArgumentException($"Project build property `--{commandLineName}` requires a value."));
            values[property.Name] = ProjectRules.NormalizeValue(property.Kind, property.Name, value);
        }

        return new BuildProperties(Properties
            .Select(property => new BuildPropertyValue(
                property.Name,
                values[property.Name],
                property.DefaultValue,
                property.Kind))
            .ToArray());
    }
}

