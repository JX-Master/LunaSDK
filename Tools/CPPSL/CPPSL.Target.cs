namespace LunaBuild.Core.Targets;

public sealed class CPPSLTargetRules : TargetRules
{
    public CPPSLTargetRules()
        : base(
            name: "CPPSL",
            targetDirectory: "Tools/CPPSL",
            rulesPath: "Tools/CPPSL/CPPSL.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.MacOS, BuildPlatform.Windows);
        Category = BuildTargetCategory.Tools;
        Kind = BuildTargetKind.DotNetProject;
        Sources("src/CPPSL.Cli/*.cs", "src/CPPSL.Core/**.cs");
        ExcludeSources("src/CPPSL.Core/bin/**.cs", "src/CPPSL.Core/obj/**.cs");
        DependsOn("cppsl-native-extractor");
    }

    protected override void Configure(BuildWorkspace workspace, BuildOptions options)
    {
        var executable = options.Platform == BuildPlatform.Windows ? "cppslc.exe" : "cppslc";
        DotNetProject(
            "src/CPPSL.Cli/CPPSL.Cli.csproj",
            Path.Combine("src", "CPPSL.Cli", "bin", "Debug", "net9.0", executable));
    }
}
