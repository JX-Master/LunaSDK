## Status
Approved.

## Last updated
2026/8/25

## Background
The legacy LunaSDK error system represents an error code with an `ErrCode` wrapper around a machine-sized integer. The integer is produced on first use by hashing an error category name and error name, then using the hash as a key into a runtime registry. Error categories are represented by runtime `errcat_t` values produced by the same system.

This design does not provide a stable cross-process or cross-build numeric ABI. Fetching an error code requires a function call and may initialize registry state on first use. Error codes also cannot be used safely before `Luna::init`, which prevents foundational and constant-evaluated Runtime facilities from reporting failures through the normal result mechanism.

LunaSDK requires result codes whose values are fixed at compile time, remain stable after publication, can be transferred between processes, and can be interpreted without initializing the runtime error information system. The existing `Error` object remains necessary for attaching a message and arbitrary information to a failure.

## Decision
### Result code representation

The public `ResultCode` type will wrap one `u64` value. The name reflects that the value can represent failure, plain success, or informative success. The value has the following layout, from most significant to least significant bits:

```text
63                         32 31             16 15              0
+----------------------------+-----------------+-----------------+
|       domain ID: u32       | category ID:u16 | result value:i16|
+----------------------------+-----------------+-----------------+
```

The low 16 bits are stored as their unsigned bit pattern and interpreted as a signed 16-bit result value in the range -32768 through 32767. Implementations must use explicit masking and conversion helpers rather than pointer reinterpretation or aliasing.

The result value determines whether an operation succeeded:

* A negative result value indicates failure.
* A zero result value indicates success.
* A positive result value indicates success with status information.

The complete 64-bit value determines whether a successful result carries information:

* The value `0x0000000000000000` is the only plain success result and carries no status information.
* Every other value with a non-negative result value is an informative success result. In particular, a non-zero domain ID or category ID makes a result informative even when its result value is zero.

Consequently, code must not use the complete value's zero/non-zero state to distinguish success from failure. LunaSDK will provide constexpr helpers for constructing a result code, extracting its fields, and testing plain success, informative success, success, and failure.

Published result code values are permanent. A domain or category must not reassign a previously published tuple to a different meaning, including after the original result has been deprecated.

### Domain allocation

Domain IDs from `0x00000000` through `0x7FFFFFFF` form the registered region. Using a registered ID requires allocation by the LunaSDK project team. Registered IDs are never reused. Domain ID 0 belongs to LunaSDK.

Domain IDs from `0x80000000` through `0xFFFFFFFF` form the self-allocated region. A project may use this region without registration, but uniqueness is not guaranteed. A self-allocated ID must be produced once by an ID generator, recorded in the project's source or manifest, and then treated as permanent. It must not be regenerated on each build. Generators should derive the 31 payload bits from a committed random UUID or use a suitable random source, then set the high bit.

Projects should retain the generator input or full UUID in a committed manifest when available. Tooling can use that information to diagnose two self-allocated domains that chose the same numeric ID without making the UUID part of every result code.

### Category and local result allocation

Each domain independently allocates its 16-bit category IDs. A category normally identifies a module or another error-producing subsystem, but the owning domain defines the exact policy. Category IDs and result values are never reassigned after publication.

For LunaSDK, category ID 0 belongs to the Runtime module and contains the common result codes. Other LunaSDK modules receive stable non-zero category IDs.

The initial category allocation in LunaSDK domain 0 is permanent:

| Category ID | Owner |
| ---: | --- |
| 0 | Runtime |
| 1 | AHI |
| 2 | Asset |
| 3 | ECS |
| 4 | Frontend |
| 5 | Image |
| 6 | Network |
| 7 | RHI |
| 8 | VFS |

The public `errcat_t` type is a stable `u64` category key. It stores the domain ID in bits 47 through 16 and the category ID in bits 15 through 0; bits 63 through 48 are zero. `0xFFFFFFFFFFFFFFFF` is reserved as the invalid category sentinel. Category keys contain no result value and can be constructed without runtime state.

Each non-zero domain/category pair can define 32768 failure results and 32768 informative success results. The LunaSDK Runtime pair `(0, 0)` has 32768 failure results, 32767 informative success results, and the globally unique plain success result.

### Compile-time declarations

Public result codes are declared as documented `inline constexpr ResultCode` constants in public headers. Calling an exported function or consulting runtime state is not required to construct, compare, return, or inspect a result code.

LunaSDK declares result constants directly in the owning module's root namespace. Runtime constants are in `Luna`; constants owned by another module are in that module namespace, such as `Luna::RHI`. Separate namespaces such as `BasicError` and `RHIError` are not used. Each module exposes its static category key as `ERROR_CATEGORY` in the same root namespace.

The declarations use LunaSDK's global-constant naming convention and encode the result class in the identifier:

* A failure constant whose local result value is negative uses the `E_` prefix.
* A success constant whose local result value is non-negative uses the `S_` prefix. This includes an informative success whose local result value is zero but whose complete code is non-zero.

The prefix is determined by the signed local result value, not by the wording of the condition. It also prevents collisions with common system macros, so a suffix such as `_ERROR` must not be added solely for macro avoidance. Runtime metadata names remain diagnostic identifiers and do not include the C++ `E_` or `S_` prefix. `ok` continues to be the dedicated `RV` value for all-zero plain success. Legacy function-style error getters are removed; all call sites use the constants directly.

The registered category display name is the owning module's canonical name, such as `"Runtime"`, `"RHI"`, or `"Network"`. It does not repeat the removed C++ error namespace or append `"Error"`.

### Result objects and propagation

`R<T>` stores a valid `T` value for every successful result, including informative success. It stores no `T` value for a failure. `R<T>::valid`, `succeeded`, `failed`, copy and move operations, destruction, and the `lutry` propagation macros must test the signed low-16-bit result value rather than the complete `u64` value.

Returning an informative success from `R<T>` requires an API that supplies both the returned value and its `ResultCode`. `R<void>` can carry an informative success directly. `ok` continues to represent the all-zero plain success result. Error-propagation and panic helpers react only to failures; informative successes are not sent to `lucatch` blocks.

`Luna::init` returns `RV` instead of `bool`. `RV` stores only a static `ResultCode`, so constructing and inspecting it does not require LunaSDK to be initialized. The first successful initialization returns `ok`; a call made while LunaSDK is already initialized returns the informative success `S_ALREADY_INITIALIZED`. An initialization failure returns the most specific available static Runtime result code from the failed initialization stage.

Initialization failures must not use `set_error`, `E_ERROR_OBJECT`, or another facility backed by the thread-local `Error` object. Runtime error metadata and rich error objects may not exist yet and are removed when a partially completed initialization is rolled back. The returned static result code remains valid after rollback even when its diagnostic metadata is unavailable.

The storage used by `R<T>` should support constant evaluation when `T` itself permits constant evaluation. Making `ResultCode` independent of runtime initialization is required even where a particular `R<T>` instantiation cannot be constant-evaluated.

### Runtime error information

The runtime error system no longer creates result codes. Its responsibility is to associate optional runtime metadata with pre-existing static codes. Metadata includes the result name, a brief description, and the category identity and display name. The domain identity remains directly decodable from the result code.

Result codes remain valid when LunaSDK is not initialized or when their metadata is not registered. Missing metadata must not change control flow. The metadata API maps static codes to diagnostic information only; it does not provide name-to-code lookup.

The `Error` object, its thread-local storage, formatted messages, and arbitrary `Variant` information remain runtime facilities. A static failure code can therefore be used before initialization, while attaching a rich `Error` object still requires the relevant Runtime services.

### Interchange representation

Binary protocols may transfer the complete `u64` value after specifying byte order. Text protocols that cannot represent every 64-bit integer exactly, including JSON consumed by JavaScript, must use a lossless representation such as a fixed-width 16-digit hexadecimal string. Names and descriptions are diagnostic metadata and are not substitutes for the stable numeric value.

## Impact
Result codes become stable, allocation-free values that can be used before `Luna::init`, in inter-process interfaces, and in constant-evaluated code where the surrounding result type permits it. Returning common errors no longer requires an exported function call or runtime registry lookup.

The change is a breaking public ABI and source-level change. The public wrapper is renamed from `ErrCode` to `ResultCode` and changes from `usize` to `u64`, which changes its size on 32-bit platforms. Existing code that treats every non-zero complete value as a failure becomes incorrect because informative successes can be non-zero. Result constants also move from dedicated error namespaces into their module root namespaces. Registered category display names change to module names; this affects diagnostics and text fields but not the stable numeric identity. `R<T>` and all result propagation macros must be migrated before any informative success is introduced.

Changing `Luna::init` from `bool` to `RV` is an additional public source and ABI break. Callers must use `succeeded`, `failed`, or the standard result propagation and panic helpers instead of Boolean conversion. Callers that ignore the return value now receive the existing `[[nodiscard]]` diagnostic on `RV`.

Every LunaSDK module needs permanent category and result assignments. Dynamic category enumeration becomes a view over registered metadata. The former name-to-code lookup and hierarchical subcategory APIs are removed because static category identities are flat and codes are referenced directly.

The registered domain region introduces an allocation and governance responsibility for the LunaSDK project team. The self-allocated region avoids that dependency for private and experimental projects but deliberately accepts collision risk.

## Alternatives considered
### Keep the runtime name registry and make its hash algorithm stable

This would remove the per-process instability if the hash inputs and algorithm were permanently specified, but it would retain probabilistic collisions, make names part of the numeric ABI, and provide no directly decodable ownership fields. It was rejected in favor of explicit domain and category ownership.

### Centrally register every domain

This would provide strict uniqueness but would impose LunaSDK project governance on private projects, local tools, forks, and experiments. It was rejected in favor of separate registered and self-allocated regions.

### Use a fully self-allocated hash space

This would remove central coordination but could only provide probabilistic uniqueness and would make public interoperability failures difficult to diagnose. It was rejected as the only allocation mechanism, while being retained for the explicitly collision-tolerant self-allocated region.

### Use a 128-bit UUID result identifier

This would reduce the practical collision risk without a registry, but it would increase the size and interchange cost of every result and would not satisfy the requirement for a compact integer result code. It was rejected in favor of a structured 64-bit value plus hybrid allocation.

### Put failure in the high bit of the complete value

This would make signed comparison of the complete value sufficient, but it would couple severity to the ownership fields and provide a less natural local result space. The selected layout keeps a domain-independent signed 16-bit result value within each category and uses the remaining 48 bits exclusively for ownership.

### Keep `Luna::init` returning `bool`

This would preserve source compatibility, but it would continue discarding specific platform, TLS, FLS, and Runtime initialization failures even though static result codes are valid before Runtime initialization. It was rejected because the original reason for the Boolean return no longer applies.

### Return a bare `ResultCode` from `Luna::init`

This would expose the same static identity with slightly less API wrapping, but it would make initialization inconsistent with other fallible operations that return no value. It would also prevent callers from using the existing `RV` propagation and panic helpers directly and would not receive the `[[nodiscard]]` behavior already provided by `R<void>`. It was rejected in favor of `RV`, whose only stored state is the same `ResultCode`.

## Remarks
The complete value is the stable interchange identity. Runtime metadata may be added, translated, or improved without changing the numeric result code.

## Version history
* **2026/8/25** Changed `Luna::init` to return `RV`, required exact static failure propagation without rich error objects, and added `S_ALREADY_INITIALIZED` for repeated initialization.
* **2026/8/20** Renamed the public wrapper type from `ErrCode` to `ResultCode`, moved result constants and `ERROR_CATEGORY` into module root namespaces, and made registered category names equal to canonical module names.
* **2026/8/19** Approved the decision, recorded the initial LunaSDK category allocation, clarified the stable `errcat_t` representation, removed function-style and name-based result getters, and adopted the `E_`/`S_` result-constant naming rule.
* **2026/8/17** Proposed.
