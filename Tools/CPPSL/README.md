# CPPSL Tool

CPPSL is the standalone compiler toolchain for LunaSDK's C++ Shader Language.
It is intentionally kept outside the LunaSDK runtime modules.

Current scope:

- Validate `.cxx` shader entry files.
- Validate CPPSL include boundaries.
- Keep CPPSL `.hxx` headers separate from LunaSDK C++ headers.
- Parse CPPSL sources through ClangSharp and record top-level declarations.
- Emit placeholder artifacts for the future IR, reflection, and backend source generators.

The current macOS arm64 prototype uses these NuGet packages:

- `ClangSharp`
- `libclang.runtime.osx-arm64`
- `libClangSharp.runtime.osx-arm64`

Native Clang extraction is intentionally deferred until ClangSharp proves
insufficient for a concrete semantic requirement.

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
