namespace LunaBuild.Core.Targets;

public sealed class IOSPackageSmokeTargetRules : TargetRules
{
    public IOSPackageSmokeTargetRules()
        : base(
            name: "IOSPackageSmoke",
            targetDirectory: "Tests/IOSPackageSmoke",
            rulesPath: "Tests/IOSPackageSmoke/IOSPackageSmoke.Target.cs")
    {
        SupportedPlatforms(BuildPlatform.IOS);
        Category = BuildTargetCategory.Tests;
        Kind = BuildTargetKind.Executable;
        Sources("Source/*.cpp");
        AppleBundle("com.lunasdk.IOSPackageSmoke", "IOSPackageSmoke");
        AppleInfoPlist("Source/Info.plist");
        AppleEntitlements("Source/IOSPackageSmoke.entitlements");
        DependsOn("Runtime", "Window");
    }
}
