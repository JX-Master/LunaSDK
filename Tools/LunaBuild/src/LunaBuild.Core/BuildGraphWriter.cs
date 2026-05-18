using System.Text.Json;
using System.Text.Json.Serialization;

namespace LunaBuild.Core;

public static class BuildGraphWriter
{
    private static readonly JsonSerializerOptions JsonOptions = new()
    {
        WriteIndented = true,
        Converters =
        {
            new JsonStringEnumConverter(),
        },
    };

    public static void Write(BuildGraph graph, string outputPath)
    {
        WriteJson(graph, outputPath);
    }

    public static void WriteJson(BuildGraph graph, string outputPath)
    {
        var directory = Path.GetDirectoryName(Path.GetFullPath(outputPath));
        if(!string.IsNullOrEmpty(directory))
        {
            Directory.CreateDirectory(directory);
        }

        var json = JsonSerializer.Serialize(graph, JsonOptions);
        File.WriteAllText(outputPath, json);
    }
}
