# CPPSL Tool

CPPSL is the standalone compiler toolchain for LunaSDK's C++ Shader Language.
It is intentionally kept outside the LunaSDK runtime modules.

Current scope:

- Validate `.cxx` shader entry files.
- Validate CPPSL include boundaries.
- Keep CPPSL `.hxx` headers separate from LunaSDK C++ headers.
- Parse CPPSL sources through the native Clang extractor and record AST facts.
- Expose a CPPSL-owned frontend model with provider-specific Clang declaration names
  kept only as debug metadata.
- Build the CPPSL semantic model from source-level attributes.
- Validate entry point stage attributes, resource `set` / `binding` metadata,
  duplicate resource bindings, attribute target rules, and duplicate struct
  locations.
- Emit `cppsl.shader_model` v1 with declarations and source-level function
  bodies, plus multiple output targets from a single frontend parse.
- Treat HLSL, GLSL, MSL, and reflection as peer output targets.
- Emit HLSL, GLSL, and Metal sources with platform-specific structs,
  resource declarations, entry signatures, and supported statement/expression
  lowering.
- Emit versioned reflection JSON.

The frontend is native-only. The extractor lives in `Tools/CPPSL/native`, links
against the LLVM SDK under `SDKs`, and runs as a separate process that emits the
frontend JSON contract consumed by the C# pipeline.

Normal LunaSDK builds do not build CPPSL from source. They consume prebuilt
tools from the SDK package:

```text
SDKs/CPPSL/<platform>/<arch>/bin/cppslc
SDKs/CPPSL/<platform>/<arch>/bin/cppsl-native-extractor
```

On Windows the executables use the `.exe` suffix. Update the third-party SDK
archives whenever CPPSL compiler changes must be distributed to normal users.

Open `CPPSL.sln` in Rider or Visual Studio to inspect the tool projects.

Enable CPPSL source-tool targets only when developing the compiler:

```sh
xmake f --build_cppsl_tools=true
xmake build CPPSL
```

Build the solution from command line:

```sh
dotnet build Tools/CPPSL/CPPSL.sln -m:1 /nr:false
```

Build the native extractor before running the compiler:

```sh
xmake f --plat=macosx --arch=arm64 --mode=debug
xmake f --build_cppsl_tools=true
xmake build cppsl-native-extractor
```

Run the sample:

```sh
dotnet run --project Tools/CPPSL/src/CPPSL.Cli -- \
  compile Tools/CPPSL/samples/Box.cxx \
  --stage vertex \
  --entry main_vs \
  --include Tools/CPPSL/std \
  --out build/cppsl/Box \
  --target hlsl,glsl,msl,reflection
```

If `--target` is omitted, all targets are emitted.

Run smoke tests:

```sh
dotnet run --project Tools/CPPSL/tests/CPPSL.SmokeTests
```

Smoke fixtures live under `Tools/CPPSL/tests/fixtures`. They currently cover
box-style vertex IO, texture/sampler reflection and source emission, and
invalid language cases. They are intentionally file-based so the native
extractor and later pipeline stages can run the same language contract tests.
