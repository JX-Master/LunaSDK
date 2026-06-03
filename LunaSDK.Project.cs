using LunaBuild.Core;

public sealed class LunaSDKProjectRules : ProjectRules
{
    public LunaSDKProjectRules()
        : base("LunaSDK")
    {
    }

    protected override void ConfigureProperties(BuildWorkspace workspace)
    {
        BooleanProperty(
            "api_validation",
            defaultValue: false,
            description: "Enable Luna public API validation checks. Debug mode enables this automatically.",
            "api-validation",
            "contract-assertion");
        BooleanProperty(
            "thread_safe_assertion",
            defaultValue: false,
            description: "Enable Luna thread-safety assertion checks.",
            "thread-safe-assertion");
        BooleanProperty(
            "memory_profiler",
            defaultValue: false,
            description: "Enable Luna runtime memory profiler instrumentation.",
            "memory-profiler");
        BooleanProperty(
            "rhi_debug",
            defaultValue: false,
            description: "Enable RHI backend debug layers and validation helpers.",
            "rhi-debug");
        BooleanProperty(
            "gui_debug",
            defaultValue: false,
            description: "Enable Luna GUI debug features.",
            "gui-debug");
        StringProperty(
            "apple_sdk",
            defaultValue: "iphoneos",
            description: "Apple SDK name for Apple-platform builds.",
            "apple-sdk");
        StringProperty(
            "ios_deployment_target",
            defaultValue: "13.0",
            description: "Minimum iOS deployment target version.",
            "ios-deployment-target");
        StringProperty(
            "ios_codesign_identity",
            defaultValue: "-",
            description: "Code signing identity for iOS app bundles. Use none to skip signing.",
            "ios-codesign-identity");
        StringProperty(
            "ios_provisioning_profile",
            defaultValue: "",
            description: "Provisioning profile to embed in iOS app bundles.",
            "ios-provisioning-profile");
    }

    protected override void Configure(BuildWorkspace workspace, BuildOptions options)
    {
        if(options.Mode == BuildMode.Debug || GetBoolean("api_validation"))
        {
            GlobalDefines("LUNA_ENABLE_API_VALIDATION");
        }
        if(GetBoolean("thread_safe_assertion"))
        {
            GlobalDefines("LUNA_ENABLE_THREAD_SAFE_ASSERTION");
        }
        if(GetBoolean("memory_profiler"))
        {
            GlobalDefines("LUNA_ENABLE_MEMORY_PROFILER");
        }
    }
}
