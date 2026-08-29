## Status
Approved.

## Last updated
2026/8/29

## Background
Currently, every asset provided by the `Asset` module can have only one associated data object, which is either loaded or unloaded. In practice, one asset often needs multiple independently managed groups of data, for example:

1. Large game levels are commonly divided into sub-levels. Only the sub-levels near the player are loaded, and they are loaded and unloaded as the player moves. This technique, usually called world streaming, requires parts of the same level asset to be managed independently.
2. An asset may have both runtime data and editor data. Runtime data contains the application payload, while editor data stores development-time metadata owned by editing tools. Both belong to the same asset, but they have different loaders and lifetimes, and editor data may be excluded when shipping a project.

The current asset model cannot manage these independent data groups without representing them as unrelated assets or implementing another lifetime system outside the `Asset` module.

## Decision
We introduce two new concepts to the `Asset` module: *asset data unit* and *asset loader*.

### Asset data unit
An *asset data unit* is an independently managed logical data binding within one asset. It is a loading and lifetime boundary, not necessarily a contiguous region in a file or in memory.

Every asset has exactly one unnamed *main data unit* and may have zero or more *named data units*. Every data unit:

1. Is indexed within the asset by one `Name`. An empty `Name` is reserved for the main data unit. Every named data unit has a non-empty ID that is unique within the asset.
2. Can be loaded, unloaded, and saved independently, and has its own `AssetDataUnitState`.
3. Has its own data object. Multiple data units may refer to the same data object.
4. Has one asset loader that defines how its data is manipulated.

The asset path and GUID are shared by all data units in the asset. The asset loader interprets the shared path together with the data unit ID.

Payload files that participate in the generic asset move, copy, and delete operations must follow the existing asset-file naming convention: the asset filename itself or the asset filename followed by an extension. A loader that owns files outside this convention must manage those files through a feature-specific tool; this decision does not add a loader file-enumeration callback.

The main data unit always exists implicitly. Its loader is selected by the asset type and is not duplicated in the asset metadata. Descriptors for named data units, including their IDs and loader names, are stored in the asset metadata file. Loader names are resolved when a data unit is operated on, so metadata can be loaded before the corresponding loaders are registered.

Adding or removing a named data unit changes in-memory asset metadata; the caller saves that change with `save_asset_meta`. A named data unit must be unloaded before it can be removed. Removing a data unit removes its descriptor but does not generically delete or rewrite physical payload data, because multiple data units may share one file. Physical cleanup is owned by the loader or by an asset processing tool.

### Asset loader
An asset loader defines how to manipulate the data of one asset data unit. It is described by a set of callback functions that were previously stored in `AssetTypeDesc`:

1. `name`
2. `userdata`
3. `on_load_asset_data_unit` (renamed from `on_load_asset`)
4. `on_load_asset_data_unit_default_data` (renamed from `on_load_asset_default_data`)
5. `on_save_asset_data_unit` (renamed from `on_save_asset`)
6. `on_set_asset_data_unit` (renamed from `on_set_asset_data`)
7. `on_get_referred_assets`

Every callback receives the asset handle and data unit ID. Loading and saving callbacks also receive the shared asset path when required. Asset loader names are stable serialized identifiers and should be globally namespaced by their owners.

After these properties are removed from `AssetTypeDesc`, it records only the asset type name and the asset loader used by the main data unit. Loaders for named data units are specified when those units are added to concrete assets.

### Data object API, state, concurrency, and saving
All data-object and lifecycle APIs identify exactly one data unit explicitly. They receive a data unit ID as a `Name`; an empty `Name` selects the main data unit. There are no asset-level convenience overloads that implicitly select the main data unit.

The data-unit APIs are named according to the boundary they operate on:

1. `get_asset_data_unit_object`
2. `set_asset_data_unit_object`
3. `load_asset_data_unit`
4. `load_asset_data_unit_default_data`
5. `get_asset_data_unit_state`
6. `save_asset_data_unit`
7. `get_asset_data_unit_referred_assets`

The state returned by `get_asset_data_unit_state` is represented by `AssetDataUnitState`. An asset does not have one aggregate loading state, because its data units may be in different states simultaneously.

Operations on the same data unit are serialized, while operations on different data units may run concurrently. Loader callbacks are invoked without holding asset registry or asset entry locks. A load operation prepares a new object and commits it only after the loader succeeds; a failed forced reload preserves the previously committed object.

Serialization is keyed by data unit ID, not by data object identity. If multiple data units refer to the same mutable object, their loaders or callers are responsible for synchronizing concurrent access to that shared object.

Saving is defined per data unit. There is no asset-wide operation that implicitly saves every loaded data unit.

Asset-level move, copy, and delete operations continue to treat the asset as one logical management unit. This logical atomicity does not promise crash-safe, all-or-nothing transactions across multiple physical files.

## Impact
This is a breaking change to asset loader registration, asset type registration, and asset data-object and lifecycle APIs. `AssetState` is renamed to `AssetDataUnitState`; `get_asset_data`, `set_asset_data`, `load_asset`, `load_asset_default_data`, `get_asset_state`, `save_asset`, and `get_referred_assets` are replaced by the explicit data-unit APIs listed above. Existing callers, including callers operating only on the main data unit, must migrate and pass the data unit ID explicitly. No compatibility aliases or main-data-unit convenience overloads are retained.

`E_ASSET_DATA_NOT_LOADED` is renamed to `E_ASSET_DATA_UNIT_NOT_LOADED`. `E_ASSET_DATA_LOADING` is removed; a concurrent operation on the same data unit now reports `E_ASSET_DATA_UNIT_BUSY`.

Old metadata files containing only `guid` and `type` are interpreted as assets with no named data units and require no manual migration. Saving them with the new implementation upgrades them to the new format. Older implementations ignore the new data-unit field and may erase it if they subsequently save the metadata, so new metadata is not safely writable by old applications.

Existing Studio asset types initially use only their main data units. Splitting scene actors, texture mip levels, or mesh pieces into named data units is outside this decision and requires feature-specific data-model work.

This decision enables editor-only data to be modeled as a named data unit, but it does not define a project cooking or shipping-strip pipeline. Such a pipeline must define how editor-only units are identified and how their physical payloads are removed.

## Alternatives considered
### Use multiple asset entries to store different parts of one compound asset
This is technically feasible and would require only a small, non-breaking change to the asset API. However:

1. An asset is the minimum logical unit managed by the asset system. Representing one compound asset as several independent entries would weaken this boundary and make tooling more error-prone, because each entry could be moved or deleted independently.
2. Different parts of one compound asset are often stored in the same physical file, such as subresources in one DDS file, while different asset entries cannot share one asset path.
3. Multiple asset entries require callers to retain and pass every related asset handle. One asset entry with multiple data units lets callers pass one asset handle and discover its data units dynamically.

## Version history
* **2026/8/29** Revised the public data-object and lifecycle API names and result codes to make the data-unit boundary explicit, renamed `AssetState` to `AssetDataUnitState`, and removed main-data-unit compatibility overloads.
* **2026/8/29** Approved. Renamed data segments to asset data units and clarified metadata compatibility, lifecycle, concurrency, saving, and logical atomicity.
* **2026/8/28** Proposed.
