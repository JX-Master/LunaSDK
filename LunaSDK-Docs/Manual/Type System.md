Type reflection is the ability of a program to introspect type name, size, layout and other information in the program. Such ability can be used to write code that can operate on different types. LunaSDK comes with a run-time type reflection system that tracks most types used in the framework, it can also be extended to accept user-defined new types, including enumeration types, structure types and generic structure types.

## Type object

```c++
#include <Luna/Runtime/TypeInfo.hpp>
```

`typeinfo_t` represents one type object that stores the type information for one type registered to type reflection system. You can get the type object of one specified type by calling `typeof<T>()`. If the specified type is not registered, the program may fail to compile or `nullptr` will be returned.

## Type name and GUID

```c++
#include <Luna/Runtime/Reflection.hpp>
```

Registered non-generic types and generic type families can be looked up by name with `get_type_by_name` or by GUID with `get_type_by_guid`. Fetch the name and GUID associated with a type object by calling `get_type_name` and `get_type_guid`.

Every non-generic registered type and every generic type family has a stable GUID. Instantiated types of one generic family share the family's name and GUID and are distinguished by their generic arguments and `typeinfo_t` handles. Multiple unrelated types may have the same name, but each must have a unique alias so that it can be distinguished from the others. If the type is defined in namespaces, its namespace should be prepended to the type name and separated with double colons (`::`).

## Type size and alignment

```c++
#include <Luna/Runtime/Reflection.hpp>
```

Every registered type except generic structure type will have one specific size and alignment value, which can be fetched by `get_type_size` and `get_type_alignment`. Generic structure type is not a real type and will return `0` for both functions.

## Type class

```c++
#include <Luna/Runtime/Reflection.hpp>
```

There are different type classes in LunaSDK, including:

1. Primitive type
2. Structure type
3. Enumeration type
4. Generic structure type
5. Generic structure instanced type

Every registered type in LunaSDK belong to one type class. You can use `is_primitive_type`, `is_struct_type`, `is_enum_type`, `is_generic_struct_type` and `is_generic_struct_instanced_type` to check the class of one `typeinfo_t` object. 

### Primitive type

Primitive types are predefined simple types, including `void`, `u8`, `i8`, `u16`, `i16`, `u32`, `i32`, `u64`, `i64`, `usize`, `isize`, `c8`, `c16`, `c32`, `f32`, `f64` and `bool`. `void` is a special type with `size` and `alignment` equal to `0`, and is mainly used as type parameters of generic types. 

Primitive types cannot be registered by users.

### Structure type

Structure types are used to represent a set of data of different types. Structure types may have properties (member objects), they can also define special functions called meta functions to let LunaSDK handle these types correctly. If such meta function is not provided, LunaSDK will use the default meta function for the type. The following table lists all meta functions provided for one structure type `T`.

| Meta function            | Usage                                                        | Default meta function                                     |
| ------------------------ | ------------------------------------------------------------ | --------------------------------------------------------- |
| Constructor              | Constructs one object of type `T`.                           | Calls constructors for all properties of `T`.             |
| Destructor               | Destructs one object of type `T`.                            | Calls destructors for all properties of `T`.              |
| Copy constructor         | Constructs one object of type `T` by coping data from another object of type `T`. | Calls copy constructors for all properties of `T`.        |
| Move constructor         | Constructs one object of type `T` by moving data from another object of type `T`. | Calls move constructors for all properties of `T`.        |
| Copy assignment operator | Assigns data of one object of type `T` by coping data from another object of type `T`. | Calls copy assignment operator for all properties of `T`. |
| Move assignment operator | Assigns data of one object of type `T` by moving data from another object of type `T`. | Calls move assignment operator for all properties of `T`. |

Note that once the user-defined meta function is provided, the corresponding default meta function will not be called.

#### Structure inheritance

One structure type can inherit from another structure type. The structure type being inherited from is called *base type* or *base structure*, and the structure type derived from the base type is called *derived type* or *derived structure*. Every structure type can only have at most one base type, but may have multiple derived types.

### Enumeration type

An enumeration type defines a group of options. Every enumeration have one integral underlying type, and every option of the enumeration is mapped to one specific value of that underlying type. Different options in the same enumeration must have different mapped values.

LunaSDK supports *multiple enumeration type*, which enables the user to select multiple options instead of only one as the value of the enumeration. In such case, every option will take one bit of the underlying integral type, and the enumeration value is stored by bitwise OR combination of selected options.

### Generic structure type and generic structure instanced type

Generic structure type represents one structure type with generic parameters, such as `Vector<T>` or `Array<T, Count>`. A generic argument can be a type or an integer. The number of generic parameters may be variable, like `Tuple<T1, T2, ...>`.

Generic structure types cannot be used directly; they must be instantiated as a *generic structure instanced type* by calling `get_generic_instanced_type`. Instantiation happens at run time. Each combination of generic structure type and generic arguments is instantiated only once, and the resulting type object is reused. A generic structure instanced type can be used like a normal structure type.

## Registering structure types

```c++
#include <Luna/Runtime/Reflection.hpp>
```

### Generated registration

For new reflected structures, use LunaMetaTool attributes and generated registration. Attach `[[Luna::struct]]` to a named non-template structure and `[[Luna::property]]` to every field that should be visible to Runtime reflection. The header must include its generated header after all ordinary includes.

```c++
#include <Luna/Runtime/Math/Vector.hpp>
#include "SpotLight.generated.hpp"

namespace Luna::Example
{
    struct [[Luna::struct("{2BB45396-E0E3-433E-8794-49BEE8BD1BB5}")]] SpotLight
    {
        [[Luna::property]] Float3 intensity = {0.5f, 0.5f, 0.5f};
        [[Luna::property]] f32 intensity_multiplier = 1.0f;
        [[Luna::property]] f32 attenuation_power = 1.0f;
        [[Luna::property]] f32 spot_power = 64.0f;
    };
}
```

List the header in the target's `MetaHeaders(...)` rules. LunaBuild then generates compile-time metadata and a target registration function. Include the target-generated header in one source file and call the registration function after Runtime and the module's dependencies are initialized:

```c++
#include "MyModule.meta.generated.hpp"

void initialize_my_module_reflection()
{
    Luna::Meta::register_MyModule_types();
}
```

For declarations owned by the same target, the generated function registers reflected base types and property dependencies before the structures that depend on them. Types owned by dependency targets must already be registered through module initialization order. See [[Reflection Metadata with LunaMetaTool]] for header layout, supported declarations, and target configuration.

### Low-level and compatibility registration

`register_struct_type`, `StructureTypeDesc`, and explicit `typeof_t<T>` specializations remain available for third-party or low-level types that intentionally bypass LunaMetaTool. Code using this path is responsible for providing a stable GUID and name, correct size and alignment, property offsets and types, base-type relationships, lifecycle operations, and a stable way for `typeof<T>()` to retrieve the registered handle.

The legacy `lustruct` and `luproperty` macros remain compatibility shims for code that has not migrated. Do not use them for new reflected declarations.

## Registering enumeration type

```c++
#include <Luna/Runtime/Reflection.hpp>
```

For new reflected enumerations, attach `[[Luna::enum]]` to a named enum with a fixed underlying type and attach `[[Luna::option]]` to each value that should be exported:

```c++
enum class [[Luna::enum("{920C8F7F-7CEC-4776-BF01-1F63A4C51D9F}")]] CameraType : u32
{
    perspective [[Luna::option]] = 0,
    orthographic [[Luna::option]] = 1,
};
```

Place the enum in a meta header just like a reflected structure. The generated target registration function registers the enum and its marked options. `register_enum_type`, `EnumerationTypeDesc`, `luenum`, and `luoption` remain available only for low-level and compatibility registration paths.

## Registering generic structure type

```c++
#include <Luna/Runtime/Reflection.hpp>
```

Generic structure type is not itself an instantiable data type, but a *type generator* for generic structure instance types. LunaMetaTool does not process templates, so generic structures are registered with `GenericStructureTypeDesc` and `register_generic_struct_type`.

The most important property of `GenericStructureTypeDesc` is `instantiate`, which is a callback function that generates one generic structure instance type based on type arguments provided:

```c++
GenericStructureInstantiateInfo instantiate(
    typeinfo_t generic_type,
    Span<const GenericArgument> generic_arguments)
```

Each `GenericArgument` contains either a `typeinfo_t` or an integer. The callback returns `GenericStructureInstantiateInfo`, which describes the size, alignment, lifecycle operations, properties, and optional base type of one instantiated structure. `get_generic_instanced_type` creates or retrieves the instance for a particular argument list. The instantiation callback does not return an error; invalid argument combinations should be rejected with an assertion or panic.

The generic structure type and all its instantiated types have the same name and GUID, but each has a unique `typeinfo_t` handle. Use `get_struct_generic_type` to get the family from an instantiated type, `get_struct_generic_arguments` to get its argument span, and `get_struct_generic_parameter_names` to get the declared parameter names.

### Implementing `typeof_t<T>` for generic structure types

The user can implement `typeof_t<T>` using C++ partial template specification like so:

```c++
LUNA_RUNTIME_API typeinfo_t vector_type(); // Returns the generic structure type.
template <typename _Ty> struct typeof_t<Vector<_Ty>>
{
	typeinfo_t operator()() const { return get_generic_instanced_type(vector_type(), { typeof<_Ty>() }); } // Returns the generic structure instanced type.
};
```
