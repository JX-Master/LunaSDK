namespace LunaBuild.Core.Targets;

public sealed class WindowTargetRules : TargetRules
{
    public WindowTargetRules()
        : base(
            name: "Window",
            targetDirectory: "Modules/Luna/Window",
            rulesPath: "Modules/Luna/Window/Window.Target.cs")
    {
        Headers(
            "*.hpp",
            "Source/*.hpp");
        MetaHeaders("Event.hpp");

        Sources("Source/*.cpp");

        DependsOn("Runtime");
    }

    protected override void Configure(BuildWorkspace workspace, BuildOptions options)
    {
        if(Platform == BuildPlatform.Windows)
        {
            Headers("Windows/*.hpp", "Source/Backend/Windows/*.hpp");
            MetaHeaders("Source/Backend/Windows/Window.hpp");
            Sources("Source/Backend/Windows/*.cpp", "Source/Windows/*.cpp");
        }
        else if(Platform == BuildPlatform.MacOS)
        {
            Headers("Cocoa/*.hpp", "Source/Backend/Cocoa/*.h");
            MetaHeaders("Source/Backend/Cocoa/Window.h");
            Sources("Source/Backend/Cocoa/*.mm", "Source/Cocoa/*.mm");
            Frameworks("AppKit", "UniformTypeIdentifiers");
        }
        else if(Platform == BuildPlatform.IOS)
        {
            Headers("UIKit/*.hpp", "Source/Backend/UIKit/*.h");
            MetaHeaders("Source/Backend/UIKit/Window.h");
            Sources("Source/Backend/UIKit/*.mm", "Source/UIKit/*.mm");
            Frameworks("UIKit", "CoreGraphics", "QuartzCore");
        }
        else if(Platform == BuildPlatform.Android)
        {
            Headers("Android/**.hpp", "Android/**.h", "Source/Backend/Android/*.hpp");
            MetaHeaders("Source/Backend/Android/Window.hpp");
            Sources("Source/Backend/Android/*.cpp");
        }
    }
}
