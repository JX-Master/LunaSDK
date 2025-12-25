LunaSDK defines its own memory management functions instead of using those provided by standard libraries. The user should use functions provided by LunaSDK to manage memory when programming with LunaSDK.

## Heap memory allocation and deallocation

```c++
#include <Luna/Runtime/Memory.hpp>
```

The following functions allocate memory blocks in heaps.

| Function                           | Description                                  | C++ STD Equivalent   |
| ---------------------------------- | -------------------------------------------- | -------------------- |
| `memalloc(size, alignment)`        | Allocates memory block.                      | `malloc(size)`       |
| `memfree(ptr, alignment)`          | Frees memory block.                          | `free(ptr)`          |
| `memrealloc(ptr, size, alignment)` | Reallocates memory block.                    | `realloc(ptr, size)` |
| `memsize(ptr, alignment)`          | Gets the size of the allocated memory block. | N/A                  |

You may notice that all heap memory allocation functions provided by LunaSDK takes an `alignment` parameter, which can be used to allocate memory blocks with special address alignment requirements. If you don't have such requirement, simply specify `alignment` as `0` and LunaSDK will use the default alignment requirement for allocating memory blocks, which is `8` on 32-bit platforms and `16` on 64-bit platforms.

### Memory leak detection

LunaSDK comes with an memory leak detection layer that tracks all memory blocks allocated from `memalloc` or `memrealloc`. The memory leak detection layer is disabled by default, you may enable it on xmake menus, or passing `--check_memory_leak=true` when building the SDK. You can use `LUNA_RUNTIME_CHECK_MEMORY_LEAK` macro to determine whether the memory leak detection layer is enabled.

If memory leak detection layer is enabled and unfreed memory blocks are detected when LunaSDK is closing, LunaSDK will print warning messages for each unfreed memory block, including the size and the memory address of the block. If these blocks were allocated using `memnew`, the type of the block will also be printed, so that the user can detect the problem quickly.

## Dynamic object creation and destruction

```c++
#include <Luna/Runtime/Memory.hpp>
```

The following functions creates and destroys dynamic objects.

| Function             | Description                | C++ STD Equivalent |
| -------------------- | -------------------------- | ------------------ |
| `memnew<T>(args...)` | Creates a dynamic object.  | `new T(args...)`   |
| `memdelete(ptr)`     | Destroys a dynamic object. | `delete ptr`       |

## Out of memory (OOM)

Although `memalloc` and `memnew` returns `nullptr` to indicate a failed memory allocation, most functions in LunaSDK do not handle OOM and assumes that the memory allocation will never fail. We treat OOM as an unrecoverable error for the following reasons:

1. Dynamic memory allocation is used in throughout LunaSDK. If we need to handle OOM correctly, the SDK code will become much complex and redundant. It is not worthwhile to pay such effort to handle one error that seldom happens in normal cases.
2. OOM actually never happens on some operating systems, if such system fails to allocate memory, it will simply kill the current process or let the user kill another process to free up some memory.
3. We consider OOM as an optimization problem, not a programming error, so it is improper to "handle" it. If your program suffers from OOM on the target platform, the best thing to do is reducing the memory size consumed by your program, rather than trying to recover from OOM.

## Memory utility library

```c++
#include <Luna/Runtime/MemoryUtils.hpp>
```

Memory utility library provides functions that can be used to manipulate memory data easily. You can check the docs for each function for their usages.

`_kb`, `_mb`, `_gb`, `_tb` are [integer literals](https://en.cppreference.com/w/cpp/language/user_literal) that can be used to define byte sizes clearly. For example, you can use `100_mb` to represent `100 * 1024 * 1024`, and they have the same meaning.

`memcpy`, `memcmp`, `memset`, `memmove` are memory manipulating functions provided by the C/C++ standard library. They can be used in LunaSDK as well. `memzero` is used to fill one range of memory with value `0`, it is equivalent to calling `memset` with value `0`.

`memcpy_bitmap` and `memcpy_bitmap3d` are used to copy binary data between two row-major 2D and 3D bitmaps. `pixel_offset` is used to fetch the address of one particular pixel in a row-major 2D or 3D bitmap. These functions can be useful when dealing with bitmap data.

`align_upper` increases the input size or address number to the nearest number that is a multiple of the alignment number.

`bit_test`, `bit_set`, `bit_reset` tests, sets and resets one specific bit on the given memory address. These functions can be useful when performing bitwise operations.

`addressof` returns the real address of one object, even if the `operator&` of the object has been overloaded.

`default_construct`, `value_construct`, `copy_construct`, `move_construct` and `direct_construct` performs object initialization on the object pointed by the specified iterator/pointer. `destruct` performs object destruction on the object pointed by the specified iterator/pointer.

`copy_assign` and `move_assign` perform copy assignment and move assignment on two objects pointed by the specified iterators/pointers.

`default_construct_range`, `value_construct_range`, `copy_construct_range` and `move_construct_range` performs object initialization on objects in the range specified by two iterators/pointers. `destruct_range` performs object destruction on objects in the range specified by two iterators/pointers.

`copy_assign_range`,  `move_assign_range` and `move_assign_range_backward` performs 

copy assignment and move assignment on objects in the range specified by two iterators/pointers.

`fill_construct_range` and `fill_assign_range` calls the copy constructor and copy assignment operator on objects with the specified instance.

`copy_relocate_range`, `copy_relocate`, `move_relocate_range` and `move_relocate_range_backward` relocates objects in the range specified by two iterators/pointers to another continuous range, preserving the order of objects. If the object is trivially relocatable, this function will perform memory copy and does not invoke any move constructor; if the object is not trivially relocatable, this call performs move construction on the new address, and destruction on the old address.