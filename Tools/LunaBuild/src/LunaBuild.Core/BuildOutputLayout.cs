namespace LunaBuild.Core;

public static class BuildOutputLayout
{
    public static string ConfigurationDirectory(BuildWorkspace workspace, BuildOptions options)
    {
        return Path.Combine(new[] { workspace.BuildDirectory }.Concat(ConfigurationSegments(options)).ToArray());
    }

    public static IReadOnlyList<string> ConfigurationSegments(BuildOptions options)
    {
        var segments = new List<string>
        {
            options.Platform.ToString(),
        };
        if(options.Platform == BuildPlatform.IOS)
        {
            segments.Add(AppleSdkName(options));
        }
        segments.Add(options.Architecture);
        segments.Add(options.Mode.ToString());
        return segments;
    }

    public static string AppleSdkName(BuildOptions options)
    {
        if(options.Platform == BuildPlatform.MacOS)
        {
            return "macosx";
        }
        if(options.Platform != BuildPlatform.IOS)
        {
            throw new ArgumentException($"Apple SDK is not defined for platform {options.Platform}.");
        }

        var rawValue = options.Apple.SdkName;
        return rawValue.Replace("-", string.Empty, StringComparison.Ordinal).ToLowerInvariant() switch
        {
            "" or "ios" or "iphoneos" or "device" => "iphoneos",
            "simulator" or "iossimulator" or "iphonesimulator" => "iphonesimulator",
            _ => throw new ArgumentException($"Unsupported Apple SDK `{rawValue}`. Expected iphoneos or iphonesimulator."),
        };
    }
}
