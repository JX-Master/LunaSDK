namespace LunaBuild.Core.Targets;

public sealed class CPPSLTargetRules : TargetRules
{
    public CPPSLTargetRules()
        : base(
            name: "CPPSL",
            targetDirectory: "Tools/CPPSL",
            rulesPath: "Tools/CPPSL/CPPSL.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.Windows);
        Kind = BuildTargetKind.DotNetProject;
        Sources("src/CPPSL.Cli/*.cs", "src/CPPSL.Core/**.cs");
        ExcludeSources("src/CPPSL.Core/bin/**.cs", "src/CPPSL.Core/obj/**.cs");
        DotNetProject("src/CPPSL.Cli/CPPSL.Cli.csproj", "src/CPPSL.Cli/bin/Debug/net9.0/cppslc.exe");
        DependsOn("cppsl-native-extractor");
    }
}
