# C# Binding Generator Prototype

This tool is a small native prototype for generating the internal C# P/Invoke
binding layer from LunaSDK hand-written C wrapper headers.

Current scope:

- Parse a single C header through the vendored LLVM/Clang SDK.
- Generate blittable struct declarations.
- Generate raw `DllImport` entry points for plain C functions.
- Support per-record managed type remaps.
- Support targeted parameter overrides for awkward ABI cases.
- Keep the public managed API hand-written.

It is intentionally narrow. The prototype is meant to validate the workflow on
headers like `CWrapper/Image/Image.h`, not to replace the public managed layer
or solve every ABI pattern in one step.

## Build

```sh
xmake f --build_binding_tools=true
xmake build csharp-binding-generator
```

## Trial command

```sh
Tools/CSharpBindingGen/native/bin/csharp-binding-generator \
  --source CWrapper/Image/Image.h \
  --output Managed/Image/Internal/Generated/ImageNativeGenerated.g.cs \
  --namespace Luna.Image.Internal \
  --class ImageNativeGenerated \
  --library LunaImageC \
  --function-prefix luna_image_ \
  --record-strip-prefix Luna \
  --record-prefix Native \
  --include CWrapper \
  --clang-arg -isysroot \
  --clang-arg "$(xcrun --show-sdk-path)"
```

## Design boundary

- `CWrapper/*.h` remains the ABI source of truth and stays hand-written.
- `Managed/*/Internal/Generated/*.g.cs` is generated.
- Public managed APIs, exception translation, object lifetime handling, and
  ergonomic overloads remain hand-written.

## Useful options

- `--record-remap NativeName=ManagedName`
  Reuse an existing managed type instead of generating a new struct.
- `--intptr-param param_name`
  Force a parameter to be emitted as `IntPtr`.
- `--function-param-remap native_function:param=ManagedType`
  Override a specific parameter type for one native function.
