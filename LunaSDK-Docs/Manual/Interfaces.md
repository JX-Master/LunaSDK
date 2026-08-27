Interfaces are C++ structures that define abstract virtual APIs. LunaSDK provides much of its functionality through interfaces so implementation details can remain encapsulated and vary between platforms.

## Declaring interfaces

```c++
#include <Luna/Runtime/Interface.hpp>
#include "Counter.generated.hpp"
```

To declare an interface, declare a structure with an `I` name prefix, virtually inherit from `Interface`, and attach a stable GUID with `[[Luna::interface]]`. The header must include its generated header after all ordinary includes and must be listed in the target's `MetaHeaders(...)` rules.

```c++
namespace Luna::Example
{
    struct [[Luna::interface("{6C2880F3-CDDB-4B6C-8C9E-FB462ADC3A3A}")]] ICounter : virtual Interface
    {
        virtual i32 get_value() = 0;
        virtual void set_value(i32 value) = 0;
    };
}
```

An interface can inherit `Interface` directly or inherit one or more other reflected interfaces. Use virtual inheritance so that a concrete object has one shared `Interface` base.

See [[Reflection Metadata with LunaMetaTool]] for target configuration, generated file layout, and registration rules.

## Implementing interfaces

Interfaces are implemented by reflected structures that inherit from them. A concrete implementation must implement `Interface::get_object` so an interface pointer can recover the underlying boxed object.

```c++
namespace Luna::Example
{
    struct [[Luna::struct("{2B14FF17-5DAF-4950-B41F-10A5EF836BD1}")]] Counter : ICounter
    {
        i32 m_value = 0;

        virtual object_t get_object() override { return this; }
        virtual i32 get_value() override { return m_value; }
        virtual void set_value(i32 value) override { m_value = value; }
    };
}
```

Runtime interface queries operate on [[Boxed Objects]]. Create the implementation with `new_object`, `object_alloc`, or another boxed-object facility rather than allocating it with `memnew` or the C++ `new` operator.

LunaMetaTool generates the boxed-type and interface-implementation registration for reflected implementation types. Include the target-generated registration header and call `Luna::Meta::register_<Target>_types()` from the module's initialization path before creating objects of those types. Do not also call `register_boxed_type` or `impl_interface_for_type` manually for the same meta-managed type.

The legacy `luiid`, `lustruct`, `register_boxed_type`, and `impl_interface_for_type` facilities remain available for compatibility and deliberately low-level registration paths. `luiimpl` also remains a convenience macro that implements `get_object`; it does not declare reflection metadata.

Use `is_interface_implemented_by_type` to check whether a registered type implements a specified interface.

## Interface conversion

Besides the dynamic casting functionality provided by boxed objects, LunaSDK provides additional functionalities for casting between interface pointers and boxed object pointers safely at run time.

### Casting typed object pointers to interface pointers

Cast a typed object pointer to one of its declared interface bases with `static_cast`.

### Casting `object_t` to interface pointers

If the underlying type is not exposed, use `query_interface<I>()` to fetch an interface pointer from an `object_t`. The function checks the registered interface information and returns `nullptr` if the object does not implement the requested interface.

### Casting interface pointers to `object_t` 

Convert an interface pointer to `object_t` by calling `get_object`. This function is declared by `Interface` and implemented once by the concrete object, either explicitly or with the `luiimpl` convenience macro. The returned type-less pointer can then be converted to a concrete pointer using the [[Boxed Objects#Run-time type identification and dynamic casting|boxed-object casting APIs]].

## Smart pointer for interface types

```c++
#include <Luna/Runtime/Ref.hpp>
```

`Ref<T>` and `WeakRef<T>` support interface types. For example, `Ref<IStream>` can retain a boxed object that implements `IStream`. Compatible `Ref` conversions use the registered interface metadata; if a run-time conversion fails, the destination pointer is set to `nullptr`.
