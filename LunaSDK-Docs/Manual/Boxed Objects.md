Boxed objects are dynamic allocated objects managed by LunaSDK, so that they have the following features:

1. The lifetime of boxed objects are managed by reference-counting.
2. Real-time type identification (RTTI) can be used to check the type of boxed objects, and can be used to perform dynamic type casting safely.
3. Boxed objects can implement [[Interfaces]].

To implement such features, every boxed object will have one *Object Header* allocated along with the object data, and is used to record the object metadata like type and reference counter. Boxed objects should also be property referred using smart pointers (`Ref<T>` for typed boxed object and `ObjRef` for type-less boxed object) so that their reference counter can be properly maintained.

## Registering boxed type

```c++
#include <Luna/Runtime/Object.hpp>
```

One type must be registered to the [[Type System]] to be used for creating boxed objects. If you want to register one type solely for creating boxed objects, you may use `register_boxed_type` instead of calling `register_struct_type` directly.

## Managing boxed object manually

```c++
#include <Luna/Runtime/Object.hpp>
```

Use `object_alloc` to allocate one boxed object. This call will allocate memory for the boxed object and the object header, initialize the object header, and returns one pointer to the allocated boxed object as `object_t`, which is an aliasing type of `void*`. The return pointer can be reinterpreted to the pointer of the required type directly, and it should be passed to all other boxed object management APIs. The object returned by `object_alloc` is not initialized, the user should then call constructors of the specified type manually to construct the object.

Boxed objects implement both strong reference counting and weak reference counting. Use `object_retain`, `object_release` to increase and decrease the strong reference counter, and `object_retain_weak`, `object_release_weak` to increase and decrease the weak reference counter. One object will have `1` strong reference and `0` weak reference when allocated. If the strong reference counter value drops to `0`, the destructor of the object will be called. If the weak reference counter is not `0` when the object is being destructed, the object will be marked as *expired*, you can call `object_expired` to check whether one object is expired. One expired object cannot be used, the only valid operation for it is to release its weak references. The memory for one boxed object will be freed if both the strong and weak reference counter values drop to `0`.

## Managing boxed object automatically

```
#include <Luna/Runtime/Ref.hpp>
```

In most of the time, you don't need to manage boxed object manually. You can use `new_object` to create one boxed object directly like `memnew`, this function allocates one boxed object, and initializes it using user-provided arguments. `new_object` returns one `Ref<T>` smart pointer, which represents one strong reference to the object. There are four smart pointers provided by LunaSDK:

* `Ref<T>` for strong references to typed boxed objects.
* `ObjRef` for strong references to type-less boxed objects, which can refer to any boxed object.
* `WeakRef<T>` for weak references to typed boxed objects.
* `WeakObjRef` for weak references to type-less boxed objects.

All smart pointers decrease the reference counter value automatically when being destructed, so the user does not need to handle this manually. Coping one smart pointer object only increase the reference counter value of the object, the object itself is not copied. You can create one weak reference smart pointer object by casting from one strong reference smart pointer directly, but you should call `pin` on one weak reference smart pointer to fetch one strong reference smart pointer from it, which will return `nullptr` if failed. The weak smart pointer will be reset to `nullptr` automatically when the object is expired and the user calls `get` on the smart pointer.

## Run-time type identification and dynamic casting

```c++
#include <Luna/Runtime/Object.hpp>
```

LunaSDK uses `object_t` to represent one type-less pointer to one boxed object. It is not safe to cast one `typeinfo_t` to one concrete typed pointer without checking whether the object type conforms to the pointer type specified. LunaSDK provides run-time type identification (RTTI) for all boxed objects to perform type casting safely at run time.

Use `get_object_type` on `object_t` to fetch the real type of the object. This function returns one `typeinfo_t` directly, so it is suitable if you want to inspect the type to perform some special operations. 

Use `object_is_type` to check whether the given object conforms to the specified type, that is, either the object is the specified type, or the object is one type that derives from the specified type. Use this function if you want to perform dynamic casting safely like `dynamic_cast` (which cannot be used in LunaSDK).

Any typed pointer to one boxed object can be casted to `object_t` without any run-time cost.