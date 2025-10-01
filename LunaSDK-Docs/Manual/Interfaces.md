Interfaces are C++ structures that only have pure virtual functions. LunaSDK provides most its functionalities through interfaces, so that the implementation detail can be encapsulated and may be different on different platforms.

## Declaring interfaces

```c++
#include <Luna/Runtime/Interface.hpp>
```

To declare one interface, declare one structure with `I` name prefix, and virtually inherit from `Interface` structure. Every interface should have one GUID, which can be declared using `luiid` macro. Methods of the interface is represented by pure virtual functions.

```c++
struct IStream : virtual Interface
{
	luiid("{0345f636-ca5c-4b4d-8416-29834377d239}");

	virtual RV read(void* buffer, usize size, usize* read_bytes = nullptr) = 0;
	virtual RV write(const void* buffer, usize size, usize* write_bytes = nullptr) = 0;
};
```

One interface can inherit `Interface` directly, or it can inherit multiple other interfaces. The behavior is correctly defined since virtual inheritance is used.

```c++
struct ISeekableStream : virtual IStream
{
	luiid("{42F66080-C388-4EE0-9C4D-1EEC1B82F692}");
    
	virtual R<u64> tell() = 0;
	virtual RV seek(i64 offset, SeekMode mode) = 0;
	virtual u64 get_size() = 0;
	virtual RV set_size(u64 sz) = 0;
};
```

## Implementing interfaces

Interfaces can be implemented by declaring structures that inherit from them.

```c++
struct WindowsFile : ISeekableStream
{
    lustruct("WindowsFile", "{95a2e5b2-d48a-4f19-bfb8-22c273c0ad4b}");
	luiimpl();
    
    HANDLE m_file;
    virtual RV read(void* buffer, usize size, usize* read_bytes) override;
    virtual RV write(const void* buffer, usize size, usize* write_bytes) override;
    virtual R<u64> tell() override;
	virtual RV seek(i64 offset, SeekMode mode) override;
	virtual u64 get_size() override;
	virtual RV set_size(u64 sz) override;
};
```

Note that interfaces only work for [[Boxed Objects]]. So the structure type that implements the interface should be registered to the type system either by `register_boxed_type` or by `register_struct_type`, and the object that implements the interface should only be created as boxed objects using `new_object`. LunaSDK also requires you to register interface implementation information to the system, so the registration code for the type above may looks like this:

```c++
register_boxed_type<WindowsFile>();
impl_interface_for_type<WindowsFile, ISeekableStream, IStream>();
```

You can always use `is_interface_implemented_by_type` to check whether one interface is implemented by the specified type.

## Interface conversion

Besides the dynamic casting functionality provided by boxed objects, LunaSDK provides additional functionalities for casting between interface pointers and boxed object pointers safely at run time.

### Casting typed object pointers to interface pointers

Casting typed object pointers to interface pointers can be done directly using `static_cast` or C-style pointer casting, since the boxed type inherits from the interface type by declaration.

### Casting `object_t` to interface pointers

If the underlying type of the interface is not exposed to the user, the user can use `query_interface` to fetch one pointer interface from `object_t`. This function will check whether the specified interface is implemented by the type of the specified object, and returns `nullptr` if not. The returned pointer can be casting to the specified interface type safely by using `static_cast` or C-style pointer casting.

### Casting interface pointers to `object_t` 

Casting interface pointers to `object_t`  can be done by calling `get_object` function of the interface. This function is declared in `Interface` structure, and is implemented by `luiimpl` macro, so all interfaces support this function. The returned type of `get_object` is `object_t`, which is a type-less pointer, the user can then casting the pointer to one typed pointer using [[Boxed Objects#Run-time type identification and dynamic casting|dynamic casting]].

## Smart pointer for interface types

```c++
#include <Luna/Runtime/Ref.hpp>
```

`Ref<T>` and `WeakRef<T>` support interface types. You can use `Ref<IStream>` to refer one boxed object that comforms to `IStream` interface directly. `Ref<T>` handles type conversions automatically, so you can assign `Ref` of any type to each other, and the destination pointer will be set to `nullptr` if type casting fails.