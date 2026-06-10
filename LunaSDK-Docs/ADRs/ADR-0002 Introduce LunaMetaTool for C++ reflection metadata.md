
## Status
Approved.

## Last updated
2026/6/9

## Background
LunaSDK's runtime reflection system needs stable type identities, interface identities, structure properties, enum options, base-type relationships, and module registration order. Before LunaMetaTool, this information was maintained by hand with macros such as `lustruct`, `luenum`, `luiid`, `luproperty`, and `luoption`, plus explicit calls to `register_struct_type`, `register_enum_type`, `register_boxed_type`, and `impl_interface_for_type`.

That model made reflection metadata easy to desynchronize from C++ declarations. Type names could drift after a C++ rename, GUID declarations were split from registration sites, property and enum option lists duplicated source declarations, and every module had to remember the right manual registration sequence. It also made interface and boxed-object registration especially repetitive.

The replacement must work with normal C++ headers, Objective-C++ implementation headers, LunaBuild's target graph, existing Runtime reflection APIs, and generated headers that are usually included after ordinary includes but before the reflected type definitions. It must also preserve compatibility with existing code that has not yet migrated away from the old macros.

## Decision
Introduce `LunaMetaTool`, a LunaBuild-integrated C++ metadata preprocessing tool. Targets declare reflected headers through `TargetRules.MetaHeaders(...)`. LunaBuild emits one `luna.meta` build action per target with meta headers, runs it before compiling any C++ source in that target, adds the generated metadata directory to the target include paths, and exposes that directory as a public include path to dependent targets.

Reflected declarations are written directly in C++ headers using attributes:

```cpp
struct [[Luna::interface("{...}")]] IFoo : virtual Interface
{
};

struct [[Luna::struct("{...}")]] Foo : IFoo
{
    [[Luna::property]] i32 value = 0;
};

enum class [[Luna::enum("{...}")]] FooMode : u32
{
    A [[Luna::option]] = 0,
};
```

Each meta header must include its generated counterpart, for example `#include "Foo.generated.hpp"`, after all non-generated includes. The generated include does not need to appear after the type definitions; the expected layout is ordinary includes, generated include, then declarations. LunaMetaTool pre-creates generated headers before parsing so this layout works on the first build.

LunaMetaTool uses Clang's C++ API through `libclang-cpp` to parse the declared headers and scans source ranges for Luna attributes. For every reflected header it generates `<HeaderBaseName>.generated.hpp` in the target's build-generated meta directory. Generated metadata lives under `Luna::Meta`, not the legacy `Luna::TypeInfo` path:

- `Luna::Meta::StructMetaData<T>` for structure and class identities, names, and property metadata.
- `Luna::Meta::EnumMetadata<T>` for enum identities, names, and option metadata.
- `Luna::Meta::InterfaceMetaData<T>` for interface identities.

For every target with meta headers, LunaMetaTool also generates `<Target>.meta.generated.hpp` and `<Target>.meta.generated.cpp`. The generated source defines `Luna::Meta::register_<Target>_types()`, registers reflected enums and structs in dependency order, handles abstract struct registration, and emits the runtime interface implementation registrations needed for object types that implement reflected interfaces. Module initialization code should call this generated function instead of manually listing each reflected type.

Runtime reflection APIs are updated to consume generated metadata. `typeof<T>()`, structure registration, enum registration, object casting, and interface registration read from `Luna::Meta::*MetaData` while retaining compatibility fallbacks for old `lustruct`, `luenum`, and `luiid` code. Structure property metadata stores the property name, byte offset, and a `typeof<T>` function pointer for the property type so compile-time generated metadata can be translated to runtime `typeinfo_t` during registration.

Boxed object types are authored as normal `[[Luna::struct]]` types that implement reflected interfaces. Call sites no longer manually invoke `register_boxed_type` or `impl_interface_for_type`; any lower-level runtime registration calls required by the current implementation are emitted by generated code.

## Impact
The reflection source of truth moves closer to the C++ declaration. Type names are derived from qualified C++ names, property and option exports are attached to the actual members, and module registration is centralized in one generated function per target. This reduces manual boilerplate and prevents the common class of errors where reflection names, exported members, or interface implementations are updated separately from the C++ type.

The build graph becomes responsible for reflection preprocessing. Generated headers and generated registration sources are build artifacts under `build/LunaBuild/<Platform>/<Arch>/<Mode>/generated/<Target>/meta`, and C++ compilation depends on successful `luna.meta` execution. This adds an LLVM SDK and LunaMetaTool availability requirement to supported build environments.

The scheme intentionally has strict constraints:

- LunaMetaTool processes headers only. Types that previously lived only in `.cpp` files must move to private headers if they need reflection metadata.
- Two meta headers in the same target may not generate the same `*.generated.hpp` name. LunaBuild reports the colliding source header paths, and the source header names must be made unique.
- Reflected records must be named, non-template class or struct declarations. Reflected enums must be named and have a fixed underlying type.
- The generated include must appear after all non-generated includes, and no non-generated include may appear after any generated include.
- The generated registration order follows reflected base-type and property dependencies; unsupported cycles or missing registration dependencies are diagnosed.

The legacy macros remain as compatibility shims for unmigrated code, but new reflected declarations should use LunaMetaTool attributes and `MetaHeaders`.

## Alternatives considered
Continue using manual macros and registration calls. This keeps the build simpler, but it preserves the duplication that caused metadata drift, stale reflection names, missing property exports, and manual registration-order bugs.

Use external schema files for reflection metadata. This would decouple metadata from C++ parsing, but it would create another source of truth and make member offsets, enum values, inheritance, and interface implementation relationships harder to validate against the real declarations.

Wait for standard C++ reflection or use compiler plugins. Standard reflection is not available for LunaSDK's current portability requirements, and compiler plugins would couple the SDK more tightly to specific compiler frontends. A standalone libclang-based tool gives LunaSDK Unreal Header Tool / Qt MOC style behavior while staying explicit in LunaBuild.

Generate reflection data only from runtime templates. Templates can reduce some boilerplate, but they cannot discover which headers and members should be exported, cannot enforce generated include layout, and cannot synthesize a complete per-module registration function without an external collection mechanism.

## Remarks
The current tool reads unknown scoped attributes from source text because Clang does not preserve them as normal semantic attributes in the way LunaMetaTool needs. The generated-code namespace is deliberately `Luna::Meta` to avoid overloading the legacy `TypeInfo` naming and to keep generated metadata distinct from runtime type objects.

## Version history
* **2026/6/9** Proposed and approved.
