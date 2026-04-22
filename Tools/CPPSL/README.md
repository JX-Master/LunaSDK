# CPPSL Tool

CPPSL is the standalone compiler toolchain for LunaSDK's C++ Shader Language.
It is intentionally kept outside the LunaSDK runtime modules.

Current scope:

- Validate `.cxx` shader entry files.
- Validate CPPSL include boundaries.
- Keep CPPSL `.hxx` headers separate from LunaSDK C++ headers.
- Parse CPPSL sources through ClangSharp and record AST facts.
- Extract a first CPPSL semantic model from source-level attributes.
- Validate entry point stage attributes, resource `set` / `binding` metadata,
  and duplicate struct locations.
- Emit placeholder IR plus early reflection data for descriptors, entry point
  parameters, stage inputs, and stage outputs.

The current macOS arm64 prototype uses these NuGet packages:

- `ClangSharp`
- `libclang.runtime.osx-arm64`
- `libClangSharp.runtime.osx-arm64`

Native Clang extraction is intentionally deferred until ClangSharp proves
insufficient for a concrete semantic requirement.

The compiler pipeline depends on `ICppslFrontend`, not directly on
`ClangSharpFrontend`. This keeps the current phase 0 implementation fast while
leaving room for a later native Clang extractor to emit the same CPPSL frontend
model.

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
  --out build/cppsl/Box
```

Run smoke tests:

```sh
dotnet run --project Tools/CPPSL/tests/CPPSL.SmokeTests
```
