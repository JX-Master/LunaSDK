LunaMetaTool is LunaSDK's C++ metadata preprocessing tool. It lets LunaSDK collect runtime reflection metadata from C++ declarations instead of maintaining type names, GUIDs, properties, enum options, interface implementations, and module registration code by hand.

LunaMetaTool is integrated into LunaBuild. A target declares the headers that need preprocessing, LunaBuild runs a `luna.meta` action before normal C++ compilation, and the generated headers and registration source are compiled with the target.

## When to use LunaMetaTool

Use LunaMetaTool for user-defined structures, classes, enums, interfaces, boxed object types, and exported properties or enum options that should be visible to LunaSDK's runtime type system.

Prefer LunaMetaTool over the legacy `lustruct`, `luenum`, `luiid`, `luproperty`, and `luoption` macros for new code. The legacy macros remain as compatibility shims for code that has not been migrated yet.

LunaMetaTool currently processes headers only. If a reflected type is defined in a `.cpp` or `.mm` file, move it to a public header or to a private header under the target's `Source` directory and list that header in `MetaHeaders(...)`.

## Header layout

Every header processed by LunaMetaTool must include its generated header:

```cpp
#pragma once
#include <Luna/Runtime/Runtime.hpp>
#include "Foo.generated.hpp"

namespace Luna
{
    struct [[Luna::struct("{01234567-89AB-CDEF-0123-456789ABCDEF}")]] Foo
    {
    };
}
```

The generated include must appear after all non-generated includes. It does not need to appear after the reflected type definitions. The common layout is:

1. `#pragma once`
2. Ordinary includes
3. `#include "Foo.generated.hpp"`
4. Reflected declarations

No non-generated include may appear after a generated include. LunaMetaTool validates this order and reports an error if a normal include is found below a generated include.

The generated header is written to LunaBuild's generated meta directory, not next to the source header. LunaBuild adds that generated directory to include paths automatically.

## Reflection attributes

### Structures and classes

Use `[[Luna::struct("{GUID}")]]` or `[[luna::struct("{GUID}")]]` on a named non-template `struct` or `class` declaration:

```cpp
struct [[Luna::struct("{7c388740-d97a-4e6c-9b7f-dc04e704629f}")]] Camera
{
    [[Luna::property]] f32 fov = PI / 3.0f;
};
```

The generated metadata specializes `Luna::Meta::StructMetaData<T>`. The reflection name is derived from the C++ qualified name. The top-level `Luna::` namespace is removed, so `Luna::Window::WindowEvent` becomes `Window::WindowEvent`.

If a reflected structure derives from another reflected structure in the same target, LunaMetaTool uses that base relationship when generating registration order. A reflected structure may have at most one non-interface reflected base type.

Abstract classes are supported. LunaMetaTool detects abstract C++ record declarations and generates abstract structure registration for them.

Templates are not supported by LunaMetaTool in this phase. Generic structure types still need the runtime generic type registration APIs described in [[Type System]].

### Properties

Use `[[Luna::property]]` on fields that should be exported as structure properties:

```cpp
struct [[Luna::struct("{CE0188A0-C1A6-421E-A60C-8D4F260972A3}")]] SceneSettings
{
    [[Luna::property]] Asset::asset_t skybox;
    [[Luna::property]] f32 exposure = 1.0f / 9.6f;
};
```

For each property, LunaMetaTool records:

- The field name.
- The byte offset of the field.
- A pointer to the property's `typeof<T>` function.

The runtime registration path converts this generated compile-time metadata into `StructurePropertyDesc` values. This lets default serialization and other reflection users discover the exported fields after the type is registered.

Property metadata is only generated for fields explicitly marked with `[[Luna::property]]`. Unmarked fields remain normal C++ fields and are not exported.

### Enums

Use `[[Luna::enum("{GUID}")]]` or `[[luna::enum("{GUID}")]]` on a named enum with a fixed underlying type:

```cpp
enum class [[Luna::enum("{920C8F7F-7CEC-4776-BF01-1F63A4C51D9F}")]] CameraType : u32
{
    perspective [[Luna::option]] = 0,
    orthographic [[Luna::option]] = 1,
};
```

The generated metadata specializes `Luna::Meta::EnumMetadata<T>`. Mark enum values that should be exported with `[[Luna::option]]`. Unmarked enum values are not exported to the runtime enum option list.

Anonymous enums and enums without fixed underlying types are not supported by LunaMetaTool.

### Interfaces

Use `[[Luna::interface("{GUID}")]]` on interfaces derived from `Luna::Interface`:

```cpp
struct [[Luna::interface("{6BF6C9B0-0541-42BD-B96B-FEF52C9E4D40}")]] IFoo : virtual Interface
{
    virtual u32 marker() = 0;
};
```

The generated metadata specializes `Luna::Meta::InterfaceMetaData<T>`. Object types that implement reflected interfaces are registered through generated target registration code, so normal module code does not need to call `impl_interface_for_type`.

### Boxed object types

A boxed object type is authored as a normal reflected structure that implements one or more reflected interfaces:

```cpp
struct [[Luna::struct("{7B993692-2949-4E6F-9B73-3CA09C23B7BA}")]] FooObject : IFoo
{
    virtual object_t get_object() override
    {
        return this;
    }

    virtual u32 marker() override
    {
        return 42;
    }
};
```

Do not call `register_boxed_type` or `impl_interface_for_type` manually for meta-managed boxed object types. LunaMetaTool generates the required registration code.

## Generated files

For a header named `Foo.hpp`, LunaMetaTool generates:

```text
Foo.generated.hpp
```

This file contains forward declarations and metadata specializations such as:

- `Luna::Meta::StructMetaData<Foo>`
- `Luna::Meta::EnumMetadata<FooEnum>`
- `Luna::Meta::InterfaceMetaData<IFoo>`

For a target named `MyModule`, LunaMetaTool also generates:

```text
MyModule.meta.generated.hpp
MyModule.meta.generated.cpp
```

The generated header declares:

```cpp
namespace Luna::Meta
{
    void register_MyModule_types();
}
```

The generated source registers all reflected types owned by the target:

- Reflected enum types.
- Reflected structure and class types.
- Abstract reflected structure types.
- Reflected boxed object types.
- Interface implementations for reflected boxed object types.

The registration order follows reflected type dependencies. Base reflected types and property type dependencies are registered before the types that depend on them when they are owned by the same target.

## Using LunaMetaTool in LunaBuild

Declare meta headers with `MetaHeaders(...)` in the target's `.Target.cs` file. Patterns are relative to the target directory, just like `Headers(...)` and `Sources(...)`.

```csharp
namespace LunaBuild.Core.Targets;

public sealed class MyModuleTargetRules : TargetRules
{
    public MyModuleTargetRules()
        : base(
            name: "MyModule",
            targetDirectory: "Modules/Luna/MyModule",
            rulesPath: "Modules/Luna/MyModule/MyModule.Target.cs")
    {
        Headers("*.hpp", "Source/**.hpp");
        MetaHeaders(
            "Foo.hpp",
            "Source/FooImpl.hpp");
        Sources("Source/**.cpp");
        DependsOn("Runtime");
    }
}
```

`MetaHeaders(...)` can list public headers and private headers. Use private headers under `Source` for implementation-only reflected object types that should not become part of the module's public C++ API.

Two meta headers in the same target must not produce the same generated header name. For example, `Foo.hpp` and `Source/Foo.hpp` would both generate `Foo.generated.hpp`, so LunaBuild reports this as an error. Rename one of the headers to make generated names unique.

When a target has meta headers, LunaBuild:

1. Adds a `luna.meta` action to the build graph.
2. Runs LunaMetaTool before compiling any C++ source in that target.
3. Generates files under `build/LunaBuild/<Platform>/<Arch>/<Mode>/generated/<Target>/meta`.
4. Adds that directory to the target include paths.
5. Exposes that directory as a public include path to dependent targets.
6. Compiles the generated `<Target>.meta.generated.cpp` source with the target.

The generated include directory is also emitted into generated IDE and compile command outputs. Do not add the generated directory manually in target rules.

LunaBuild locates `LunaMetaTool` from the current build output, common host debug build outputs, `Tools/LunaMetaTool/bin`, or the `SDKs/LunaMetaTool` prebuilt locations. If LunaBuild reports that `LunaMetaTool` is missing, build it first:

```sh
dotnet run --project LunaBuild.csproj -- build --target LunaMetaTool
```

Then build the target that uses `MetaHeaders(...)`:

```sh
dotnet run --project LunaBuild.csproj -- build --target MyModule
```

## Registering generated types in a module

Include the generated target registration header in one module source file and call the generated function during module initialization:

```cpp
#include "MyModule.meta.generated.hpp"
#include <Luna/Runtime/Module.hpp>

namespace Luna
{
    struct ModuleImpl : Module
    {
        virtual const c8* get_name() override { return "MyModule"; }

        virtual RV on_init() override
        {
            Meta::register_MyModule_types();
            return ok;
        }
    };
}
```

Call the generated registration function once, after the module's dependencies have been initialized and before runtime code expects those reflected types to be available through `typeof<T>()`, `cast_object<T>()`, serialization, or interface queries.

If the module still has manually registered generic structure types or other special runtime-only reflection entries, keep those explicit calls beside `Meta::register_<Target>_types()`.

## Generated metadata and Runtime APIs

Generated metadata lives in `Luna::Meta`. User code normally does not need to instantiate these templates directly, but they are useful when C++ code needs the compile-time GUID of a reflected type:

```cpp
Guid guid = Meta::StructMetaData<Foo>::__guid;
```

Runtime APIs such as `typeof<T>()`, `register_struct_type<T>()`, `register_enum_type<T>()`, `new_object<T>()`, `cast_object<T>()`, and `query_interface<T>()` consume generated metadata through Runtime helper templates.

The old `lustruct`, `luenum`, and `luiid` macros still feed compatibility defaults for `Luna::Meta::*MetaData`, so migrated and unmigrated code can coexist while the codebase transitions to LunaMetaTool.

## Diagnostics and limitations

LunaMetaTool diagnoses common authoring errors:

- Missing generated include.
- Normal includes after a generated include.
- Missing GUID string in Luna attributes.
- Duplicate Luna reflection attributes on one declaration.
- Wrong attribute kind on a declaration.
- Anonymous reflected records or enums.
- Template reflected records.
- Reflected enums without fixed underlying types.
- Generated header name collisions inside one target.
- Unsupported generated registration dependency cycles.

Keep GUIDs globally unique. LunaMetaTool validates local declaration shape, but the runtime type system still relies on GUID uniqueness across the program.

LunaMetaTool scans source ranges for Luna attributes because unknown scoped C++ attributes are not preserved by Clang in the form this tool needs. Use the documented spellings exactly: `[[Luna::struct(... )]]`, `[[Luna::enum(... )]]`, `[[Luna::interface(... )]]`, `[[Luna::property]]`, and `[[Luna::option]]`.

## Example

Header:

```cpp
#pragma once
#include <Luna/Runtime/Runtime.hpp>
#include "MetaSmoke.generated.hpp"

namespace Luna::MetaToolSmoke
{
    struct [[Luna::interface("{6BF6C9B0-0541-42BD-B96B-FEF52C9E4D40}")]] IMetaSmokeInterface : virtual Interface
    {
        virtual u32 marker() = 0;
    };

    struct [[Luna::struct("{C7D43C71-9895-47DA-9A78-F5502609BE30}")]] MetaSmokeStruct
    {
        [[Luna::property]] i32 value = 0;
    };

    struct [[Luna::struct("{7B993692-2949-4E6F-9B73-3CA09C23B7BA}")]] MetaSmokeBoxed : IMetaSmokeInterface
    {
        virtual object_t get_object() override { return this; }
        virtual u32 marker() override { return 42; }
    };

    enum class [[Luna::enum("{3B894695-E906-40D7-85EC-74E209541438}")]] MetaSmokeEnum : u32
    {
        A [[Luna::option]] = 0,
        B [[Luna::option]] = 1,
    };
}
```

Target rule:

```csharp
Headers("Source/*.hpp");
MetaHeaders("Source/MetaSmoke.hpp");
Sources("Source/*.cpp");
DependsOn("Runtime");
```

Source:

```cpp
#include "MetaSmoke.hpp"
#include "LunaMetaToolSmoke.meta.generated.hpp"

int main()
{
    Luna::init();
    Luna::Meta::register_LunaMetaToolSmoke_types();

    auto type = Luna::typeof<Luna::MetaToolSmoke::MetaSmokeStruct>();
    luassert(type);

    Luna::close();
    return 0;
}
```
