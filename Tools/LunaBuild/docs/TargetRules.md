# LunaBuild Target Rules

`*.Target.cs` files describe LunaBuild targets for a project. `*.Project.cs`
files may describe project-level build properties and defaults. LunaBuild Core
is project-agnostic: it scans the repository for rules files, compiles those
files into a project-local rules assembly, and loads concrete `TargetRules` and
`ProjectRules` types in an isolated load context. Each project root must have
exactly one concrete, root-level `ProjectRules` type.

## Project Imports

Use `ConfigureProject(...)` in a root `*.Project.cs` rule to import another
LunaBuild project. Imports form a tree: a project can reference its own targets
and targets in its import subtree, but not ancestors or sibling subtrees.
Canonical project roots and project names must both be unique in one build
session; duplicate or cyclic imports report the complete import chain.

```csharp
protected override void ConfigureProject(Project project)
{
    var runtime = project.ImportProject("../RuntimeProject");
    runtime.PrimaryOptions = runtime.DefaultBuildOptions with
    {
        Platform = Options.Platform,
        Architecture = Options.Architecture,
        Mode = Options.Mode,
        Shared = Options.Shared,
        RhiApi = Options.RhiApi,
        Properties = runtime.ResolveProperties(new Dictionary<string, string?>
        {
            ["validation"] = "true",
        }),
    };

    project.UseActionConfiguration(runtime, "cppsl.shader");
}
```

The imported project's `PrimaryOptions` and `BuildDirectory` may be assigned
only while its importer is running `ConfigureProject(...)`. Configuration is
frozen before the imported project configures its own imports. A custom
`BuildDirectory` must be absolute; otherwise imported outputs default to a
canonical-root-hashed directory below the host build tree. Two projects cannot
use the same build directory in one session.

Target names are project-qualified across project boundaries:

```csharp
DependsOn("RuntimeProject.Runtime");
```

Bare dependency names stay local to the declaring project. On the command
line, `--target Runtime` means the host project's `Runtime` target, while
`--target RuntimeProject.Runtime` selects the imported target explicitly.

## Location

Place each target rule file next to the target's project files.
`<TargetName>.Target.cs` is the authoritative LunaBuild rule.

Examples:

```text
LunaSDK.Project.cs
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
context assembly. LunaSDK supports macOS arm64 only.
POSIX x86_64 branches are for other supported POSIX platforms.

```csharp
if(Architecture is "arm64" or "aarch64" or "arm64-v8a")
{
    Sources("Source/Platform/POSIX/FiberContext_arm64.S");
}
else if(Architecture is "x86_64" or "x64")
{
    Sources("Source/Platform/POSIX/FiberContext_x86_64.S");
}
```

Use `SupportedPlatforms(...)` when the whole target should only exist on a
subset of platforms:

```csharp
SupportedPlatforms(BuildPlatform.Windows, BuildPlatform.MacOS, BuildPlatform.Linux);
```

`inspect` and graph generation filter unsupported targets before dependency
resolution.

## Project Build Properties

Project-specific build switches should be declared in a `*.Project.cs` file,
not hard-coded in LunaBuild Core. A project rules file can declare typed
properties, command-line aliases, and project-wide effects such as global
defines:

```csharp
using LunaBuild.Core;

public sealed class ExampleProjectRules : ProjectRules
{
    public ExampleProjectRules()
        : base("ExampleProject")
    {
    }

    protected override void ConfigureProperties(BuildWorkspace workspace)
    {
        BooleanProperty(
            "memory_profiler",
            defaultValue: false,
            description: "Enable memory profiler instrumentation.",
            "memory-profiler");
    }

    protected override void Configure(BuildWorkspace workspace, BuildOptions options)
    {
        if(GetBoolean("memory_profiler"))
        {
            GlobalDefines("EXAMPLE_ENABLE_MEMORY_PROFILER");
        }
    }
}
```

Project rules can also define project-wide include directories and a library
name prefix without adding project-specific policy to LunaBuild Core:

```csharp
protected override void Configure(BuildWorkspace workspace, BuildOptions options)
{
    GlobalIncludeDirectories("Modules");
    LibraryPrefix("Example");
}
```

Named action configurations bind generic graph actions to project-owned tools
and resources. File and directory values are resolved relative to the provider
project; bound files and recursively enumerated directory contents become graph
inputs for cache invalidation. An importer must explicitly adopt a configuration with
`UseActionConfiguration(provider, name)` before its targets can use it.

```csharp
ActionConfiguration(
    "cppsl.shader",
    targets: new Dictionary<string, string>
    {
        ["compiler"] = "CPPSL",
        ["native_extractor"] = "cppsl-native-extractor",
    },
    files: new Dictionary<string, string>
    {
        ["dxc"] = "SDKs/DirectXShaderCompiler/bin/dxc",
        ["glslang"] = "SDKs/VulkanSDK/bin/glslangValidator",
    },
    directories: new Dictionary<string, string>
    {
        ["standard_library"] = "Tools/CPPSL/stdlib",
    });
```

Users can set project properties by using their declared aliases or the generic
property form:

```sh
lunabuild build --all --memory-profiler
lunabuild build --all --property memory_profiler=true
```

Target rules can also read project properties directly when the effect should
only apply to one target:

```csharp
protected override void Configure(BuildWorkspace workspace, BuildOptions options)
{
    if(options.Properties.GetBoolean("rhi_debug"))
    {
        Defines("LUNA_RHI_DEBUG");
    }
}
```

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

The current executor supports DXIL for D3D12, SPIR-V for Vulkan, and Metal
library output for Metal.

## Meta Headers

Use `MetaHeaders(...)` for C++ headers that should be scanned by
LunaMetaTool before normal C++ compilation:

```csharp
Headers("Source/*.hpp");
MetaHeaders("Source/MyType.hpp");
```

Targets with meta headers automatically get a build-graph dependency from their
`luna.meta` action to the `LunaMetaTool` target when that tool target is
available for the current platform. This builds the tool before metadata
generation without making it a normal module dependency.

Each meta header must include its generated header by basename after all normal
includes:

```cpp
#pragma once
#include <Luna/Runtime/Runtime.hpp>
#include "MyType.generated.hpp"

struct [[luna::struct("{dbeecd7a-2dc5-423e-8e20-7521826c3f06}")]] MyType
{
};
```

The generated header is placed under:

```text
build/LunaBuild/<Platform>/<Arch>/<Mode>/generated/<Target>/meta
```

The generated meta include directory is added to the target and propagated as a
public include directory to dependent targets. In the first phase these headers
are intentionally empty except for `#pragma once`.

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
dotnet run --project LunaBuild.csproj -- generate --root . --target RHITest4_Box --output build/LunaBuild/RHITest4_Box.lunarules
```

Build and dump at the same time:

```powershell
dotnet run --project LunaBuild.csproj -- build --root . --target RHITest4_Box --output build/LunaBuild/RHITest4_Box.lunarules
```

Use the timeout wrapper for commands that may invoke compilers or tests:

```powershell
.\Tools\run_with_timeout.ps1 -FilePath (Get-Command dotnet).Source -ArgumentList @('run','--no-restore','--project','LunaBuild.csproj','--','build','--root','.','--target','RHITest4_Box') -TimeoutSeconds 300
```

## Checklist For New Targets

- Put `<TargetName>.Target.cs` in the target's project directory.
- Keep `rulesPath` equal to the rule file's repository-relative path.
- Use `Category = BuildTargetCategory.Tests` for every test and test helper target.
- Declare target dependencies with `DependsOn(...)`.
- Declare runtime assets with `RuntimeFiles(...)`, not ad-hoc copy commands.
- Prefer simple patterns and explicit lists for special files.
- Run `inspect`, then build the target with a timeout.
