## Functionality

The LunaSDK project generator creates a standalone C++ application project at
a user-selected filesystem path. The new project imports the current LunaSDK
checkout through LunaBuild instead of copying SDK source files. It can therefore
keep application code in a separate repository while building the required
LunaSDK targets from source with the same platform, architecture, mode, shared
library, and RHI options as the application.

Run the generator from a LunaSDK checkout:

```powershell
./create_project.sh ../MyLunaApp
```

On Windows, use:

```powershell
create_project.bat ..\MyLunaApp
```

The destination must be absent or empty. It must not contain the LunaSDK
checkout and must not be inside that checkout. This restriction prevents a
project's recursive rule discovery from treating the other project tree as its
own source tree.

## Concepts

### Host project

The generated application is the LunaBuild host project. Its root-level
`<ProjectName>.Project.cs` file declares the project identity and imports
LunaSDK. Command-line target names without a project qualifier select targets
from this host project.

### Imported LunaSDK project

The generated project rules call `Project.ImportProject` with a relative path
to the LunaSDK checkout. They copy the host build mode, platform, architecture,
linkage, and RHI selection into the imported project. LunaSDK targets are then
visible through qualified names such as `LunaSDK.Runtime`.

### Local LunaBuild runner

The generated `LunaBuild.csproj` links the LunaBuild CLI sources and references
LunaBuild Core from the selected SDK checkout. This gives the external project
the same cross-platform command shape as LunaSDK itself:

```powershell
dotnet run --project LunaBuild.csproj -- <command> [options]
```

### Application target

`<ProjectName>.Target.cs` declares one executable target, compiles
`Source/*.cpp`, and depends on `LunaSDK.Runtime`. Public include directories,
defines, libraries, and runtime files propagate from that imported dependency.

## Programming guide

### 1. Create the project

Let the destination directory name become the project and target name:

```powershell
./create_project.sh ../MyLunaApp
```

Specify a different name when needed:

```powershell
./create_project.sh ../my-luna-app --name MyLunaApp
```

Names may contain ASCII letters, digits, underscores, and hyphens. A generated
C# rule type replaces hyphens with underscores, while the LunaBuild project and
target keep the requested name.

The generator creates:

```text
MyLunaApp/
  .gitignore
  global.json
  LunaBuild.csproj
  MyLunaApp.Project.cs
  MyLunaApp.Target.cs
  README.md
  Source/
    Main.cpp
```

`global.json` selects the .NET 9 SDK family used by LunaBuild while allowing
newer installed .NET 9 feature bands.

### 2. Inspect the project

Change to the generated directory and inspect its projects and targets:

```powershell
cd ../MyLunaApp
dotnet run --project LunaBuild.csproj -- inspect
```

The output should contain the host project, the imported `LunaSDK` project, and
the host executable target.

### 3. Build and run

Build the application:

```powershell
dotnet run --project LunaBuild.csproj -- build --target MyLunaApp
```

Build and launch it from the terminal:

```powershell
dotnet run --project LunaBuild.csproj -- run --target MyLunaApp
```

The generated `Main.cpp` initializes LunaSDK, prints one message, shuts LunaSDK
down, and exits. Application-owned Luna resources should be released before
calling `Luna::close()`.

### 4. Add LunaSDK modules

Add qualified LunaSDK target names to `DependsOn` in the generated target rule:

```csharp
DependsOn(
    "LunaSDK.Runtime",
    "LunaSDK.Window",
    "LunaSDK.RHI");
```

Include module headers normally after adding the corresponding dependency:

```cpp
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Window/Window.hpp>
```

### 5. Generate IDE files

Generate a VS Code workspace on any supported host:

```powershell
dotnet run --project LunaBuild.csproj -- generate --format vscode --all
```

Generate Xcode files on macOS:

```powershell
dotnet run --project LunaBuild.csproj -- generate --format xcode --all --platform MacOS --arch arm64
```

Generate Visual Studio files on Windows:

```powershell
dotnet run --project LunaBuild.csproj -- generate --format vs2022 --all --platform Windows --arch x64
```

### 6. Move the SDK or project

The generator writes the SDK reference as a relative path in both
`LunaBuild.csproj` and `<ProjectName>.Project.cs`. If their relative layout
changes, update `LunaSdkRoot` in the project file and the path passed to
`ImportProject` in the project rules file.

## Complete rules example

The generated project rules use the following configuration pattern:

```csharp
using LunaBuild.Core;

public sealed class MyLunaAppProjectRules : ProjectRules
{
    public MyLunaAppProjectRules()
        : base("MyLunaApp")
    {
    }

    protected override void ConfigureProject(Project project)
    {
        var lunaSdk = project.ImportProject("../LunaSDK2");
        lunaSdk.PrimaryOptions = lunaSdk.DefaultBuildOptions with
        {
            Mode = Options.Mode,
            Platform = Options.Platform,
            Architecture = Options.Architecture,
            Shared = Options.Shared,
            RhiApi = Options.RhiApi,
        };
        project.UseActionConfiguration(lunaSdk, "luna.meta");
        project.UseActionConfiguration(lunaSdk, "cppsl.shader");
    }
}
```

The action configuration imports allow application targets to use Luna metadata
generation and CPPSL shaders later without changing the project import model.
