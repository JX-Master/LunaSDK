using System;
using System.Collections.Generic;

namespace Luna.Window;

public sealed class FileDialogFilter
{
    private readonly string[] _extensions;

    public FileDialogFilter(string name, params string[] extensions)
    {
        ArgumentException.ThrowIfNullOrEmpty(name);
        ArgumentNullException.ThrowIfNull(extensions);
        if (extensions.Length == 0)
        {
            throw new ArgumentException("At least one extension is required.", nameof(extensions));
        }

        Name = name;
        _extensions = new string[extensions.Length];
        for (var i = 0; i < extensions.Length; ++i)
        {
            _extensions[i] = NormalizeExtension(extensions[i]);
        }
    }

    public string Name { get; }

    public IReadOnlyList<string> Extensions => _extensions;

    private static string NormalizeExtension(string extension)
    {
        ArgumentException.ThrowIfNullOrEmpty(extension);
        extension = extension.Trim();
        while (extension.StartsWith(".", StringComparison.Ordinal))
        {
            extension = extension[1..];
        }
        if (extension.Length == 0)
        {
            throw new ArgumentException("Extension cannot be empty.", nameof(extension));
        }
        return extension;
    }
}
