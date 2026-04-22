# CPPSL Tool

CPPSL is the standalone compiler toolchain for LunaSDK's C++ Shader Language.
It is intentionally kept outside the LunaSDK runtime modules.

Current scope:

- Validate `.cxx` shader entry files.
- Validate CPPSL include boundaries.
- Keep CPPSL `.hxx` headers separate from LunaSDK C++ headers.
- Parse CPPSL sources through ClangSharp and record AST facts.
- Expose a CPPSL-owned frontend model with provider-specific Clang cursor names
  kept only as debug metadata.
- Extract a first CPPSL semantic model from source-level attributes.
- Validate entry point stage attributes, resource `set` / `binding` metadata,
  duplicate resource bindings, attribute target rules, and duplicate struct
  locations.
- Emit declaration-level `cppsl.ir` v0 plus multiple output targets from a
  single frontend parse.
- Treat HLSL, GLSL, MSL, and reflection as peer output targets.
- Emit versioned reflection JSON.

The current macOS arm64 prototype uses these NuGet packages:

- `ClangSharp`
- `libclang.runtime.osx-arm64`
- `libClangSharp.runtime.osx-arm64`

The phase 0 frontend still defaults to ClangSharp, and the first native Clang
extractor is available as an alternate frontend for cases where ClangSharp does
not expose enough semantic AST detail.

The compiler pipeline depends on `ICppslFrontend`, not directly on
`ClangSharpFrontend`. This keeps the current phase 0 implementation fast while
leaving room for a later native Clang extractor to emit the same CPPSL frontend
model.

An initial native extractor lives in `Tools/CPPSL/native`. It runs as a separate
process and emits the same frontend JSON contract as the ClangSharp frontend.

Open `CPPSL.sln` in Rider or Visual Studio to inspect the tool projects.

Build the solution from command line:

```sh
dotnet build Tools/CPPSL/CPPSL.sln -m:1 /nr:false
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
box-style vertex IO, texture/sampler reflection, and invalid language cases.
They are intentionally file-based so ClangSharp and the native extractor can run
the same language contract tests.
