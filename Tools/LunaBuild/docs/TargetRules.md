# LunaBuild Target Rules

`*.Target.cs` files describe LunaBuild targets for a project. LunaBuild Core
is project-agnostic: it scans the repository for `*.Target.cs`, compiles those
files into a temporary rules assembly, and loads concrete `TargetRules` types.

## Location

Place each target rule file next to the target's project files. During the
xmake migration this means the same directory as the target's `xmake.lua`.

Examples:

```text
Modules/Luna/Runtime/Runtime.Target.cs
Modules/Luna/RHI/RHI.Target.cs
Tests/RHITests/RHITest4_Box/RHITest4_Box.Target.cs
```

Do not place SDK project rules under `Tools/LunaBuild`. That directory is for
the generic build system implementation.

## Naming

- File name: `<TargetName>.Target.cs`.
- Class name: descriptive and unique, usually `<TargetName>TargetRules`.
- Namespace: `LunaBuild.Core.Targets`.
- Target name: use `target`, not module terminology.

The target name in the constructor is the canonical name used by
`--target <name>` and dependency declarations.

## Minimal Shape

```csharp
namespace LunaBuild.Core.Targets;

public sealed class ExampleTargetRules : TargetRules
{
    public ExampleTargetRules()
        : base(
            name: "Example",
            targetDirectory: "Modules/Luna/Example",
            rulesPath: "Modules/Luna/Example/Example.Target.cs")
    {
        Headers("*.hpp", "Source/**.hpp");
        Sources("Source/**.cpp");
        DependsOn("Runtime");
    }
}
```

All paths are repository-relative unless they are source/header/runtime file
patterns, which are relative to `targetDirectory`.

## Target Kind

Default kind is `BuildTargetKind.SharedLibrary`.

Set `Kind` only when the target is not a normal shared library:

```csharp
Kind = BuildTargetKind.Executable;
```

Current common kinds:

- `SharedLibrary`: Luna SDK modules such as `Runtime`, `RHI`, `Image`.
- `Executable`: tests and programs that produce `.exe` on Windows.
- `DotNetProject`: C# tool targets built through a `dotnet.build` action.
- `External`: prebuilt or header-only third-party dependencies consumed through
  existing files.

## Target Category

Targets are grouped into build categories. `Engine` targets are built by
default. `Tests` and `Tools` targets are built only when explicitly requested by
category or target name.

```csharp
Category = BuildTargetCategory.Tools;
```

Use `Tests` for all test targets, including test helper libraries such as
`RHITestBed`. Do not infer test-ness from `Executable`, because some test
support targets are libraries.

```csharp
Category = BuildTargetCategory.Tests;
```

Use `Tools` for build-time tools such as CPPSL. Tool targets remain visible to
IDE generators and can still be built with `--target <name>` or
`--category Tools`, but they are not part of the normal SDK `--all` build.

## Sources And Headers

Use explicit patterns. `**` is recursive.

```csharp
Headers("*.hpp", "Source/**.hpp");
Sources("Source/*.cpp", "Source/Platform/Windows/*.cpp");
ExcludeSources("Source/Experimental/*.cpp");
```

Windows resource files can be listed in `Sources(...)` too:

```csharp
Sources("Source/*.cpp", "Source/Windows/*.rc");
```

LunaBuild compiles `.rc` files with `rc.compile` and links the resulting `.res`
file into the target.

Patterns are intentionally simple. Avoid encoding complex build logic in glob
shape; use `Configure(...)` for platform, architecture, or option branches.

## Platform And Architecture Selection

Override `Configure(...)` when a target has different source files per
platform. Use normal C# `if` / `else` and call the same `Headers(...)`,
`Sources(...)`, `ExcludeSources(...)`, `Defines(...)`, and dependency helpers
used by unconditional rules:

```csharp
public sealed class ExampleTargetRules : TargetRules
{
    public ExampleTargetRules()
        : base("Example", "Modules/Luna/Example", "Modules/Luna/Example/Example.Target.cs")
    {
        Headers("*.hpp", "Source/*.hpp");
        Sources("Source/*.cpp");
        DependsOn("Runtime");
    }

    protected override void Configure(BuildWorkspace workspace, BuildOptions options)
    {
        if(Platform == BuildPlatform.Windows)
        {
            Sources("Source/Platform/Windows/*.cpp");
        }
        else if(Platform is BuildPlatform.Linux or BuildPlatform.MacOS or BuildPlatform.Android or BuildPlatform.IOS)
        {
            Headers("Source/Platform/POSIX/*.hpp");
            Sources("Source/Platform/POSIX/*.cpp");
        }
    }
}
```

Use `Architecture` inside the same method for files such as POSIX fiber
context assembly:

```csharp
if(Architecture is "x86_64" or "x64")
{
    Sources("Source/Platform/POSIX/FiberContext_x86_64.S");
}
else if(Architecture is "arm64" or "aarch64" or "arm64-v8a")
{
    Sources("Source/Platform/POSIX/FiberContext_arm64.S");
}
```

Use `SupportedPlatforms(...)` when the whole target should only exist on a
subset of platforms:

```csharp
SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
```

`inspect` and graph generation filter unsupported targets before dependency
resolution.

## Includes, Defines, External Targets, Frameworks

```csharp
IncludeDirectories("Include", "ThirdParty/Headers");
Defines("LUNA_CUSTOM_DEFINE=1");
```

LunaBuild is only a build system. It does not download packages, maintain SDK
roots, or resolve package registries. Put SDK path helpers and platform-specific
path composition in project rule files, then pass final paths to target APIs.

Use `BuildTargetKind.External` for third-party dependencies that already exist
on disk:

```csharp
public sealed class StbTargetRules : TargetRules
{
    public StbTargetRules()
        : base("stb", ".", "ThirdParty.Target.cs")
    {
        Kind = BuildTargetKind.External;
        PublicIncludeDirectories("SDKs/stb/include");
        RequiredFiles("SDKs/stb/include/stb/stb_image.h");
    }
}
```

External targets do not compile sources or produce link actions. They only
validate declared files and propagate public usage requirements through the same
dependency graph as normal targets:

```csharp
PublicIncludeDirectories("SDKs/example/include");
PublicDefines("EXAMPLE_USE_STATIC=1");
LinkLibraryFiles("SDKs/example/lib/example.lib");
Frameworks("Metal");
RuntimeFiles("SDKs/example/bin/example.dll");
RequiredFiles("SDKs/example/include/example.hpp");
```

`Packages(...)` is intentionally disabled. Migrate packages to external targets
and use `DependsOn(...)` from ordinary targets.

For native tools that link SDK-local libraries directly, use explicit link
inputs and system libraries:

```csharp
Defines("NDEBUG", "_ITERATOR_DEBUG_LEVEL=0");
Undefines("_DEBUG");
MsvcRuntimeLibrary("MD");
LinkLibraryFiles("ThirdParty/lib/example.lib");
SystemLibraries("advapi32.lib", "user32.lib");
Frameworks("AppKit", "Metal");
```

Use this for toolchain-style targets only. Luna SDK modules should prefer
normal target dependencies and external target declarations.

Use `Frameworks(...)` for Apple platform frameworks that must be passed to the
linker as `-framework <name>`.

## Dependencies

Use `DependsOn(...)` for target dependencies:

```csharp
DependsOn("Runtime", "RHI", "Window");
```

Link inputs are transitive. If target `A` depends on `B`, and `B` depends on
`Window`, executable `A` receives both `B` and `Window` import libraries.
Public include directories, public defines, frameworks, and runtime files from
external targets propagate through the same dependency closure.

## Shaders

Use `Shader(sourceFile, stage, entryPoint)` for CPPSL shader files:

```csharp
Shader("TestBoxVS.cxx", "vertex", "vs_main");
Shader("TestBoxPS.cxx", "pixel", "ps_main");
```

The generated headers are placed under:

```text
build/LunaBuild/<Platform>/<Arch>/<Mode>/generated/<Target>/shaders
```

Shader targets should include generated headers by name:

```cpp
#include <TestBoxVS.hpp>
```

The current executor supports Windows/D3D12 DXIL output first.

## Runtime Files

Use `RuntimeFiles(...)` for files that must be copied next to the produced
binary:

```csharp
RuntimeFiles("luna.png");
```

LunaBuild emits `file.copy` graph nodes for these files. They are incremental:
if the source file changes, the runtime copy is rebuilt.

## Embedded Headers

Use `EmbeddedHeader(...)` when a binary asset should be compiled into a C++
header:

```csharp
EmbeddedHeader("Res/luna.png", "LunaTex.hpp", "LUNA_PNG_DATA", "LUNA_PNG_SIZE");
```

The generated header is placed under:

```text
build/LunaBuild/<Platform>/<Arch>/<Mode>/generated/<Target>/embedded
```

## DotNet Projects

Use `BuildTargetKind.DotNetProject` for C# tool targets. Declare real source
files as inputs and point LunaBuild at the project and expected output:

```csharp
Kind = BuildTargetKind.DotNetProject;
Sources("src/Tool.Cli/*.cs", "src/Tool.Core/**.cs");
ExcludeSources("src/Tool.Core/bin/**.cs", "src/Tool.Core/obj/**.cs");
DotNetProject("src/Tool.Cli/Tool.Cli.csproj", "src/Tool.Cli/bin/Debug/net9.0/tool.exe");
```

Do not include `bin` or `obj` generated `.cs` files as inputs; dotnet may update
them during build, which would make the target rebuild forever.

## Output And Debugging

Generate a readable graph dump:

```powershell
dotnet run --project Tools/LunaBuild/src/LunaBuild.Cli -- generate --root . --target RHITest4_Box --output build/LunaBuild/RHITest4_Box.lunarules
```

Build and dump at the same time:

```powershell
dotnet run --project Tools/LunaBuild/src/LunaBuild.Cli -- build --root . --target RHITest4_Box --output build/LunaBuild/RHITest4_Box.lunarules
```

Use the timeout wrapper for commands that may invoke compilers or tests:

```powershell
.\Tools\run_with_timeout.ps1 -FilePath (Get-Command dotnet).Source -ArgumentList @('run','--no-restore','--project','Tools\LunaBuild\src\LunaBuild.Cli','--','build','--root','.','--target','RHITest4_Box') -TimeoutSeconds 300
```

## Checklist For New Targets

- Put `<TargetName>.Target.cs` beside the target's `xmake.lua`.
- Keep `rulesPath` equal to the rule file's repository-relative path.
- Use `Category = BuildTargetCategory.Tests` for every test and test helper target.
- Declare target dependencies with `DependsOn(...)`.
- Declare runtime assets with `RuntimeFiles(...)`, not ad-hoc copy commands.
- Prefer simple patterns and explicit lists for special files.
- Run `inspect`, then build the target with a timeout.
