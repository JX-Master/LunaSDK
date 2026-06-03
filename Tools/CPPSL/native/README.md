# CPPSL Native Extractor

This directory contains the first native Clang extractor for CPPSL. It is a
separate executable that prints the same frontend JSON contract consumed by the
C# `ICppslFrontend` pipeline.

Current scope:

- Parse CPPSL `.cxx` files with Clang C++ APIs.
- Emit CPPSL-owned frontend node kinds.
- Keep provider-specific Clang declaration kinds as debug metadata.
- Extract declaration-level structs, resources, entry functions, fields, and
  parameters.
- Emit source ranges, structured type info, return type info, and type template
  arguments for declaration nodes.
- Emit the first function body AST slice: compound statements, declaration
  statements, local variables, returns, calls, member accesses, declaration
  references, constructor calls, operators, and basic literals.
- Return structured Clang diagnostics with file, line, and column information.

The extractor currently links against the local LLVM SDK:

```text
SDKs/llvm-21.1.1/macosx/arm64
SDKs/llvm-21.1.1/windows/x64
```

If macOS rejects the downloaded SDK dylibs, remove the quarantine attribute:

```sh
xattr -dr com.apple.quarantine SDKs/llvm-21.1.1/macosx/arm64
```

Build locally on macOS arm64 with the direct native build script:

```sh
sh Tools/CPPSL/native/build-macos-arm64.sh
```

Build locally on Windows x64 with LunaBuild:

```sh
dotnet run --project LunaBuild.csproj -- \
  build --target cppsl-native-extractor --platform Windows --arch x64
```

Normal SDK builds consume the prebuilt extractor from `SDKs/CPPSL`; the source
target is only for CPPSL tool development.

Run through the C# CLI:

```sh
dotnet run --project Tools/CPPSL/src/CPPSL.Cli -- \
  compile Tools/CPPSL/tests/fixtures/valid/box/Box.cxx \
  --stage vertex \
  --entry main_vs \
  --include Tools/CPPSL/std \
  --out build/cppsl/BoxNative \
  --target reflection \
  --native-extractor Tools/CPPSL/native/bin/cppsl-native-extractor
```
