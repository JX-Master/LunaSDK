# C# Runtime Migration

This document is a draft plan for moving LunaSDK application and game logic from C++ to C#/.NET, while keeping the C++ layer focused on native platform, rendering, audio, filesystem, job, and other low-level engine services.

The goal is not to rewrite the engine in one pass. The first goal is to establish a small but stable foothold: a native C ABI, a managed host/runtime path, and one C# sample that can create a window, receive events, and render through existing native engine services.

## Background

LunaSDK currently provides several facilities in C++ that are useful for application logic but expensive to maintain as the engine grows:

1. Runtime type registration and reflection.
2. Property metadata.
3. Default serialization and deserialization through reflected properties.
4. C++ business data types in Studio, assets, scenes, components, and editor operations.

The migration direction is:

1. C# owns business data, reflection, serialization, editor metadata, and most sample or game logic.
2. C++ owns native engine services and opaque native objects.
3. The managed/native boundary is a stable per-module C ABI consumed from C# through P/Invoke.
4. The existing C++ reflection and serialization systems are removed after dependent business code is migrated.

## Current State

The current C++ type system is deeper than a pure reflection utility. It supports several runtime services:

1. `object_t` boxed objects store `typeinfo_t` in the object header.
2. `object_alloc` uses the type system for size, alignment, and destruction.
3. `object_is_type` uses reflected base type information.
4. Interface registration stores interface cast thunks in type private data.
5. Serialization uses reflected properties and enum options.
6. ECS uses `typeinfo_t` to allocate, construct, destruct, and relocate component arrays.
7. Window events are boxed objects whose dynamic types are registered through the reflection system.
8. Studio registers C++ component and asset data types, then uses reflected properties for editor UI, serialization, and undo/redo.

This means that removing reflection directly would also break boxed objects, interfaces, Window events, ECS, Asset, and Studio. The first implementation phase must split "native object metadata needed by core runtime" from "full reflection metadata used by business systems."

## Non-Goals

The first phase should not:

1. Port all of Studio to C#.
2. Wrap the full RHI API.
3. Remove all `typeinfo_t` usages immediately.
4. Replace ECS wholesale.
5. Expose C++ classes, templates, exceptions, or STL/Luna containers directly to C#.
6. Make C# own native object memory directly.

## Foothold

The first stable foothold is:

1. Per-module native C ABI layers.
2. Managed Luna C# proxy modules.
3. A minimal native-hosted C# application entry point.
4. A C# sample equivalent to the smallest useful slice of `MultiPlatformSample`.

The foothold is stable when C# can:

1. Initialize and shut down Luna runtime.
2. Create a native window.
3. Receive window/application events without using C++ reflection.
4. Create the minimum rendering objects needed to clear or draw a triangle.
5. Dispose all native handles deterministically through `SafeHandle`.

## Proposed Boxed Object Core

Keep boxed object/reference-counting/interface support, but move it off the full reflection system.

Introduce a minimal boxed object type descriptor:

```c++
struct BoxedTypeDesc
{
    Guid guid;
    const c8* debug_name;
    usize size;
    usize alignment;
    void (*ctor)(void* object);
    void (*dtor)(void* object);
    BoxedTypeHandle base_type;
    Span<const BoxedInterfaceImplDesc> interfaces;
};
```

The exact shape can change, but the descriptor should support only what boxed objects and interfaces need:

1. Memory size and alignment.
2. Construction and destruction.
3. Optional base-type walk for native object casts.
4. Interface cast table.
5. Debug name for diagnostics and memory profiler labels.

It should not support:

1. Property enumeration.
2. Enum option enumeration.
3. Generic type instancing.
4. Type/property attributes.
5. Default serialization.
6. ECS component storage or value relocation.
7. Generic move/copy operations for arbitrary value types.

After this split:

1. `object_alloc` uses `BoxedTypeDesc`.
2. `object_release` calls the descriptor destructor directly.
3. `query_interface` reads an interface table owned by the boxed descriptor.
4. Legacy reflection can still exist temporarily, but boxed objects and interfaces no longer depend on it.

`BoxedTypeDesc` is not intended to be a replacement for the current reflection type system. It is only a compact runtime descriptor for boxed object lifetime and interface casting. ECS should not depend on it. If ECS remains native, it should use a separate component or value descriptor designed for its storage model.

## Native C ABI

The C ABI should be the only supported managed/native binary boundary.

LunaSDK modules should not be imported into C# as one monolithic native module. Every LunaSDK module that needs managed access should provide its own native wrapper and managed proxy.

Do not place C# or C wrapper files under `Modules/Luna`. Keep the original native LunaSDK modules as C++ modules, then add two repository-root directories:

1. One directory for C wrapper targets.
2. One directory for C# managed proxy module targets.

`CWrapper` and `Managed` are for reusable module code only. Samples, tests, tools, and applications should continue to live under purpose-based root directories regardless of whether they are implemented in C++ or C#. `Samples` is for example programs and learning/demo code. `Programs` is only for real tools and applications that are useful outside validation and demonstration.

Rules:

1. Export only `extern "C"` functions.
2. Use fixed-size integer types, POD structs, opaque handles, and LunaSDK `ErrCode` values represented as pointer-sized unsigned integers.
3. Do not expose C++ classes, virtual methods, templates, references, C++ exceptions, `String`, `Vector`, `Variant`, or `Ref<T>`.
4. Use UTF-8 strings.
5. Use two-call buffer APIs for variable-size output.
6. Use explicit retain/release or create/destroy ownership rules.
7. Return LunaSDK `ErrCode` values for fallible calls and expose LunaSDK error query APIs for diagnostic text.
8. Prefix exported symbols by module and interface to avoid collisions.
9. Keep cross-module handles based on the same boxed object ownership rules.
10. Never let C++ exceptions cross the C ABI.
11. Use `.h` and `.cpp` for C wrapper files.
12. Keep `.h` files pure C-compatible headers.
13. Use `.cpp` files for wrapper implementations that include and call LunaSDK C++ APIs.
14. Build C wrapper DLLs through xmake.
15. If a C wrapper must return a variable-length UTF-8 string or string list, the wrapper allocates a copy with LunaSDK memory APIs and exports a matching free function. Managed code must free the result immediately after copying it into C# strings; do not keep static native `String` buffers alive across `Luna::close`.

Possible repository layout:

```text
CWrapper/
    xmake.lua
    Runtime/
        Runtime.h
        RuntimeC.cpp
    Window/
        Window.h
        WindowC.cpp
    RHI/
        RHI.h
        RHIC.cpp

Managed/
    xmake.lua
    Runtime/
        xmake.lua
        Runtime.cs
        ObjectBase.cs
        Internal/
            RuntimeNative.cs
    Window/
        xmake.lua
        IWindow.cs
        WindowModule.cs
        Internal/
            WindowNative.cs
            NativeWindow.cs

Programs/
    Studio/
        xmake.lua
    LunaShader/
        xmake.lua

Samples/
    HelloWindow/
        xmake.lua
        Main.cs

Tests/
    RuntimeCSharpTest/
        xmake.lua
        Main.cs
```

The root `xmake.lua` should include both directories:

```lua
includes("Modules")
includes("CWrapper")
if has_config("managed") then
    includes("Managed")
end
includes("Programs")
includes("Samples")
```

During the first skeleton phase, managed C# targets should be guarded by a `managed` xmake option until the repository baseline requires xmake 3.0.8 or newer. Older xmake versions can still build native LunaSDK modules and C wrapper DLLs without parsing `.cs` targets.

Possible C wrapper xmake target:

```lua
target("LunaRuntimeC")
    set_kind("shared")
    add_files("Runtime/*.cpp")
    add_headerfiles("Runtime/*.h")
    add_deps("Runtime")
    if is_plat("macosx") then
        add_rpathdirs("@loader_path")
    end
```

On macOS, C wrapper dylibs should carry `@loader_path` in their rpath so transitive LunaSDK module dylibs can be resolved when the wrapper is loaded by .NET through P/Invoke instead of by a native executable link.

Possible first API groups:

```c
typedef void* luna_handle_t;
typedef uintptr_t luna_errcode_t;
typedef uintptr_t luna_errcat_t;

int32_t luna_runtime_init(void);
void luna_runtime_close(void);

void luna_runtime_object_retain(luna_handle_t object);
void luna_runtime_object_release(luna_handle_t object);

luna_errcode_t luna_window_create(const LunaWindowDesc* desc, luna_handle_t* out_window);
void luna_window_iwindow_close(void* self);
void luna_window_poll_events(bool wait_events);
luna_errcode_t luna_window_set_event_callback(LunaWindowEventCallback callback, void* userdata);

const char* luna_runtime_error_explain(luna_errcode_t code);
luna_errcode_t luna_runtime_error_unwrap(luna_errcode_t code);
```

The first RHI wrapper should be intentionally narrow. It only needs enough surface for a C# sample to create a device, create a swap chain, record a frame, and present.

## Managed Module Projection

Every imported LunaSDK module should have a corresponding managed proxy module. A proxy module has three layers:

1. Public C# API layer, used by managed application and game code.
2. Internal C# binding layer, which imports the module's C wrapper through P/Invoke.
3. Internal C wrapper layer, built as a native DLL and implemented in C++ by calling the module's public C++ APIs.

Managed projects should also be maintained by xmake. Xmake supports C# targets with `add_files("*.cs")`, C# shared libraries, NuGet packages, and C# to C/C++ interop through P/Invoke. For P/Invoke programs and tests, xmake can use `add_deps` from a C# target to a native shared library target and handle native shared library search paths.

Example shape:

```text
Managed/Window/
    xmake.lua
    IWindow.cs
    WindowModule.cs
    Internal/
        WindowNative.cs
        NativeWindow.cs

Samples/HelloWindow/
    xmake.lua
    Main.cs

CWrapper/Window/
    Window.h
    WindowC.cpp
```

Possible managed xmake target:

```lua
target("Luna.Window")
    set_kind("shared")
    add_files("Window/**/*.cs")
    add_deps("LunaRuntimeC", "LunaWindowC")
    set_values("csharp.target_framework", "net10.0")
    set_values("csharp.nullable", "enable")
    set_values("csharp.allow_unsafe_blocks", true)
```

Possible managed program target:

```lua
target("HelloWindow")
    set_luna_sdk_sample()
    add_files("*.cs")
    add_deps("Luna.Window")
```

Samples, programs, and tests should be grouped by purpose, not by implementation language. A C# `HelloWindow` sample belongs under `Samples`, real user-facing tools and applications belong under `Programs`, and a C# Runtime test belongs under `Tests`.

Managed sample and test targets should use the same `managed` option guard. Samples live under `Samples`, tests live under `Tests`:

```lua
-- Samples/xmake.lua
if has_config("managed") then
    includes("HelloWindow")
end

-- Tests/xmake.lua
if has_config("managed") then
    includes("RuntimeCSharpTest")
end
```

C# binding rules:

1. Prefer source-generated P/Invoke using `LibraryImport`.
2. Represent native objects with `SafeHandle` subclasses.
3. Keep raw P/Invoke declarations internal.
4. Expose small idiomatic C# wrappers on top.
5. Avoid finalizer-only ownership; user-facing handles should implement `IDisposable`.
6. Avoid passing managed object references through native except through `GCHandle` tokens with clear lifetime rules.
7. Marshal strings as UTF-8.
8. Keep callbacks pinned and explicitly unregistered before shutdown.
9. Keep all native pointers opaque in C#.
10. Keep raw P/Invoke functions module-internal.
11. Make high-level managed API semantics close to C++ semantics, but use C# syntax and ownership patterns where they help.
12. `IObject.GetNativeHandle()` may expose the opaque native `object_t` handle publicly, matching C++ `Interface::get_object`.
13. C# code may pass this handle back to Luna C wrapper APIs, but must not inspect, cast, free, or dereference it directly.

Managed visibility rules:

1. Public managed APIs should expose interfaces, abstract base contracts, value descriptors, and module entry points.
2. Concrete managed classes that hold native object pointers or implement C++ interfaces should be internal to their proxy module assembly.
3. Managed factories should return public interfaces instead of concrete native-backed implementation classes.
4. This mirrors the C++ module rule where most implementation classes live under `Source/` and are not visible to external modules.
5. Shared runtime infrastructure such as `ObjectBase` may stay public when other managed module assemblies need to inherit from it, but module-specific concrete wrappers such as `NativeWindow` should remain internal.
6. Managed modules should use the public `IObject.GetNativeHandle()` contract for cross-module object forwarding, rather than opening friend assemblies or exposing concrete implementation classes.

Example shape:

```csharp
internal static partial class NativeMethods
{
    [LibraryImport("LunaRuntime")]
    internal static partial int luna_runtime_init();

    [LibraryImport("LunaRuntime")]
    internal static partial void luna_runtime_close();
}
```

## Error Boundary

C++ should keep using LunaSDK's existing `Result` and `ErrCode` style for fallible operations. C++ code currently avoids exceptions for performance and predictability, and the C wrapper should preserve that rule.

C# public APIs should use exceptions by default. This gives managed application code idiomatic C# control flow while keeping the native ABI simple and stable.

The error boundary is:

```text
C++ implementation returns Result / ErrCode
-> C wrapper returns ErrCode.code as luna_errcode_t
-> internal C# binding checks luna_errcode_t and queries Runtime error APIs
-> public C# API throws ErrorException or a derived exception
```

Rules:

1. C++ implementation code uses `Result` and `ErrCode`.
2. C wrapper functions return `luna_errcode_t` for fallible calls, where `0` means success and non-zero values are LunaSDK `ErrCode.code`.
3. C wrapper functions do not invent another error registry or last-error object.
4. The Runtime C wrapper exports LunaSDK error query APIs such as error-code lookup, category lookup, `explain`, `unwrap_errcode`, and current thread error access.
5. Internal C# binding code converts failed `luna_errcode_t` values into managed exceptions.
6. Public C# APIs throw exceptions for unexpected failures.
7. Public C# APIs may expose `Try*` methods for expected negative results.
8. Exceptions must not cross the native boundary in either direction.
9. C# callbacks invoked by native code must catch managed exceptions before returning to C++.

Possible C ABI shape:

```c
typedef uintptr_t luna_errcode_t;
typedef uintptr_t luna_errcat_t;

luna_errcode_t luna_window_create(const LunaWindowDesc* desc, luna_handle_t* out_window);

luna_errcode_t luna_runtime_error_get_code_by_name(const char* category_name, const char* code_name);
luna_errcat_t luna_runtime_error_get_code_category(luna_errcode_t code);
const char* luna_runtime_error_get_code_name(luna_errcode_t code);
const char* luna_runtime_error_get_category_name(luna_errcat_t category);
const char* luna_runtime_error_explain(luna_errcode_t code);
luna_errcode_t luna_runtime_error_unwrap(luna_errcode_t code);
```

Possible C# binding shape:

```csharp
internal static class Error
{
    public static void ThrowIfFailed(UIntPtr result)
    {
        if (result == UIntPtr.Zero) return;

        var code = RuntimeNative.ErrorUnwrap(result);
        var category = RuntimeNative.ErrorGetCodeCategory(code);
        var message = RuntimeNative.ErrorExplain(result);
        throw ErrorException.FromNativeError(category, code, message);
    }
}
```

The managed exception hierarchy should preserve native information:

```csharp
public class ErrorException : Exception
{
    public UIntPtr Category { get; }
    public UIntPtr Code { get; }
    public string? CategoryName { get; }
    public string? CodeName { get; }

    public ErrorException(UIntPtr category, UIntPtr code, string message)
        : base(message)
    {
        Category = category;
        Code = code;
    }
}
```

Specific exception types can be added gradually for common categories such as invalid arguments, not found, not supported, I/O failure, timeout, and device lost. The base `ErrorException` should always be sufficient to preserve the native error category, code, and message.

Expected-failure queries should not use exceptions as control flow. For example, a managed API may expose:

```csharp
public static bool TryGetAsset(Guid id, out Asset asset);
```

The internal binding may still receive a native error code, but the public API can choose a `Try*` shape when failure is a common, expected result.

## Runtime RTTI Projection

C++ still needs a small RTTI facility after dynamic reflection and default serialization are removed. This RTTI layer compensates for C++'s limited built-in RTTI and should remain available to managed code through the Runtime C wrapper.

The C wrapper should expose type identity only:

1. `object_t` to dynamic type.
2. Type handle to GUID, name, alias, size, and alignment.
3. Type handle to base type.
4. GUID to type handle.
5. Type/base-type checks using type handles.

It should not expose:

1. A `LunaTypeInfo` aggregate structure.
2. A `luna_runtime_type_get_info` API.
3. Property enumeration.
4. Enum option enumeration.
5. Construction, destruction, copy, move, assignment, or relocation APIs.
6. Native object field access.

GUID lookup should be a cold-path operation. APIs that test type compatibility should use `luna_type_t` handles rather than `LunaGuid`, so callers resolve GUIDs once, handle invalid GUIDs explicitly, and then use efficient handle comparisons on hot paths.

Possible C ABI shape:

```c
typedef void* luna_type_t;

luna_type_t luna_runtime_type_get_by_guid(const LunaGuid* guid);
luna_type_t luna_runtime_type_get_object_type(luna_handle_t object);
luna_type_t luna_runtime_type_get_base(luna_type_t type);
void luna_runtime_type_get_guid(luna_type_t type, LunaGuid* out_guid);
const char* luna_runtime_type_get_name(luna_type_t type);
const char* luna_runtime_type_get_alias(luna_type_t type);
uint64_t luna_runtime_type_get_size(luna_type_t type);
uint64_t luna_runtime_type_get_alignment(luna_type_t type);
int32_t luna_runtime_type_is_type(luna_type_t type, luna_type_t target_type);
int32_t luna_runtime_object_is_type(luna_handle_t object, luna_type_t target_type);
```

Managed code should wrap this as `Type` and `RuntimeTypes`. Module-specific bindings can also expose known native type handles directly, such as `WindowEventTypes`, so managed code can use C++ RTTI without doing GUID lookup in event hot paths.

## Boxed Object Projection

C# application code should not operate on native boxed object pointers directly. Public managed APIs should mirror C++ interfaces using C# interfaces, while internal implementation classes hold native pointers and call C wrapper functions.

For each C++ interface exposed to C#, create a corresponding C# public interface:

```csharp
public interface IWaitable : IDisposable
{
    void Wait();
}
```

The internal implementation class owns the boxed object's `object_t` pointer and caches non-owning interface pointers for every supported native interface:

```csharp
internal sealed class WaitableObject : ObjectBase, IWaitable
{
    private readonly nint _waitable;

    public void Wait()
    {
        RuntimeNative.luna_runtime_iwaitable_wait(_waitable);
    }
}
```

Rules:

1. The managed wrapper owns one strong reference to the native `object_t`.
2. Creating a managed wrapper from `object_t` must retain the native object.
3. `Dispose` releases the strong reference immediately.
4. `SafeHandle.ReleaseHandle` releases the strong reference if the wrapper was not disposed.
5. Cached interface pointers are non-owning and valid only while the owning `object_t` is alive.
6. Interface pointers do not increase native reference counts.
7. `IObject.GetNativeHandle()` returns the opaque native `object_t` handle.
8. C# code may pass `object_t` back to C wrapper APIs that perform native `query_interface`, but must not inspect or dereference it.
9. Interface pointers remain internal implementation details and should not be exposed by public C# APIs.
10. C# business code should primarily call through public C# interfaces and wrapper classes.
11. The managed wrapper should keep the owning handle alive for the duration of every native interface call.

Every C++ interface method should be exported as a C function. For example:

```c
luna_errcode_t luna_runtime_iwaitable_wait(void* self);
```

The C wrapper casts `self` to `IWaitable*`, calls `IWaitable::wait`, returns the resulting Luna `ErrCode.code`, and never exposes the C++ vtable or class layout to C#.

This calling path is viable:

```text
C# public interface call
-> internal managed implementation
-> P/Invoke binding
-> C wrapper function
-> C++ interface method
```

The path is appropriate for module-level and object-level operations such as window creation, asset loading, waits, command submission, and resource creation. It should not be treated as a zero-cost replacement for direct C++ calls in hot loops.

Performance rules:

1. Keep cross-boundary calls coarse grained where possible.
2. Avoid per-element, per-component, per-pixel, or very small per-frame property calls across P/Invoke.
3. Batch descriptors, arrays, and command data into blittable buffers.
4. Keep frequently used descriptors blittable.
5. Avoid marshaling `string`, managed arrays, or non-blittable structs in hot paths.
6. Consider `SuppressGCTransition` only for very short, non-blocking, non-callback native functions after measurement.
7. For RHI, prefer command recording and submission APIs that batch work rather than one P/Invoke per tiny state change.

## Hosting Model

Two host directions are needed eventually:

1. Native host starts .NET and calls the managed entry point.
2. Managed tests/tools start first and P/Invoke into native libraries.

The engine application path should use native hosting:

1. C++ executable initializes the platform layer.
2. C++ loads .NET using `nethost` and `hostfxr`.
3. C++ loads the managed app assembly through its runtime config.
4. C++ invokes a known managed entry point.
5. Managed code uses P/Invoke back into per-module Luna C wrapper DLLs.

The initial target should be .NET 10 LTS, unless a target platform blocks it. The host integration should keep the runtime version configurable through the managed app `runtimeconfig.json`.

## Event Boundary

Window events should cross into C# as opaque boxed objects plus explicit C wrapper accessors. C# should not read reflected properties or native C++ object layout.

The chosen model is:

1. The Window C wrapper exports type-handle getters for every known event type, such as `luna_window_get_window_event_type` and `luna_window_get_window_resize_event_type`.
2. Managed code wraps these handles as `Type` values in `WindowEventTypes`.
3. Event dispatch passes the native `object_t` to C# as an opaque `IObject`.
4. Managed code uses `IObject.IsA(WindowEventTypes.X)` to test the dynamic C++ event type through RTTI.
5. Event payload is read through explicit accessors, such as `luna_window_resize_event_get_size`, not through reflection.
6. The managed event object retains the native event object while the C# callback is running, then releases it after the callback returns.
7. Managed exceptions raised by event callbacks are captured and rethrown after `WindowModule.PollEvents` returns, so exceptions never cross the native callback boundary.

Possible managed usage:

```csharp
WindowModule.SetEventHandler(evt =>
{
    if (evt.IsA(WindowEventTypes.WindowResizeEvent))
    {
        var size = WindowEvents.GetResizeSize(evt);
        Resize(size.Width, size.Height);
    }
    else if (evt.IsA(WindowEventTypes.WindowClosedEvent))
    {
        Shutdown();
    }
});
```

This keeps the event path aligned with the retained C++ RTTI capability while avoiding a separate event-kind enum table that can drift from native event type registration.

## Serialization Boundary

Long term, business serialization should move to C#.

Recommended direction:

1. C# owns scene, component, asset description, editor metadata, and undo/redo serialization.
2. C++ `Variant` may remain as a generic native data node for tools and low-level protocols, but not as the default serializer for C# business objects.
3. Existing `.json` asset formats should get migration readers in C# where compatibility is needed.
4. New formats should be versioned at the C# model layer.

During transition:

1. Keep `VariantUtils` only for native systems that still need it.
2. Keep legacy C++ serialization until Studio and asset types are migrated.
3. Do not add new business types to C++ reflection.

## ECS Direction

The current ECS is native and type-info-based. There are two possible paths:

1. Move ECS-level business component storage to C#.
2. Keep a native ECS, but replace `typeinfo_t` component descriptors with explicit C ABI component descriptors.

The first migration should not decide the final ECS design. It should only avoid expanding the current C++ reflected ECS for new business code. ECS should not use `BoxedTypeDesc`; if it remains native, it should use a separate component descriptor or another storage technology.

If native ECS is retained, component types should be registered through a small descriptor:

```c
typedef struct LunaComponentDesc
{
    LunaGuid guid;
    uint32_t size;
    uint32_t alignment;
    void (*construct)(void* data);
    void (*destruct)(void* data);
    void (*move_construct)(void* dst, void* src);
} LunaComponentDesc;
```

C# components with references or managed objects should not be stored as raw native component bytes. They should live in managed storage or be represented by stable managed IDs.

## Campaign Plan

The migration should proceed as a long campaign. The current phase is the defensive phase: the direction is clear, but the new C ABI, managed modules, host integration, and boxed object core are not yet proven in code. The first job is to establish a small, running supply line before expanding the front.

### Phase 0: Campaign Preparation

Goal: Freeze the rules into executable specifications.

Tasks:

1. Confirm root directory structure: `CWrapper`, `Managed`, `Samples`, `Programs`, and `Tests`.
2. Define C ABI basic types: `luna_errcode_t`, `luna_errcat_t`, `luna_handle_t`, `LunaGuid`, string rules, and buffer rules.
3. Define the error boundary from C++ `Result` / `ErrCode` to C# `ErrorException`.
4. Define boxed object exposure: owning `object_t` handles and non-owning cached interface pointers.
5. Define xmake target naming conventions.

Exit criteria:

1. The base ABI names are not blocked by open design questions.
2. `CWrapper/Runtime` and `Managed/Runtime` can be implemented without inventing new ground rules.

### Phase 1: Build Skeleton Loop

Goal: Prove that xmake can build native C wrapper DLLs, C# proxy modules, and C# programs together.

Tasks:

1. Add `CWrapper/xmake.lua`.
2. Add `Managed/xmake.lua`.
3. Include `CWrapper` from root `xmake.lua`, and include `Managed` behind the `managed` option until xmake 3.0.8+ is the repository baseline.
4. Add `CWrapper/Runtime`, producing `LunaRuntimeC`.
5. Add `Managed/Runtime`, producing `Luna.Runtime`.
6. Add `Tests/RuntimeCSharpTest`, proving that C# can P/Invoke into the Runtime C wrapper.

Exit criteria:

1. xmake can build the native Runtime C wrapper.
2. xmake can build the managed Runtime proxy module.
3. xmake can build and run a C# Runtime program or test.
4. C# can call `luna_runtime_init` and `luna_runtime_close`.
5. A native failure can be translated into `ErrorException`.

Current first implementation slice:

1. `CWrapper/Runtime` is the first default-built native wrapper target.
2. `Managed/Runtime` and `Tests/RuntimeCSharpTest` are present as xmake C# targets, guarded by `managed`.
3. With xmake 3.0.8 and .NET 10 installed, `xmake build RuntimeCSharpTest` builds `LunaRuntimeC`, `Luna.Runtime`, and `RuntimeCSharpTest`.
4. `xmake run RuntimeCSharpTest` successfully calls `luna_runtime_init`, observes `Runtime.IsInitialized == true`, and calls `luna_runtime_close`.

### Phase 2: Runtime ABI Foothold

Goal: Stabilize cross-language lifetime, error handling, and boxed object rules.

Tasks:

1. Implement Runtime C wrapper exports for init, close, object retain, object release, and LunaSDK error query APIs.
2. Implement `Managed/Runtime` with `ErrorException`, `ObjectBase`, `SafeHandle`, and internal `RuntimeNative`.
3. Prove `Dispose` releases native references deterministically.
4. Prove `SafeHandle` is a fallback, not the normal ownership path.
5. Prove cached interface pointers are non-owning and remain valid while `object_t` is alive.

Exit criteria:

1. C# holding an `object_t` increments the native strong reference.
2. `Dispose` releases the native strong reference.
3. Exceptions do not cross the native boundary.
4. Runtime ownership and error rules are documented and covered by a small program or test.

Current implementation slice:

1. `CWrapper/Runtime` exports object retain/release/ref-count, weak retain/release/ref-count, expired checks, retain-if-not-expired, query-interface APIs, and handle-based RTTI APIs.
2. `Managed/Runtime` provides `RuntimeErrors`, `RuntimeTypes`, `Type`, `ErrorCode`, `ErrorCategory`, `Guid`, `ObjectBase`, and a `SafeHandle`-based native object handle.
3. `RuntimeCSharpTest` covers Runtime init/close and LunaSDK error-code/category/name/explain lookup.
4. `WindowCSharpTest` covers RTTI lookup against a real boxed window object.
5. Cached interface pointer behavior still needs event or interface-specific tests.

### Phase 3: Window Minimal Usability

Goal: Connect the first real engine module without touching the RHI hot path.

Tasks:

1. Add `CWrapper/Window` for window creation, close, event polling, event callbacks, and essential `IWindow` methods.
2. Add `Managed/Window` with `IWindow`, `WindowModule`, internal native-backed implementation classes, event type handles, and event payload accessors.
3. Add `Samples/HelloWindow`.
4. Stop sending reflected boxed event structs across the managed/native boundary for this path.

Exit criteria:

1. C# can create a native window.
2. C# can receive resize, close, keyboard, mouse, and text events.
3. The event path used by C# does not require full C++ reflection.
4. All native handles in the program are released correctly.

Current implementation slice:

1. `CWrapper/Window` exports Window module initialization, hidden/visible window creation, event polling, event handler registration, event type-handle getters, explicit event payload accessors, and essential `IWindow` methods.
2. `Managed/Window` provides `WindowModule`, public `IWindow` contracts, `WindowEventTypes`, `WindowEvents`, internal native-backed implementation classes, `WindowCreationDesc`, window flags, and basic size APIs.
3. `Samples/HelloWindow` is the first managed sample program. It creates a native window, registers a C# event handler, dispatches events through `WindowEventTypes`, and supports a `--smoke` mode for short automated startup validation.
4. `WindowCSharpTest` creates a hidden native window from C#, updates its title, queries the boxed object's dynamic type through Runtime RTTI, observes a typed `WindowClosedEvent`, closes the window, and disposes the boxed object handle.
5. Resize, keyboard, mouse, text, touch, drop-file, and application event accessors are present, but broader behavior tests are still pending.

### Phase 4: Minimal RHI Rendering Loop

Goal: Draw the first frame from C# through native RHI without wrapping the full RHI API.

Tasks:

1. Add the smallest `CWrapper/RHI` surface for adapter/device, swapchain, command queue, command buffer, and present.
2. Add the smallest `Managed/RHI` surface for frame creation and clear/present.
3. Add a C# sample under `Samples` that clears the screen or draws a triangle.
4. Validate descriptor marshaling and command submission granularity.

Exit criteria:

1. C# can create a window and render through native RHI.
2. Descriptor structs are blittable or intentionally marshaled.
3. P/Invoke call granularity is acceptable for the tested path.

Current implementation slice:

1. `CWrapper/RHI` exports RHI module initialization, backend query, real device/swap-chain/texture/command-buffer handles, resource creation, descriptors, pipeline state, fences, query heaps, and pass/copy/compute command recording surfaces.
2. `Managed/RHI` provides public `Module`, `IDevice`, `ISwapChain`, `ITexture`, `ICommandBuffer`, resource, descriptor, pipeline, fence, query, and clear-value contracts backed by internal native implementations.
3. `Samples/HelloRHI` creates a Window sample, initializes RHI, creates `IDevice`/`ISwapChain`/`ICommandBuffer`, handles framebuffer resize events, records a clear render pass, submits, waits, and presents.
4. The temporary Phase 4 `FrameContext` bridge has been retired. Managed RHI samples now use the same object model shape as the native C++ RHI API.
5. `HelloRHI --smoke` gracefully skips when the platform RHI cannot initialize. On the current macOS run environment, both `HelloRHI --smoke` and the native `RHITest0_Empty` hit platform/GUI/Metal initialization limitations, so real present validation still needs a suitable desktop session.

### Phase 5: Managed Host Loop

Goal: Prove the final application startup direction.

Tasks:

1. Add a native host prototype.
2. Load .NET through `nethost` and `hostfxr`.
3. Load the managed app assembly through `runtimeconfig.json`.
4. Invoke a known managed entry point.
5. Let the managed entry point call back into per-module Luna C wrapper DLLs.

Exit criteria:

1. A native executable can start a managed Luna app.
2. The managed app can use Runtime, Window, and the minimal RHI path.
3. Runtime config, working directory, and native DLL search behavior are understood.

Current implementation slice:

1. `Samples/ManagedHostApp` is a managed sample app. It builds as a C# binary so xmake emits `ManagedHostApp.dll`, `ManagedHostApp.deps.json`, and `ManagedHostApp.runtimeconfig.json`.
2. `Samples/NativeManagedHost` is a native C++ sample host. It uses the platform app-main path, resolves `hostfxr` through `nethost`, initializes .NET from `ManagedHostApp.runtimeconfig.json`, loads `ManagedHostApp.dll`, and invokes `Luna.Samples.ManagedHostApp.ManagedEntry.Run`.
3. The managed entry point is marked with `UnmanagedCallersOnly` and catches all managed exceptions before returning to native code.
4. Native host command-line arguments are forwarded to managed code as opaque UTF-8 strings through a small blittable argument block.
5. By default, the native host looks for the managed app files next to the executable. It also accepts `--runtimeconfig <path>` and `--assembly <path>` for explicit hosting experiments.
6. The sample managed app initializes Runtime and Window, creates a hidden window in `--smoke` mode, and attempts to initialize the temporary RHI frame-context path. RHI failures can be tolerated unless `--require-rhi` is passed.
7. The native host links against the .NET host native pack detected from `DOTNET_ROOT`, `/usr/local/share/dotnet`, `/opt/homebrew/share/dotnet`, or `$HOME/.dotnet`.
8. When building targeted samples manually, build `ManagedHostApp` before `NativeManagedHost` so the default adjacent managed app files exist before running the host.

Verified command:

```sh
xmake build ManagedHostApp
xmake build NativeManagedHost
xmake run NativeManagedHost -- --smoke
```

On the current macOS run environment, `NativeManagedHost --smoke` reaches managed code and Window successfully, while RHI is skipped with the same `bad_platform_call` limitation seen in `HelloRHI --smoke` and the native `RHITest0_Empty`.

### Phase 6: Module Expansion

Goal: Expand the proven pattern across low-level engine modules without migrating Studio yet.

Suggested priority:

1. Runtime extensions: thread, file, stream, log, and time.
2. Window extensions: display, clipboard, file dialogs, and message boxes.
3. RHI object-model completion: continue filling gaps in the managed interfaces that correspond to C++ RHI objects.
4. RHI extensions: resources, descriptors, pipeline, shader, fence, and query.
5. Image, VFS, and low-level Asset capabilities.
6. Font, then ImGui, VG, and other tool-facing modules as Studio migration requires.

Exit criteria:

1. New modules follow the existing C wrapper and managed proxy pattern.
2. No module needs new fundamental ABI ownership rules.
3. Most low-level engine services needed by C# programs are available from managed code.

Current implementation slice:

1. Runtime C wrapper and `Managed/Runtime` expose a first low-risk Runtime service expansion: high-resolution ticks, timestamp/date-time conversion, simple global logging, current directory, process path, and current-directory update.
2. The log bridge currently exposes unformatted global log emission and built-in platform/file sink configuration. Custom C# log handler registration is intentionally deferred because it introduces another native-to-managed callback lifetime and exception boundary.
3. Runtime File/Stream now has a first managed object-model slice. `Managed/Runtime` exposes public `IStream`, `ISeekableStream`, `IFile`, and `IFileIterator` interfaces, while native-backed implementations stay internal.
4. `RuntimeFile` exposes open, whole-file byte loading, file attribute query, copy, move, delete, create directory, and open directory. The stream wrapper currently supports `byte[]` read/write with explicit returned byte counts, seek/tell, size get/set, and file flush.
5. Window Display now has a first managed slice on desktop platforms only. `Managed/Window` exposes opaque `WindowDisplay` handles plus display list, primary display, video mode, supported modes, position, working area, and display name queries when `LUNA_PLATFORM_DESKTOP` is defined.
6. Display handles are not boxed objects and do not carry retain/release ownership; they are opaque platform handles only valid for passing back to Window display APIs. Mobile and console platforms should not expose these managed APIs, matching the native `LUNA_PLATFORM_DESKTOP` conditional surface.
7. Window Clipboard now exposes a small managed `WindowClipboard.Text` property. It throws Luna exceptions for native failures or unsupported platforms and does not add new ownership rules because text is copied across the boundary.
8. Window MessageBox now exposes `WindowMessageBox.Show`. It is intentionally not exercised in automated tests because it blocks the current thread until the user closes the native dialog.
9. Window FileDialog now exposes `WindowFileDialogs.OpenFiles`, `SaveFile`, and `OpenDirectory` on Windows/macOS. Native cancellation still comes back as `BasicError::interrupted`; the C# API provides `TryOpenFiles`, `TrySaveFile`, and `TryOpenDirectory` helpers so normal user cancellation does not need to be written as an exception path.
10. File dialog filters are marshaled as internal blittable C structs containing UTF-8 pointer arrays. Selected paths are copied from native `Path` values into C-owned UTF-8 string lists, then immediately copied again into managed `string` values and freed through the Window C wrapper.
11. `IWindow` now exposes a broader direct operation slice to C#: position and size get/set, style get/set, foreground/minimize/maximize/restore operations, hover/maximized queries, screen/client coordinate conversion, and text input begin/area/end/state APIs. Automated tests exercise the non-disruptive paths on a hidden window; foreground and minimize/maximize operations are intentionally not run in smoke tests because they affect the desktop session.
12. `RuntimeFile.LoadData` covers the first `Blob`-based file loading path by detaching native `Blob` memory through the Runtime C wrapper, copying it into a managed `byte[]`, and freeing the native buffer before returning to user code.
13. RHI now has the first object-model replacement slice: `Module.GetMainDevice` returns public `IDevice`, backed by an internal `ObjectBase` implementation that retains the native `object_t`; C# can query command queue count, command queue descriptors, timestamp frequency, and device feature values through the native `IDevice` pointer.
14. RHI swap chains now have an initial managed object slice. `IDevice.CreateSwapChain` returns public `ISwapChain`, backed by an internal native implementation; C# can inspect and reset swap chain descriptors, query surface transform/reset suggestions, present, and fetch the current back buffer as a public `ITexture` handle with a readable texture descriptor.
15. RHI command buffers now expose the first managed render-loop surface: reset, texture/buffer barriers, render pass begin/end, graphics pipeline layout/state binding, graphics descriptor set binding, vertex/index buffer binding, viewport/scissor, draw/draw-indexed, submit, wait, and try-wait.
16. RHI resources now expose initial managed buffer/texture creation and buffer upload writes. Buffers and textures remain public interfaces with internal native implementations; direct native pointers are only used inside the RHI managed binding layer.
17. RHI pipeline and descriptor objects now have a broader graphics-focused managed slice: descriptor set layout/set creation, uniform/read/read-write buffer and texture descriptor updates, descriptor arrays, pipeline layout creation, graphics pipeline state creation from shader bytecode, blend state, rasterizer state, full depth/stencil operation state, and the state enums/descriptors needed by RHITest2-4.
18. `RHICSharpTest` is the aggregate managed RHI smoke target under `Tests`, with separate `RHICSharpTest0_Empty`, `RHICSharpTest1_Clear`, `RHICSharpTest2_Triangle`, `RHICSharpTest3_Texture`, and `RHICSharpTest4_Box` targets for local per-case execution. Each target skips gracefully when the local environment cannot initialize the platform RHI.
19. RHI `IDeviceChild` is now represented by public managed `IDeviceChild`. Device-created public interfaces inherit it, and the internal binding resolves the native device child from the retained object handle to expose `Device` and `SetName`.
20. RHI managed implementation classes live under `Managed/RHI/Internal` while public interfaces, descriptors, enums, and module entry points remain in `Managed/RHI`, matching the C++ rule that concrete implementation classes are module-internal.
21. RHI adapter enumeration is now exposed through public `IAdapter` and `Module.GetAdapters()`, backed by a retained native adapter object handle and the native `IAdapter::get_name` query.
22. RHI resource memory is now exposed through public `IResource` and `IDeviceMemory`. Buffers and textures inherit `IResource`, can return their backing memory, and `IDevice` exposes aliasing compatibility checks, explicit memory allocation, and aliasing buffer/texture creation.
23. RHI fences are now exposed through public `IFence`, `IDevice.CreateFence()`, and an overload of `ICommandBuffer.Submit` that accepts wait and signal fence arrays while preserving the existing no-fence convenience submit path.
24. RHI query heaps are now exposed through public `IQueryHeap`, `IDevice.CreateQueryHeap()`, `QueryHeapDesc`, `PipelineStatistics`, and the timestamp, occlusion, and pipeline statistics result readback methods.
25. RHI command buffers now expose the remaining graphics-pass helper commands that do not introduce new object models: `AttachDeviceObject`, event begin/end markers, batch graphics descriptor set binding, batch vertex buffer binding, batch viewport/scissor binding, blend factor, stencil reference, and instanced draw variants.
26. RHI command buffers now expose render-pass query attachment fields, resolve attachments, occlusion query begin/end, compute pass begin/end with compute pipeline and descriptor binding plus dispatch, and copy pass begin/end with resource, buffer, texture, buffer-to-texture, and texture-to-buffer copy commands.
27. RHI devices now expose texture data placement queries and compute pipeline state creation. Texture placement returns a managed `TextureDataPlacementInfo`, while compute PSO creation mirrors the graphics PSO bytecode pinning path and returns the same public `IPipelineState` interface.
28. RHIUtility now has its own module proxy instead of being folded into RHI. The first slice exposes `Module.Init`, `IResourceWriteContext`, resource write context creation from a public `IDevice`, buffer writes, texture writes with row/slice pitch handling, reset, commit, and `IDeviceChild` semantics through the managed RHI friend assembly.
29. The first managed ShaderCompiler prototype is no longer on the active migration path after the CPPSL toolchain landed. The old `Managed/ShaderCompiler` and `CWrapper/ShaderCompiler` source trees have been removed, and the current battle line is to consume CPPSL precompiled shader assets directly from managed tests instead of keeping a parallel runtime shader compiler binding alive.
30. `RHICSharpTest` now contains the managed RHITest2 triangle path using CPPSL precompiled shader assets. The test loads backend-specific shader bytecode (`.dxil`, `.spv`, or `.metallib`) from its output directory, creates a real graphics pipeline layout and PSO, writes the triangle vertex buffer from managed bytes, binds pipeline state/layout/vertex buffers, sets viewport/scissor, and records a draw call in the existing render pass.
31. Image decoding has a first managed proxy (`Luna.Image`) that can initialize the native Image module and decode image file bytes or paths into managed `byte[]` pixel data. `RHICSharpTest` now uses it together with CPPSL precompiled shader assets, RHIUtility resource uploads/readback, descriptor sets, texture sampling, index buffers, uniform buffers, and a depth attachment to cover the core RHITest3 texture path and RHITest4 textured-box path. The texture cases read uploaded textures back and compare them with decoded image bytes, and the shared setup validates blit/mipmap context creation, descriptor arrays and read-write descriptors, naming, and `IDeviceChild.Device`. Shader-to-RHI convenience helpers and fuller standalone managed RHITest sample parity remain follow-up work.
32. `RHICSharpTest` has been split internally into app orchestration, shader helpers, asset/data generation, and named test cases. The aggregate target still runs the managed empty frame, clear frame, triangle frame, textured quad, and textured-box cases in RHITest0/1/2/3/4 order, while the individual targets allow testing or debugging one case at a time. These managed RHITest targets now keep a visible window alive and render until the user closes it, matching the native visual-test workflow better than one-frame smoke execution.
33. RHI now exposes adapter-based device creation through `Module.CreateDevice(IAdapter)`, swap-chain-to-window lookup through `ISwapChain.Window`, and a managed `Errors` convenience surface backed by the existing Runtime error-code lookup APIs instead of duplicating native error handling. The managed test setup validates the desktop device creation path once, verifies swap chain window identity, and checks the exported RHI error category/code names.
34. RHI optimized texture clear values are exposed through `ClearValue`, `ClearValueType`, and `IDevice.CreateTexture`/`CreateAliasingTexture` overloads that marshal to native `ClearValue*` only when the caller provides one.
35. The temporary managed `IFrameContext` bridge and the native `luna_rhi_frame_context_*` C wrapper exports have been removed. `ManagedHostApp` now creates the real `IDevice`, `ISwapChain`, and `ICommandBuffer` objects directly, handles resize by resetting the swap chain, and records the clear/present pass in managed code.
36. Zero-copy `Span<byte>`/`Memory<byte>` hot paths, async I/O, custom C# log handlers, and broader RHI object coverage remain follow-up work because each introduces additional ownership, pinning, callback, blocking UI, or scheduling rules.
37. VFS now has a first managed slice (`Luna.VFS`) built directly on top of the existing Runtime file object wrappers. The current bridge exposes module init, platform filesystem driver lookup, mount/unmount/remount, VFS file open, file attribute query, copy/move/delete, directory open, directory creation, and VFS-to-native path translation, all using UTF-8 path strings instead of introducing managed `Name` or `Path` wrappers. `VFSCSharpTest` mounts a temporary native directory, exercises read/write/copy/move/remount/delete flows through VFS, and validates native-path translation.
38. Font now has a first managed slice (`Luna.Font`) with a public `IFontFile` interface and internal native-backed implementation. The current bridge exposes module init, default font access, TTF loading from managed bytes or file data, font-face count, glyph lookup, pixel-height scaling, vertical and horizontal metrics, kerning, glyph shape extraction, glyph and bitmap bounding boxes, and CPU glyph bitmap rendering. Native font file data is copied into engine-owned memory on load, and `FontCSharpTest` validates the default embedded font plus a managed byte-roundtrip reload path.
39. Asset now has its first managed base slice (`Luna.Asset`) covering `asset_t` handle wrapping, module init, register/new/get-by-path flows, GUID/path/name/type/state queries, metadata load/save, asset file enumeration, copy/move/delete, and opaque asset-data object forwarding through public `IObject`. `AssetCSharpTest` mounts a temporary VFS root, creates and registers sample assets, validates metadata files and path mappings, and exercises asset copy/move/delete workflows end to end.
40. Asset also now has the first managed `AssetTypeDesc` registration bridge. Managed code can register native asset types with delegates for load, load-default-data, save, set-data, and referred-assets queries; the C wrapper stores callback sets by asset type name and forwards the native `AssetTypeDesc` callbacks back into C# through pinned delegates and GCHandle userdata.
41. Managed callback exceptions are captured and rethrown on the original `AssetModule.LoadAsset`, `LoadAssetDefaultData`, `SetAssetData`, `SaveAsset`, and `GetReferredAssets` call paths instead of being silently flattened into generic native failures. The current `AssetCSharpTest` uses a managed callback-backed font asset type to validate the callback registration path, state transitions, save callback, set-data callback, and referred-assets callback.
42. VG now has a broader managed slice (`Luna.VG`) covering both the data path and the first renderer path: `TextArrangeResult` arrangement/query, `get_font_glyph_shape`, `get_rect_shape_draw_vertices`, native-backed `IShapeBuffer`, `IFontAtlas`, `IShapeDrawList`, `IShapeRenderer`, and managed `ShapeBuilder` helpers for the built-in primitive generators. The C wrapper keeps `TextArrangeResult` as an opaque native handle so C# can inspect lines/glyphs without copying nested arrays back into C++ when generating draw vertices or committing to draw lists.
43. `VGCSharpTest` currently validates the CPU side of the VG bridge unconditionally (`arrange_text`, glyph-shape extraction, rect-vertex helper) and conditionally exercises the RHI-backed path (`FontAtlas`, `ShapeBuffer`, `ShapeDrawList`, compile/draw-call inspection) when `Luna.RHI` can initialize on the current platform. In sandboxed macOS sessions the RHI branch is skipped on `BasicError::bad_platform_call`, while normal desktop runs should still execute the full path.
44. Two managed visual parity tests now exist for desktop validation: `FontArrangeCSharpTest` mirrors the native `FontArrangeTest` render loop with a live window, animated FPS header text, and the large arranged body text; `VGVisualCSharpTest` mirrors the native `VGTest` scene with the primitive-shape gallery, text heading, perspective camera transform, and right-mouse/keyboard navigation implemented from `Window` events instead of a separate managed `HID` bridge. Both managed visual tests keep the window alive until the user closes it so they can be used for manual render inspection alongside the native tests.
45. ImGui now has a complete managed bridge for the Luna-specific `ImGui.hpp` surface (`Luna.ImGui`). In addition to the original demo-loop slice, the managed API now exposes default glyph ranges, native-backed `ISampledImage`, `Image`/`ImageButton` overloads for raw textures and sampled images, `InputText`/`InputTextMultiline`/`InputTextWithHint` with managed callback trampolines, and the Luna `Gizmo` helper. `ImGuiCSharpTest` has been expanded from a plain demo window into an interactive feature test that exercises text editing, callback mutation, texture display, sampled-image buttons, and gizmo manipulation in one live desktop loop. Full raw Dear ImGui parity (`imgui.h`) is still intentionally out of scope for this module and remains a separate possible future battle if needed.
46. HID now has a first complete managed bridge (`Luna.HID`) for the native public surface across `Keyboard.hpp`, `Mouse.hpp`, `Controller.hpp`, and `KeyCode.hpp`. The managed API exposes module init, `KeyCode`, `MouseButton`, `ControllerButton`, controller state structs, keyboard/mouse/controller support checks, key/button state polling, mouse cursor position query and warp, and controller input/output state calls. `HIDCSharpTest` creates a live desktop window and updates the window title with current keyboard, mouse, and controller state so the bridge can be manually verified against real input devices without introducing a separate rendering dependency.
47. AHI now has a first managed bridge (`Luna.AHI`) for the public `AHI.hpp`, `Adapter.hpp`, `Device.hpp`, and `AHIError.hpp` surface. The current managed API exposes module init, the public `BitDepth`, `WaveFormat`, `DeviceFlag`, `DeviceIoDesc`, and `DeviceDesc` types, adapter enumeration, adapter name/primary/native-format queries, audio device creation, device format/property queries, and playback/capture callback registration and removal. The callback bridge keeps managed delegates pinned while registered, forwards native audio-thread callbacks into C#, and defers any managed callback exception until the next managed device interaction instead of letting exceptions cross the ABI boundary. `AHICSharpTest` now validates adapter enumeration, native-format queries, playback-device creation, and a live playback callback invocation on a desktop run without taking a dependency on the rendering stack.

RHI completion battle plan:

1. Build and preserve a native RHITest baseline for `RHITest0_Empty` through `RHITest4_Box`; these tests define the required managed RHI surface.
2. Completed: replace the temporary `FrameContext` rendering path with real managed `IDevice`, `ISwapChain`, `ITexture`, and `ICommandBuffer` objects. This covers RHITest0/1 and proves the render loop shape.
3. Expose resources and upload paths: `IBuffer`, `ITexture` creation, buffer map/unmap, texture descriptors, direct copy commands, and the minimal `RHIUtility` resource write bridge. This covers the data setup used by RHITest2/3/4.
4. Expose graphics pipeline and descriptor objects: `IDescriptorSetLayout`, `IPipelineLayout`, `IPipelineState`, `IDescriptorSet`, shader bytecode descriptors, input layout, rasterizer, blend, depth/stencil, sampler, buffer views, and texture views. This covers the triangle, textured quad, and box rendering paths.
5. Expose the remaining native RHI surface that is not directly exercised by RHITest0-4 but belongs to the public RHI module: other less common pass helpers.
6. Add managed RHITest equivalents under `Tests` and keep C++/C# sample and test code outside `Managed`/`CWrapper`. The final acceptance target is that all native RHITest targets still build, the aggregate managed `RHICSharpTest` target builds, and the per-case managed targets `RHICSharpTest0_Empty` through `RHICSharpTest4_Box` build and run interactively on a desktop session with graphics access until the user closes each window. If the sandbox reports `bad_platform_call`, the code path is considered locally build-verified and handed to a desktop run.

### Phase 7: Business Migration And Legacy Removal

Goal: Move business data and editor behavior to C#, then remove legacy C++ reflection and serialization.

Tasks:

1. Build a C# version of `MultiPlatformSample`.
2. Prototype C# asset models for material, model, mesh, scene, and project metadata.
3. Add C# serializers and version migration readers.
4. Move editor metadata from C++ property attributes to C#.
5. Move undo/redo data away from C++ reflected `Variant` diffs.
6. Move Studio feature slices one by one.
7. Delete or quarantine legacy `Reflection.hpp`, `Serialization.hpp`, and business-oriented `TypeInfo` APIs.

Exit criteria:

1. New business types are not registered into C++ reflection.
2. Studio business data is not stored as C++ reflected structs.
3. C++ Asset keeps native resources and asset handles, not business object graphs.
4. Runtime boxed object and interface support no longer depends on full reflection.

## Migration Stages

### Stage 1: Native Bridge Foothold

Deliverables:

1. Repository-root `CWrapper` directory with per-module C ABI targets for Runtime and the first required Window/RHI subset.
2. Repository-root `Managed` directory with managed proxy modules for Runtime and the first required Window/RHI subset.
3. Native host loading a managed app through `hostfxr`.
4. C# `HelloWindow` sample under `Samples`.
5. C# minimal rendering sample under `Samples`.
6. Documentation for ownership, threading, callback, and error rules.

Exit criteria:

1. No full reflection dependency in the event path used by the C# program.
2. All native handles in the program are disposed or released correctly.
3. The program works in debug and release on one desktop platform.

### Stage 2: Runtime Core Split

Deliverables:

1. Minimal `BoxedTypeDesc` for boxed objects and interfaces.
2. `object_alloc`, `object_release`, `object_is_type`, and `query_interface` moved to `BoxedTypeDesc`.
3. Legacy reflection separated from the boxed object core.
4. Window event registration no longer requires `register_struct_type`.
5. Runtime tests updated around object lifetime and interface casts.

Exit criteria:

1. Boxed object and interface support builds without including full `Reflection.hpp`.
2. Existing native modules still build through a compatibility layer.
3. No new business-facing API is added to legacy reflection.

### Stage 3: Business Code Migration

Deliverables:

1. C# version of `MultiPlatformSample`.
2. C# asset model prototypes for material, model, mesh, scene, and project metadata.
3. C# editor metadata replacing C++ property attributes.
4. C# undo/redo data model replacing `DiffAssetEditingOp` over native reflected `Variant`.
5. Studio feature slices moved one by one.

Exit criteria:

1. Studio no longer registers business components and assets through C++ reflection.
2. New assets and scenes are saved by C# serializers.
3. C++ Asset module stores opaque native resources and asset handles, not business object graphs.

### Stage 4: Legacy Removal

Deliverables:

1. Delete or quarantine `Reflection.hpp`, `Serialization.hpp`, and legacy type registration.
2. Remove `lustruct`, `luenum`, `luproperty`, `luoption`, `set_serializable`, and property attributes from active business code.
3. Remove `VariantUtils` dependencies from modules that only used it for business serialization.
4. Update manuals.

Exit criteria:

1. Runtime core no longer has full reflection or default serialization.
2. C++ modules expose only low-level native capabilities.
3. C# owns application data and serialization.

## Risk List

1. ABI drift: solve with generated or centrally maintained C# bindings and ABI tests.
2. Lifetime bugs: solve with strict ownership tables and `SafeHandle`.
3. Callback and GC hazards: solve with pinned delegates, unregister APIs, and main-thread dispatch rules.
4. Platform packaging: solve early by testing native host plus managed runtime deployment.
5. RHI wrapper scope creep: solve by wrapping only one rendering path first.
6. Managed RHI parity drift: keep samples and tests on the real C++ RHI object model instead of reintroducing simplified managed-only abstractions.
7. Studio migration size: solve by moving one asset/editor feature slice at a time.
8. Serialization compatibility: solve with explicit format migration readers instead of trying to preserve native reflection behavior.
9. Excessive P/Invoke overhead: solve by measuring and batching hot-path calls.
10. Cross-module handle ambiguity: solve with shared Runtime ownership conventions and module-prefixed APIs.
11. Error semantic drift: solve by keeping native `ErrCode` details inside `ErrorException`.
12. Exceptions crossing ABI boundaries: solve by catching at every managed callback and returning Luna `ErrCode` values.

## Open Questions

1. Should the long-term engine app entry point be native-hosted managed code only, or should managed-first tools also be first-class?
2. Should native ECS survive as a low-level data-oriented store, or should gameplay/editor ECS move fully to C#?
3. Which desktop platform should be the first supported bridge target?
4. How much of RHI should be hand-written C ABI versus generated from a schema?
5. Should C++ `Variant` remain part of the C ABI, or should it stay internal to native tooling only?
6. What compatibility level is required for existing Studio asset files?
7. Should managed wrappers use identity caching so the same `object_t` maps to the same C# wrapper instance?
8. Which modules are required for the first managed program besides Runtime and Window?

## Immediate Next Tasks

1. Write the first per-module C ABI ownership and error-code conventions.
2. Decide the exact root directory names for C wrapper and C# files.
3. Add xmake target conventions for C wrapper DLLs and C# proxy modules.
4. Define `luna_errcode_t`, exported Runtime error query APIs, and the first `ErrorException` shape.
5. Create a narrow Runtime and Window API list for `HelloWindow`.
6. Decide the first platform and .NET SDK/runtime target.
7. Prototype native hosting in a separate program target.
8. Prototype `SafeHandle` wrappers for boxed objects and cached interface pointers.
9. After the prototype works, start the Runtime `BoxedTypeDesc` split.
