This section discusses the coding conventions used in LunaSDK. All users using LunaSDK should follow these conventions to achieve a consistent code style and prevent programming errors.

## Disabled C++ features

The following C++ features are disabled in LunaSDK:

1. Exceptions (`try`, `throw`, `catch`).
2. Real-time type identification (RTTI, `dynamic_cast` and `typeid` for objects). The use of `typeid` for static types is allowed.

In rare cases, if you have to use these features (like integrating one third-party library that uses these features), make sure these features are used internally in your module, and not cross the module interface, or they may not be handled correctly.

The `noexcept` decorator is not used in LunaSDK, since exceptions are disabled by default.

## File naming conventions

Use `.hpp` file extension name for C++ header files, use `.cpp` file extension name for C++ source files, use `.inl` file extension name for inlined C++ source files.

Use Pascal case for file and folder names, like `FileIterator.hpp`. Do not add interface prefix `I` to the filename.

Add the following comments at the beginning of every C++ header, source or inlined source file if you want to contribute it as part of LunaSDK.

```c++
/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file {The filename of this file}
* @author {Author name}
* @date {The file creation date, YYYY/MM/DD}
*/
```

## Lexical formats

### Indents

One indent can be represented by one tab character or four whitespace characters. Both forms are allowed.

### Scopes

In most cases, scope opening and closing brackets should occupy a whole line. Nested scopes should be properly indented.

```c++
namesapce A
{
	namespace B
	{
		class C
		{
			
		};
	}
}
```

However, if the scope is empty or contains simple statements, you may write the opening and coding brackets without beginning a new line:

```c++
class A
{
	i32 n;
public:
	A() {} // Empty scope.
	i32 get_n() { return n; }	// Simple scope.
}
```

Always use the form that maximizes the readability when deciding scoping forms.

### Documenting comments

Use [doxygen comment blocks](https://www.doxygen.nl/manual/docblocks.html) for documenting comments (comments that exists in the header file as the documentation of function, type or object).

```c++
//! Resets the task context and begins a new task.
//! @param[in] world The world to run the task on.
//! @param[in] exec_mode The task execution mode.
//! @param[in] read_components If task mode is `shared`, specify components that will be read by this task.
//! @param[in] write_components If task mode is `shared`, specify components that will be read and written by
//! this task.
//! @remark This call may block the current thread until all components required by this task can be safely
//! accessed by this task, or until all other tasks are finished if this is a exclusive task.
virtual void begin(
    IWorld* world,
    TaskExecutionMode exec_mode,
    Span<typeinfo_t> read_components,
    Span<typeinfo_t> write_components
) = 0;
```

## Naming conventions

### Primitive types

Primitive types (numbers, characters, pointers) and the aliasing primitive types are named using the underscore case:

* `u8`
* `i32`
* `usize`

For all user-defined aliasing types of primitive types, uses `_t` suffix:

* `using opaque_t = void*`

### Enumeration types

Use `enum classs` instead of `enum` for all enumerations. All enumeration types are named using the Pascal case. Options of the enumeration are named using the underscore case.

```c++
enum class ResourceType : u8
{
	buffer,
	texture_1d,
	texture_2d,
	texture_3d
};
```

If the enumeration represents a single-value type, the use of `Type` or `Kind` suffix is suggested, but not required; if the enumeration represents a bit-OR-combined multi-value type, the use of `Flag` suffix is suggested, but not required. For all multi-value enumeration types, always provides one option `none` with the value `0`. Using hexadecimal form to represent multi-value enumeration option values is suggested, but not required.

```c++
enum class ResourceUsageFlag : u32
{
	none = 0x00,
	shader_resource = 0x01,
	constant_buffer = 0x02,
	unordered_access = 0x04,
	render_target = 0x08,
	depth_stencil = 0x10,
	vertex_buffer = 0x20,
	index_buffer = 0x40,
	stream_output = 0x80,
	indirect_buffer = 0x100,
};
```

### Structure and class types

Types defined by `struct` and `class` are considered the same in LunaSDK, we use "structure type" to refer both.

Most structure types are named using the Pascal case.

```c++
struct ResourceDesc
{
	ResourceType type;
	ResourceHeapType heap_type;
	Format pixel_format;
	ResourceUsageFlag usages;
	u64 width_or_buffer_size;
	u32 height;
	u32 depth_or_array_size;
	u32 mip_levels;
	u32 sample_count;
	u32 sample_quality;
	ResourceFlag flags;
};
```

The only exception is the structure type that:

1. contains only one primitive typed member object.
2. can be trivially constructed, destructed, copied and moved.
3. can be constructed by providing one value with the same type of its member variable, and the behavior is assigning its member object with the provided value.

In such case, we consider the structure type as a aliasing type of the primitive type, thus use the underscore case with `_t` suffix for the type:

```c++
struct asset_t
{
	opaque_t handle;
	asset_t() :
		handle(nullptr) {}
	constexpr asset_t(opaque_t handle) :
		handle(handle) {}
	constexpr bool operator==(const asset_t& rhs) const
	{
		return handle == rhs.handle;
	}
	constexpr bool operator!=(const asset_t& rhs) const
	{
		return handle != rhs.handle;
	}
	operator bool() const { return handle != nullptr; }
};
```

Note that using the Pascal case for such structure type is also allowed.

#### Interface structure types

If the structure type represents an interface, append `I` prefix to the structure type name.

```c++
struct IStream : virtual Interface
{
	luiid("{0345f636-ca5c-4b4d-8416-29834377d239}");

	virtual RV read(void* buffer, usize size, usize* read_bytes = nullptr) = 0;
	virtual RV write(const void* buffer, usize size, usize* write_bytes = nullptr) = 0;
};
```

### Functions

All functions, including member functions of structure types, are named using the underscore case. Function parameters are also named using the underscore case.

```c++
RV read_file(opaque_t file, void* buffer, usize size, usize* read_bytes = nullptr);
```

For functions with long parameter lists, you can separate parameter lists into multiple lines, providing the parameter list line is correctly indented.

```c++
void draw_shape(u32 begin_command, u32 num_commands,
	const Float2U& min_position, const Float2U& max_position,
	const Float2U& min_shapecoord, const Float2U& max_shapecoord,
	u32 color = 0xFFFFFFFF,
	const Float2U& min_texcoord = Float2U(0.0f), 
    const Float2U& max_texcoord = Float2U(0.0f));
```

These rules are also applied to member functions.

### Objects

All objects except global constants are named using the underscore case, including member object of structure types.

```c++
Float2U origin_point;
```

For member objects of structure types that are not exposed as part of module interface, the prefix `m_` is suggested, but not required.

```c++
struct ComponentBuffer
{
	typeinfo_t m_type;
	void* m_data = nullptr;
	usize m_size = 0;
	usize m_capacity = 0;
};
```

For global variables that are not exposed as part of module interface, the prefix `g_` is suggested, but not required.

```c++
Ref<IFontFile> g_default_font;
```

Global constants are named using uppercase words separated by underscores, and is decorated by `constexpr`.

```c++
constexpr f32 PI = 3.141592654f;
constexpr entity_id_t NULL_ENTITY(0);
```

Prevent defining global constants using macros.

### Namespace

All namespaces are named using the Pascal case.

```c++
namespace Luna
{
	namespace RHI
	{
	}
}
```

All LunaSDK components are defined in `Luna` namespace, every LunaSDK module except `Runtime` should have its own namespace under `Luna` containing its own components.

### Macros

Macros can be declared using two forms. The first form is uppercase words separated by underscores, with `LUNA_` prefix. These macros are usually used for conditional compiling and replacing some platform-dependent keywords.

```c++
#define LUNA_DEBUG		1
#define LUNA_PROFILE	1
#define LUNA_RELEASE	1

#if defined(LUNA_COMPILER_MSVC)
    #define LUNA_DLL_EXPORT __declspec(dllexport)
#else
    #define LUNA_DLL_EXPORT __attribute__ ((visibility("default")))
#endif
```

The second form is the underscore case with `lu` prefix. These macros are usually used to replace some code patterns to improve coding efficiency.

```c++
#define luassert(_condition) //...
#define luassert_msg(_condition, _message) //...
#define lustruct(_name, _guid) //...
#define luproperty(_struct, _type, _name) //...
#define luenum(_type, _name, _guid) //...
#define luoption(_enum, _item) //...
#define lucatchret //...
```

Macro parameters are allowed for both macro forms. Macro parameter names should be prefixed with one underscore character.

### Templates

Template type parameters should be decorated with `typename` instead of `class`, and should be prefixed with one underscore character.

```c++
template <typename _Ty>
struct less
{
	constexpr bool operator()(const _Ty& lhs, const _Ty& rhs) const
	{
		return lhs < rhs;
	}
};
```