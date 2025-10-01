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

Every registered type can be identified by name or by GUID, you can get one type object from its name by calling `get_type_by_name`, and from its GUID by calling `get_type_by_guid`. The name and GUID of one type object can be fetched by calling `get_type_name` and `get_type_guid`.

Every type must have one unique GUID, but multiple types may have the same name. If multiple types have the same name, each of them should have one unique alias so that it can be differed from others. If the type is defined in namespaces, its namespace should be appended before the type name, separated by double colons (`::`).

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

Generic structure type represents one structure type with generic type parameters, such as `Vector<T>`. The number of generic type parameters can be uncertain, like `Tuple<T1, T2, ...>`.

Generic structure types cannot be used directly, they must be instantiated to a *generic structure instanced type* by calling `get_generic_instanced_type`. The generic instantiation process is happened at run time, every generic instanced type with one particular set of generic structure type and generic type parameters will be instantiated only once, and the instantiated type will be reused. One generic structure instanced type can be used just as one normal structure type.

## Registering structure type

```c++
#include <Luna/Runtime/Reflection.hpp>
```

There are two methods to register one structure type. The first method is simpler and can be used for most cases, the second method is non-intrusive can be used if the structure is defined in another module or third-party library and cannot be changed directly.

### The first method

The first method is to insert one `lustruct` macro in your structure definition, specifying the name and GUID of the structure.

```c++
struct SpotLight
{
	lustruct("SpotLight", "{2BB45396-E0E3-433E-8794-49BEE8BD1BB5}");
	Float3 intensity = { 0.5f, 0.5f, 0.5f };
	f32 intensity_multiplier = 1.0f;
	f32 attenuation_power = 1.0f;
	f32 spot_power = 64.0f;
};
```

Then you can call `register_struct_type<T>` to register the type. The properties of the type can be specified quickly using `luproperty` macro:

```c++
register_struct_type<SpotLight>({
	luproperty(SpotLight, Float3, intensity),
	luproperty(SpotLight, f32, intensity_multiplier),
	luproperty(SpotLight, f32, attenuation_power),
	luproperty(SpotLight, f32, spot_power)
	});
```

If the structure type has base type, the base type should be specified as the second argument, after the property list.

### The second method

In the second method, the user should fill one `StructureTypeDesc` structure, then call `register_struct_type` to register the type. For example, the following code registers `Name` type into the system.

```c++
StructureTypeDesc desc;
desc.guid = Guid("{E5EEA2C6-2D51-4658-9B3F-C141DDE983D8}");
desc.name = "Name";
desc.alias = "";
desc.size = sizeof(Name);
desc.alignment = alignof(Name);
desc.base_type = nullptr;
desc.ctor = nullptr;
desc.dtor = default_dtor<Name>;
desc.copy_ctor = default_copy_ctor<Name>;
desc.move_ctor = default_move_ctor<Name>;
desc.copy_assign = default_copy_assign<Name>;
desc.move_assign = default_move_assign<Name>;
desc.trivially_relocatable = true;
typeinfo_t type = register_struct_type(desc);
```

After the type is registered, the user should also implement `typeof_t<T>` structure for the type like so:

```c++
// In .hpp file:
LUNA_MYMODULE_API typeinfo_t get_my_type();
template <> struct typeof_t<MyType>
{
	typeinfo_t operator()() const { return get_my_type(); }
};

// In .cpp file:
typeinfo_t g_my_type;
LUNA_XXX_API typeinfo_t get_my_type() { return g_my_type; }
```

## Registering enumeration type

```c++
#include <Luna/Runtime/Reflection.hpp>
```

The user can use `register_enum_type` function and `luoption` macro to register one enumeration type. For example, if we have the following type:

```c++
enum class CameraType : u32
{
	perspective = 0,
	orthographic = 1,
};
```

The registration code will be:

```c++
register_enum_type<CameraType>({
	luoption(CameraType, perspective),
	luoption(CameraType, orthographic)
});
```

Since enumeration types cannot include static variables, the GUID of the enumeration type must be declared separately using `luenum` like so:

```c++
luenum(CameraType, "CameraType", "{920C8F7F-7CEC-4776-BF01-1F63A4C51D9F}");
```

`luenum` must be defined directly in `Luna` namespace, not the sub-namespace of `Luna` namespace.

## Registering generic structure type

```c++
#include <Luna/Runtime/Reflection.hpp>
```

Generic structure type is not actually a real type, but a *type generator* for generic structure instance types. To register one generic structure type, the user should fill one `GenericStructureTypeDesc` structure, and call `register_generic_struct_type` to register the generic structure type.

The most important property of `GenericStructureTypeDesc` is `instantiate`, which is a callback function that generates one generic structure instance type based on type arguments provided:

```c++
GenericStructureInstantiateInfo instantiate(typeinfo_t generic_type, const typeinfo_t* generic_arguments, usize num_generic_arguments)
```

This function should returns one `GenericStructureInstantiateInfo` structure, which is similar to `StructureTypeDesc` and describes one generic structure instanced type. The generic structure instanced type is then registered to the system can will be returned by `get_generic_instanced_type`. The instantiation function never fails, if the instantiation function cannot handle the input type arguments, it should call `lupanic_msg` to crash the program.

The base generic structure type and all its instanced types will have the same name and GUID, but each of them will have a unique `typeinfo_t` handle. You can get the type arguments of one generic structure instanced type by calling `count_struct_generic_arguments` and `get_struct_generic_argument`.

### Implementing `typeof_t<T>` for generic structure types

The user can implement `typeof_t<T>` using C++ partial template specification like so:

```c++
LUNA_RUNTIME_API typeinfo_t vector_type(); // Returns the generic structure type.
template <typename _Ty> struct typeof_t<Vector<_Ty>>
{
	typeinfo_t operator()() const { return get_generic_instanced_type(vector_type(), { typeof<_Ty>() }); } // Returns the generic structure instanced type.
};
```