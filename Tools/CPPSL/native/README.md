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

The extractor currently links against the local LLVM SDK:

```text
SDKs/llvm-21.1.1-release-macosx-arm64
```

If macOS rejects the downloaded SDK dylibs, remove the quarantine attribute:

```sh
xattr -dr com.apple.quarantine SDKs/llvm-21.1.1-release-macosx-arm64
```

Build locally on macOS arm64 with xmake:

```sh
xmake f --plat=macosx --arch=arm64 --mode=debug -vD
xmake build -vD cppsl-native-extractor
```

The native extractor is included by the LunaSDK root xmake project as a named
tool target. It is marked `set_default(false)`, so normal SDK builds do not
build it unless `cppsl-native-extractor` is requested explicitly.

The fallback direct build script is:

```sh
sh Tools/CPPSL/native/build-macos-arm64.sh
```

Run through the C# CLI:

```sh
dotnet run --project Tools/CPPSL/src/CPPSL.Cli -- \
  compile Tools/CPPSL/tests/fixtures/valid/box/Box.cxx \
  --stage vertex \
  --entry main_vs \
  --include Tools/CPPSL/std \
  --out build/cppsl/BoxNative \
  --target reflection \
  --frontend native \
  --native-extractor Tools/CPPSL/native/bin/cppsl-native-extractor
```
