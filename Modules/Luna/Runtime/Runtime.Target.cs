namespace LunaBuild.Core.Targets;

public sealed class RuntimeTargetRules : TargetRules
{
    public RuntimeTargetRules()
        : base(
            name: "Runtime",
            targetDirectory: "Modules/Luna/Runtime",
            rulesPath: "Modules/Luna/Runtime/Runtime.Target.cs")
    {
        Headers(
            "*.hpp",
            "Impl/**.hpp",
            "Impl/**.inl",
            "Math/**.hpp",
            "Math/**.inl",
            "Source/**.hpp");

        Sources("Source/*.cpp");
    }

    protected override void Configure(BuildWorkspace workspace, BuildOptions options)
    {
        if(Platform == BuildPlatform.Windows)
        {
            Headers("Platform/Windows/**.hpp", "Source/Platform/Windows/*.hpp");
            Sources("Source/Platform/Windows/*.cpp");
        }
        else if(Platform is BuildPlatform.Linux or BuildPlatform.MacOS or BuildPlatform.Android or BuildPlatform.IOS)
        {
            Headers("Source/Platform/POSIX/*.hpp", "Source/Platform/POSIX/FiberContextBase.h");
            Sources("Source/Platform/POSIX/*.cpp");

            if(Architecture is "arm64" or "aarch64" or "arm64-v8a")
            {
                Headers("Source/Platform/POSIX/FiberContext_arm64.h");
                Sources("Source/Platform/POSIX/FiberContext_arm64.S");
            }
            else if(Architecture is "x86_64" or "x64")
            {
                Headers("Source/Platform/POSIX/FiberContext_x86_64.h");
                Sources("Source/Platform/POSIX/FiberContext_x86_64.S");
            }
            else if(Architecture is "i386" or "x86")
            {
                Headers("Source/Platform/POSIX/FiberContext_x86.h");
                Sources("Source/Platform/POSIX/FiberContext_x86.S");
            }
            else if(Architecture is "arm" or "armv7" or "armv7k" or "armeabi-v7a")
            {
                Headers("Source/Platform/POSIX/FiberContext_arm.h");
                Sources("Source/Platform/POSIX/FiberContext_arm.S");
            }

            if(Platform is BuildPlatform.Linux or BuildPlatform.MacOS or BuildPlatform.IOS)
            {
                SystemLibraries("pthread");
            }
        }
    }
}
