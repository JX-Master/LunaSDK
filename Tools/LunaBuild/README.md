# LunaBuild

LunaBuild is the C# front-end for LunaSDK builds.

The build front-end and back-end are separated in C#:

- `LunaBuild.Core` owns workspace discovery, project target rule loading, build options, and build graph data.
- `LunaBuild.Cli` exposes `inspect`, `generate`, and `build`.
- `build` passes the in-memory `BuildGraph` object directly to the C# MakeSystem backend.
- `generate` writes a LunaRules debug dump by default. JSON output is still available for inspection with `--format json`; IDE/project files are available with `--format compile_commands`, `--format vs2022`, `--format vscode`, and `--format xcode`.

Target configuration lives in project-local C# `TargetRules` classes. A rule
file must be named `*.Target.cs` and should sit next to the target's existing
`xmake.lua` while xmake is still present. LunaBuild scans these project rule
files, compiles them into a temporary project rules assembly under
`build/LunaBuild/ProjectRules`, then loads the resulting `TargetRules` types.
`Tools/LunaBuild` should stay project-agnostic.

The target rule authoring format is documented in `docs/TargetRules.md`.
The LunaRules debug format is documented in `docs/LunaRules.md`. It is not the normal front-end/back-end transport.

Common commands:

```powershell
dotnet run --project Tools/LunaBuild/src/LunaBuild.Cli -- inspect --root .
dotnet run --project Tools/LunaBuild/src/LunaBuild.Cli -- generate --root . --target ObjLoader --output build/LunaBuild/ObjLoader.lunarules
dotnet run --project Tools/LunaBuild/src/LunaBuild.Cli -- generate --root . --target ObjLoader --format json --output build/LunaBuild/ObjLoader.graph.json
dotnet run --project Tools/LunaBuild/src/LunaBuild.Cli -- generate --root . --format xcode --platform MacOS --arch arm64 --no-tests
dotnet run --project Tools/LunaBuild/src/LunaBuild.Cli -- build --root . --target ObjLoader --output build/LunaBuild/ObjLoader.lunarules
```

The Xcode generator writes an external-build-tool project under
`build/LunaBuild/Xcode/<repo>.xcodeproj` by default. Xcode owns browsing,
schemes, and target dependencies; build, clean, and forced rebuild still call
back into LunaBuild through the generated `lunabuild-xcode.sh` helper script.

When running long builds locally, prefer the repository timeout wrapper:

```powershell
.\Tools\run_with_timeout.ps1 -FilePath (Get-Command dotnet).Source -ArgumentList @('run','--no-restore','--project','Tools\LunaBuild\src\LunaBuild.Cli','--','build','--root','.','--target','RuntimeTest') -TimeoutSeconds 300
```

The C# MakeSystem backend currently owns graph validation, incremental cache
state, ready-queue scheduling, Windows/MSVC `cpp.compile` and `cpp.link.*`
executors, `cppsl.shader`, and `file.copy`.
