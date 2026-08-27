LunaSDK does not use C++ exceptions. Operations that can fail return stable result codes, usually through `R<T>` or `RV`. A result code can be constructed, compared, returned, and transferred between processes without initializing LunaSDK. The runtime error information system is optional and only adds diagnostic metadata.

## Functionality

The error handling facilities provide four related features:

1. `ResultCode` is a stable 64-bit result identity. Its value is fixed at compile time and is independent of process-local state.
2. `R<T>` carries either a value and a successful result code, or a failure result code.
3. The runtime metadata registry associates optional names, descriptions, and categories with static result codes.
4. `Error` attaches a formatted message and arbitrary `Variant` information to a failure on the current thread.

Static result codes are available before `Luna::init`. Metadata registration and rich thread-local `Error` objects are runtime services and require Runtime initialization.

## Concepts

### Result code

```c++
#include <Luna/Runtime/Error.hpp>
```

`ResultCode` wraps a `u64` so that a result code cannot be confused with an ordinary integer returned by `R<T>`. The complete value has this layout:

```text
63                         32 31             16 15              0
+----------------------------+-----------------+-----------------+
|       domain ID: u32       | category ID:u16 | result value:i16|
+----------------------------+-----------------+-----------------+
```

The low 16 bits store the bit pattern of a signed 16-bit result value:

| Result value | Meaning |
| ---: | --- |
| -32768 through -1 | Failure |
| 0 | Successful result |
| 1 through 32767 | Successful result with status information |

The result value determines success or failure. The complete 64-bit value determines whether a successful result is plain or informative:

- `0x0000000000000000` is the only plain success result.
- Every other code whose result value is non-negative is an informative success.
- Therefore, a code with a non-zero domain or category and a result value of 0 is an informative success.

Do not test `ResultCode::code` for zero to distinguish success from failure. Use these `constexpr` functions:

```c++
constexpr bool succeeded(ResultCode code);
constexpr bool failed(ResultCode code);
constexpr bool is_plain_success(ResultCode code);
constexpr bool is_informative_success(ResultCode code);
```

Codes and fields are constructed and decoded with:

```c++
constexpr ResultCode make_error_code(u32 domain_id, u16 category_id, i16 result);
constexpr u32 get_error_code_domain(ResultCode code);
constexpr u16 get_error_code_category_id(ResultCode code);
constexpr i16 get_error_code_result(ResultCode code);
```

These functions do not access runtime state and can be used in constant evaluation.

### Domain ID

The high 32 bits identify the project or ownership domain. Domain IDs have two allocation regions:

| Range | Allocation policy |
| --- | --- |
| `0x00000000`–`0x7FFFFFFF` | Registered by the LunaSDK project team |
| `0x80000000`–`0xFFFFFFFF` | Self-allocated; collisions are possible |

Domain 0 belongs to LunaSDK. A project that needs a globally coordinated public domain should request an ID from the LunaSDK project team.

A private, experimental, or independently distributed project may generate an ID in the self-allocated region. Generate it once using a random or hash-based ID generator, set the high bit, commit the chosen value to source control, and never regenerate or reuse it. The self-allocated region removes the coordination requirement but does not guarantee uniqueness.

`is_registered_error_domain` and `is_self_allocated_error_domain` test which region contains an ID.

### Category ID

Bits 31 through 16 identify a category owned by the domain. Each domain defines its own category allocation policy. A category normally represents a module or error-producing subsystem. Published category IDs must never be reassigned.

`errcat_t` is a stable category key retained by the public API. It packs a domain ID and category ID, but no result value:

```c++
constexpr errcat_t make_error_category(u32 domain_id, u16 category_id);
constexpr u32 get_error_category_domain(errcat_t category);
constexpr u16 get_error_category_id(errcat_t category);
constexpr errcat_t get_error_code_category(ResultCode code);
```

`INVALID_ERROR_CATEGORY` is the reserved invalid category sentinel. Categories are flat numeric identities; the former hierarchical subcategory API has been removed.

LunaSDK permanently assigns these category IDs in domain 0:

| ID | Constant | Owner |
| ---: | --- | --- |
| 0 | `LunaErrorCategory::RUNTIME` | Runtime |
| 1 | `LunaErrorCategory::AHI` | AHI |
| 2 | `LunaErrorCategory::ASSET` | Asset |
| 3 | `LunaErrorCategory::ECS` | ECS |
| 4 | `LunaErrorCategory::FRONTEND` | Frontend |
| 5 | `LunaErrorCategory::IMAGE` | Image |
| 6 | `LunaErrorCategory::NETWORK` | Network |
| 7 | `LunaErrorCategory::RHI` | RHI |
| 8 | `LunaErrorCategory::VFS` | VFS |

### Local result value

The low signed 16-bit value is allocated by the category owner. Negative values identify failures. Non-negative values identify successful statuses. Once published, a value must not be reassigned to a different meaning, even after its original meaning is deprecated.

The tuple `(domain ID, category ID, result value)` is the complete stable identity. The same local value can be used by different categories without collision.

### Plain and informative success

`ok` is an `RV` containing the all-zero plain success code. It means that the operation completed without additional status information.

An informative success reports a condition that callers may care about without invalidating the returned value. Examples include returning cached data, reporting a fallback path, or completing only the work requested by an optional mode. Callers that do not need the status can continue to treat the result as successful.

The special rule for a local result value of 0 is important:

```c++
static_assert(is_plain_success(make_error_code(0, 0, 0)));
static_assert(is_informative_success(make_error_code(1, 0, 0)));
static_assert(is_informative_success(make_error_code(0, 1, 0)));
```

Only the first code is plain success.

### Runtime metadata

Names and descriptions are diagnostic metadata, not the identity of a result. The runtime registry associates a static category and code with:

- a category name;
- a result name;
- a brief result description.

The registry never creates numeric result codes. Missing metadata does not invalidate a code or change control flow. Metadata is queried from a static code; the registry does not support name-to-code lookup. LunaSDK registers each category using its canonical module name, such as `"Runtime"`, `"RHI"`, or `"Network"`.

### Rich error object

`Error` adds contextual information to one failure:

1. `code` contains the underlying static result code.
2. `message` contains a UTF-8 explanation specific to the failed operation.
3. `info` contains optional structured `Variant` data.

`get_error` returns the current thread's runtime `Error` object. The object is thread-local and is not automatically transferred between threads or processes.

## Programming guide

### Declaring a domain, category, and result codes

Declare public codes as documented `inline constexpr` constants in a public header. The following domain ID is illustrative only; a real project must obtain or generate its own permanent ID.

Prefix every static result constant according to its signed local result value:

- Use `E_` for a negative failure result.
- Use `S_` for a non-negative successful result, including a local result value of 0 when the complete code is non-zero.

The prefix makes the success or failure class visible at each use and avoids collisions with common system macros. Do not append `_ERROR` merely to avoid such a collision; for example, use `E_NO_DATA` rather than `NO_DATA_ERROR`. Metadata names such as `"no_data"` remain unprefixed diagnostic identifiers. Use `ok`, not a named result-code constant, for ordinary all-zero success.

```c++
namespace Luna::MyModule
{
    //! The permanent project domain ID used by this example.
    inline constexpr u32 DOMAIN = 0x8ABCDE01;
    //! The category ID allocated to this module by the project.
    inline constexpr u16 CATEGORY_ID = 1;
    //! The stable category key.
    inline constexpr errcat_t ERROR_CATEGORY = make_error_category(DOMAIN, CATEGORY_ID);

    //! The requested record was not found.
    inline constexpr ResultCode E_RECORD_NOT_FOUND = make_error_code(DOMAIN, CATEGORY_ID, -1);
    //! The returned value came from a cache.
    inline constexpr ResultCode S_CACHE_HIT = make_error_code(DOMAIN, CATEGORY_ID, 1);
    //! Successful category-specific status whose local result value is zero.
    inline constexpr ResultCode S_CATEGORY_STATUS = make_error_code(DOMAIN, CATEGORY_ID, 0);
}
```

Changing any published field changes the result identity and is an API/ABI protocol break. Add a new value instead of renumbering an existing value.

Declare result constants and `ERROR_CATEGORY` directly in the owning module's root namespace. Do not add a separate namespace such as `MyModuleError`. Runtime constants are declared directly in `Luna`, so common failures are referenced as `E_BAD_ARGUMENTS`, `E_NOT_FOUND`, and `E_OUT_OF_MEMORY` from within that namespace. Other modules qualify their constants with the module namespace, such as `RHI::E_DEVICE_REMOVED`.

### Registering optional metadata

Register metadata after Runtime initialization, normally by calling a helper like this from the owning module's `on_register` function:

```c++
#include <Luna/Runtime/Result.hpp>

Luna::RV register_my_module_error_metadata()
{
    using namespace Luna;
    if(!register_error_category(MyModule::ERROR_CATEGORY, "MyModule") ||
        !register_error_code(MyModule::E_RECORD_NOT_FOUND, "record_not_found", "The requested record was not found.") ||
        !register_error_code(MyModule::S_CACHE_HIT, "cache_hit", "The returned value came from a cache.") ||
        !register_error_code(MyModule::S_CATEGORY_STATUS, "category_status", "The operation returned category-specific status."))
    {
        return set_error(E_ALREADY_EXISTS, "MyModule metadata conflicts with an existing registration.");
    }
    return ok;
}
```

Registration is idempotent when all supplied information is identical. It fails when an ID is assigned different metadata or when a name is reused for another code in the same category.

Runtime and registered modules remove their metadata during normal Runtime shutdown. Static constants remain valid after shutdown; only metadata queries and rich error services become unavailable.

The following functions inspect registered metadata:

```c++
const c8* get_error_code_name(ResultCode code);
const c8* get_error_code_description(ResultCode code);
const c8* get_error_category_name(errcat_t category);
```

Use constants for program control flow. Use names and descriptions for logs, tools, diagnostics, and user-facing explanations.

### Returning values and failures with `R<T>`

```c++
#include <Luna/Runtime/Result.hpp>
```

`R<T>` contains a `T` for every successful result, including informative success. It contains no `T` for a failure.

```c++
R<Record> find_record(record_id_t id)
{
    Record* cached = find_cached_record(id);
    if(cached)
    {
        return R<Record>(*cached, MyModule::S_CACHE_HIT);
    }
    Record* record = find_stored_record(id);
    if(!record) return MyModule::E_RECORD_NOT_FOUND;
    return *record;
}
```

For `R<T>`, construct an informative success with both the value and its successful `ResultCode`. Construct a failure with its `ResultCode` alone. Passing a successful code to the failure constructor is invalid.

`RV` is an alias of `R<void>`. It can carry a failure, an informative success, or `ok`:

```c++
RV refresh_cache()
{
    if(cache_is_current()) return MyModule::S_CACHE_HIT;
    if(!perform_refresh()) return E_BAD_PLATFORM_CALL;
    return ok;
}
```

Because `RV` only stores a static `ResultCode`, it can be returned before Runtime initialization. `Luna::init` uses `RV` to report initialization failures without relying on runtime metadata or the thread-local `Error` object.

Inspect both the success state and optional status when it matters:

```c++
auto result = find_record(id);
if(failed(result))
{
    handle_failure(result.errcode());
    return;
}

consume(result.get());
if(result.errcode() == MyModule::S_CACHE_HIT)
{
    record_cache_hit();
}
```

### Attaching contextual error information

Use `set_error` when a stable failure type is not sufficient to explain a particular failed operation:

```c++
RV open_record(const c8* path)
{
    if(!platform_open(path))
    {
        return set_error(MyModule::E_RECORD_NOT_FOUND, "The record file '%s' was not found.", path);
    }
    return ok;
}
```

`set_error` stores `MyModule::E_RECORD_NOT_FOUND` and the formatted message in the current thread's `Error`, then returns `E_ERROR_OBJECT`. The code passed to `set_error` must be a failure; informative successes do not use the thread-local `Error` path. Call `unwrap_errcode` to obtain the underlying code:

```c++
#include <Luna/Runtime/Log.hpp>

auto result = open_record(path);
if(failed(result))
{
    ResultCode code = unwrap_errcode(result);
    log_error("MyModule", "%s", explain(result.errcode()));
    if(code == MyModule::E_RECORD_NOT_FOUND)
    {
        // Handle the specific failure.
    }
}
```

`explain` returns the thread-local message for `E_ERROR_OBJECT`; otherwise it returns registered result metadata when available.

### Propagating failures with macros

`lutry`, `luexp`, `lulet`, `luset`, `luthrow`, `lucatch`, and `lucatchret` reduce repetitive failure checks:

```c++
R<Record> load_and_validate(record_id_t id)
{
    Record result;
    lutry
    {
        lulet(record, find_record(id));
        result = record;
        luexp(validate_record(result));
        if(result.is_obsolete()) luthrow(E_BAD_DATA);
    }
    lucatchret;
    return result;
}
```

The macros branch only for negative local result values. Informative successes do not enter a `lucatch` block. `luthrow` requires a failure code. If a caller needs to preserve or react to an informative success, store and inspect the returned `R<T>` explicitly instead of relying on failure-propagation macros.

### Transferring result codes

The complete `u64` is the interchange identity. A binary protocol may transfer it after defining byte order. Names and descriptions may accompany the number for diagnostics but must not replace it.

JSON numbers cannot represent every `u64` exactly in common JavaScript implementations. Encode result codes in text protocols as a fixed-width 16-digit hexadecimal string, for example:

```text
8123456789ABFFFF
```

Frontend error objects follow this rule in their `result_code` field. Their category and result names remain separate diagnostic fields.

Parse the string back to `u64`, construct `ResultCode`, and use the field helpers to interpret it. Do not reinterpret object memory or depend on host endianness.

## Migration from runtime-generated codes

When migrating older code:

1. Replace the legacy `ErrCode` type with `ResultCode`; no compatibility alias is provided.
2. Allocate permanent domain, category, and local result values.
3. Replace calls to the removed name-based result lookup API with public `inline constexpr ResultCode` constants.
4. Replace `code.code == 0` and `code.code != 0` control-flow checks with `succeeded`, `failed`, `is_plain_success`, or `is_informative_success` as appropriate.
5. Make runtime registration supply metadata for the constants instead of creating their identities.
6. Transfer the complete 64-bit value across process boundaries.
7. Remove function-style result getters and use the constants directly.
8. Move constants from dedicated error namespaces into the module root namespace, expose the category key as `ERROR_CATEGORY`, and register the category using the canonical module name.

The static code system has no initialization or shutdown step. Only metadata and rich error objects follow the Runtime lifecycle.
