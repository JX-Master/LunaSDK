# LunaBuild

LunaBuild is the C# front-end for LunaSDK builds.

The build front-end and back-end are separated in C#:

- `LunaBuild.Core` owns workspace discovery, project target rule loading, build options, and build graph data.
- `LunaBuild.Cli` exposes `inspect`, `generate`, `build`, `clean`, `install`, `run`, and `package`.
- `build` passes the in-memory `BuildGraph` object directly to the C# MakeSystem backend.
- `generate` writes a LunaRules debug dump by default. JSON output is still available for inspection with `--format json`; IDE/project files are available with `--format compile_commands`, `--format vs2022`, `--format vscode`, and `--format xcode`.

Target configuration lives in project-local C# `TargetRules` classes. A rule
file must be named `*.Target.cs` and should live in the target's project
directory. LunaBuild target rules are the authoritative build definitions.
LunaBuild scans these project rule files, compiles them into a temporary project rules assembly under
`build/LunaBuild/ProjectRules`, then loads the resulting `TargetRules` types.
`Tools/LunaBuild` should stay project-agnostic.

The target rule authoring format is documented in `docs/TargetRules.md`.
The LunaRules debug format is documented in `docs/LunaRules.md`. It is not the normal front-end/back-end transport.

Common commands:

```powershell
dotnet run --project LunaBuild.csproj -- inspect --root .
dotnet run --project LunaBuild.csproj -- generate --root . --target ObjLoader --output build/LunaBuild/ObjLoader.lunarules
dotnet run --project LunaBuild.csproj -- generate --root . --target ObjLoader --format json --output build/LunaBuild/ObjLoader.graph.json
dotnet run --project LunaBuild.csproj -- generate --root . --format vs2022 --all --platform Windows --arch x64
dotnet run --project LunaBuild.csproj -- generate --root . --format vscode --all
dotnet run --project LunaBuild.csproj -- generate --root . --format xcode --platform MacOS --arch arm64
dotnet run --project LunaBuild.csproj -- build --root . --target ObjLoader --output build/LunaBuild/ObjLoader.lunarules
dotnet run --project LunaBuild.csproj -- build --root . --category Tests
dotnet run --project LunaBuild.csproj -- build --root . --all --mode Release --api-validation --memory-profiler
dotnet run --project LunaBuild.csproj -- run --root . --target RuntimeTest
dotnet run --project LunaBuild.csproj -- run RuntimeTest -- --list
dotnet run --project LunaBuild.csproj -- package MultiPlatformSample --platform Android --arch arm64-v8a --rhi Vulkan --output build/LunaBuild/AndroidPackages
dotnet run --project LunaBuild.csproj -- clean --root . --all
dotnet run --project LunaBuild.csproj -- install --root . --all --output ./install/debug
```

Project-specific build switches are declared in project `*.Project.cs` rules,
not in LunaBuild Core. LunaSDK currently declares `--api-validation` (also
accepted as `--contract-assertion`), `--thread-safe-assertion`,
`--memory-profiler`, and `--rhi-debug`. The generic spelling
`--property name=value` works for any project-defined property.

The Xcode generator writes an external-build-tool project under
`build/LunaBuild/Xcode/<repo>.xcodeproj` by default. Xcode owns browsing,
schemes, and target dependencies; build, clean, and forced rebuild still call
back into LunaBuild through the generated `lunabuild-xcode.sh` helper script.

Android application packaging is also driven by LunaBuild. `package` first
builds the selected executable target as an Android native shared library,
copies all required `.so` files into the target's `AndroidProject` `jniLibs`
directory, then invokes the Gradle wrapper only for APK packaging. Gradle does
not compile native code.

When running long builds locally, prefer the repository timeout wrapper:

```powershell
.\Tools\run_with_timeout.ps1 -FilePath (Get-Command dotnet).Source -ArgumentList @('run','--no-restore','--project','LunaBuild.csproj','--','build','--root','.','--target','RuntimeTest') -TimeoutSeconds 300
```

The C# MakeSystem backend currently owns graph validation, incremental cache
state, ready-queue scheduling, Windows/MSVC `cpp.compile` and `cpp.link.*`
executors, `cppsl.shader`, and `file.copy`.
