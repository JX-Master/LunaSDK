This article describes how we exports C++ entities to C#, so that C++ codes and C# codes can call each other.

Since .NET only supports C ABI for native platform invoking (P/Invoke), we cannot export C++ entities directly to C#. Instead, we use a **C wrapper layer** to encapsulate all C++ entities using opaque pointers and pure C enumerations, structures and functions. Such C wrapper layer are located in {SDK_ROOT}/CWrapper directory, every exported module has one subdirectory in that directory.

### Exporting C++ functions
C++ functions are exported by declaring C equivalent functions in C wrapper layer. Such C functions must be exported as symbols, so that they can be imported by C# runtime:
```c
LUNA_RUNTIME_C_API void luna_runtime_log(uint32_t verbosity, const char* tag, const char* message);
```
and in C#:
```c#
[DllImport(LibraryName, EntryPoint = "luna_runtime_log")]
internal static extern void Log(
    uint verbosity,
    [MarshalAs(UnmanagedType.LPUTF8Str)] string tag,
    [MarshalAs(UnmanagedType.LPUTF8Str)] string message);
```
consult C# and .NET docs for more information about type marshaling in C ABI.
### Exporting C++ structures
C++ structures are exported by declaring a C equivalent type in C wrapper layer. For example, the following C++ code
```c++
namespace Luna
{
    struct DateTime
    {
        //! The year since 1 BC. 0 means 1 BC, -1 means 2 BC, 2022 means AD 2022.
        i16 year;
        //! The month [1-12]
        u8 month;
        //! The month of day [1-31]
        u8 day;
        //! The hour [0-23]
        u8 hour;
        //! The minute [0-59]
        u8 minute;
        //! The second [0-60]. Mostly [0-59], 60 for leap second.
        u8 second;
        //! The day of week (0: Sunday, 1: Monday, 2: Tuesday, 3: Wednesday, 4: Thursday, 5: Friday, 6: Saturday).
        u8 day_of_week;
    };
}
```
will have C equivalent type in C wrapper layer:
```c
typedef struct LunaDateTime
{
    int16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t day_of_week;
} LunaDateTime;
```
then we define the same structure in C#, matching the memory layout of C types:
```c#
[StructLayout(LayoutKind.Sequential)]
public readonly struct DateTime
{
    public DateTime(short year, byte month, byte day, byte hour, byte minute, byte second, byte dayOfWeek)
    {
        Year = year;
        Month = month;
        Day = day;
        Hour = hour;
        Minute = minute;
        Second = second;
        DayOfWeek = dayOfWeek;
    }

    public readonly short Year;
    public readonly byte Month;
    public readonly byte Day;
    public readonly byte Hour;
    public readonly byte Minute;
    public readonly byte Second;
    public readonly byte DayOfWeek;

    public override string ToString()
    {
        return $"{Year:D4}-{Month:D2}-{Day:D2} {Hour:D2}:{Minute:D2}:{Second:D2}";
    }
}
```
note that by default C# may reorder members in memory to optimize type size, use `[StructLayout(LayoutKind.Sequential)]` to enforce C# to use the exact same member order declared by the type for memory layouts.

After the structure is mapped, it can be used directly in the C ABI exported to C#:
```c
LUNA_RUNTIME_C_API void luna_runtime_time_timestamp_to_datetime(int64_t timestamp, LunaDateTime* out_datetime);
LUNA_RUNTIME_C_API int64_t luna_runtime_time_datetime_to_timestamp(const LunaDateTime* datetime);
```
and in C#:
```c#
[DllImport(LibraryName, EntryPoint = "luna_runtime_time_timestamp_to_datetime")]
internal static extern void TimeTimestampToDateTime(long timestamp, out DateTime dateTime);

[DllImport(LibraryName, EntryPoint = "luna_runtime_time_datetime_to_timestamp")]
internal static extern long TimeDateTimeToTimestamp(in DateTime dateTime);
```
### Exporting C++ boxed objects
[[Boxed objects]] are typed memory blocks managed by C++ side with the following features:
1. Object lifetime is managed by reference counting, which requires calling `retain` and `release` explicitly to increase and decrease the reference counter. The object is freed automatically when the reference count drops to 0.
2. Boxed objects support RTTI, which enables runtime querying for the real type (and all base types) of one object. Supported interfaces can also be queried form the object.

Boxed objects are exported by passing `object_t` pointer directly through C wrapper. The pointer is kept in C# as an `IntPtr`, which is opaque for C#. C wrapper also exports all functions to operate on `object_t`, like `luna_runtime_object_retain`, `luna_runtime_object_release`, `luna_runtime_object_is_type`, etc, which is used by C# to provide full functionality for managing boxed objects.

In C# side, since .NET uses garbage collecting to manage object lifetime, the time when object is freed is not certain. To prevent object from releasing too late, we use `ObjectBase` class in C# to represent one boxed object. `ObjectBase` implements `IDisposable`, which supports releasing the boxed object immediately by calling `Dispose` method. The user should call `Dispose` as soon as the object is no longer needed. `ObjectBase` also has `GetNativeHandle()` method, which returns the underlying `object_t` pointer without reference counter being modified, and `IsA()` method to check the C++ type of the underlying object.

### Exporting C++ interfaces
[[Interfaces]] are exported to C# through **opaque interface pointer** and C functions. For example:
```c++
struct IStream : virtual Interface
{
    luiid("{0345f636-ca5c-4b4d-8416-29834377d239}");

    virtual RV read(void* buffer, usize size, usize* read_bytes = nullptr) = 0;
    virtual RV write(const void* buffer, usize size, usize* write_bytes = nullptr) = 0;
};
```
Will be exported to C wrapper as:
```c
LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_stream_read(void* self, void* buffer, uint64_t size, uint64_t* out_read_bytes);
LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_stream_write(void* self, const void* buffer, uint64_t size, uint64_t* out_write_bytes);
```
the first parameter of C functions are always `void* self`, which is the opaque interface pointer that can be reinterpret-casted to `IStream*`, so these two functions can be implemented like so:
```c++
LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_stream_read(void* self, void* buffer, uint64_t size, uint64_t* out_read_bytes)
{
    Luna::usize read_bytes = 0;
    auto result = reinterpret_cast<Luna::IStream*>(self)->read(buffer, static_cast<Luna::usize>(size), &read_bytes);
    if (out_read_bytes)
    {
        *out_read_bytes = static_cast<uint64_t>(read_bytes);
    }
    return from_result(result);
}

LUNA_RUNTIME_C_API luna_errcode_t luna_runtime_stream_write(void* self, const void* buffer, uint64_t size, uint64_t* out_write_bytes)
{
    Luna::usize write_bytes = 0;
    auto result = reinterpret_cast<Luna::IStream*>(self)->write(buffer, static_cast<Luna::usize>(size), &write_bytes);
    if (out_write_bytes)
    {
        *out_write_bytes = static_cast<uint64_t>(write_bytes);
    }
    return from_result(result);
}
```

In order to call such functions in C#, the C# code need to get the interface pointer from `object_t` by calling `ObjectQueryInterface`, which is `luna_runtime_object_query_interface` in C:
```c
LUNA_RUNTIME_C_API void* luna_runtime_object_query_interface(luna_handle_t object, const LunaGuid* iid);
```
The returned pointer is stored as `IntPtr` and can be passed to interface functions. Note that interfaces do not responsible for object lifetime management, the code should only use interface pointer when it already owns `object_t` handle to prevent UAF cases.

In order to replicate C++ coding experience, most interfaces have equivalent in C#:
```c#
namespace Luna.Runtime;

public interface IStream : IObject
{
    ulong Read(byte[] buffer);

    ulong Read(byte[] buffer, int offset, int count);

    ulong Write(byte[] buffer);

    ulong Write(byte[] buffer, int offset, int count);
}
```

However, such C# interfaces are different from C++ interfaces, they are usually implemented by routing the call to corresponding interface functions in C++, but you should not confuse C# interfaces with C++ interfaces (which is just an opaque pointer and a set of native functions in C# domain).

