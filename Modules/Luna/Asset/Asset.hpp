/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Asset.hpp
* @author JXMaster
* @date 2022/5/11
*/
#pragma once
#include <Luna/Runtime/Result.hpp>
#include <Luna/Runtime/Ref.hpp>
#include <Luna/Runtime/Path.hpp>
#include "Asset.generated.hpp"
#ifndef LUNA_ASSET_API
#define LUNA_ASSET_API
#endif

namespace Luna
{
    namespace Asset
    {
        //! @addtogroup Asset Asset
        //! Runtime asset management system.
        //! @{
        
        //! Represents one handle that identifies one asset entry in process scope.
        //! @details An asset is one logical group of application data identified by one GUID and one shared path.
        //! Its data is divided into one main data unit and zero or more independently managed named data units.
        //! Every asset is identified by one asset GUID. The asset GUID is generated and assigned
        //! to one asset when the asset is created, and cannot be changed after the asset is created. 
        //! The asset handle is the runtime representation of asset GUID. Every GUID has one unique asset handle, 
        //! which can be fetched by @ref get_asset, the asset handle will be valid until 
        //! the asset registry is closed, so we can always query asset information using one asset handle.
        //!  
        //! The main reason we use asset handle instead of using GUID directly to refer one asset
        //! is performance: the asset handle is actually one pointer to the internal asset entry 
        //! information block, so we can fetch the asset information directly by dereferring the 
        //! asset handle internally. If we use asset GUID instead, every asset information query call 
        //! will have look up a global GUID-to-entry map to route the actual asset information block, 
        //! which is slow and even slower in multi-threaded environment, since every look up to global GUID
        //! map must be synchronized.
        //!  
        //! The asset handle is unique in process scope, that is to say, if two processes (or restart the 
        //! current process) refer to the same asset, their asset handle values are not the same. 
        //! However, the asset GUID is unique globally, so that fetching assets using the same GUID 
        //! will always get the same asset, even through their asset handle values may not be the same. Due to
        //! this reason, when serializing one reference to one asset, we should save the asset GUID rather than 
        //! the handle value, so that we can get the same asset and restore reference after the application
        //! is restarted.
        struct [[luna::struct("{69A0F401-6B30-4C91-B790-07BD02E64C56}")]] asset_t
        {
            opaque_t handle;
            asset_t() :
                handle(nullptr) {}
            explicit constexpr asset_t(opaque_t handle) :
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
            void reset()
            {
                handle = nullptr;
            }
        };

        //! Identifies the state of one asset data unit.
        enum class AssetDataUnitState
        {
            //! The containing asset is not registered. The asset should be registered first by
            //! calling @ref register_asset or @ref new_asset.
            unregistered = 0,
            //! The asset data unit object is not loaded.
            unloaded = 1,
            //! The asset data unit object is loading.
            loading = 2,
            //! The asset data unit object is loaded.
            loaded = 3
        };

        //! Describes one asset loader.
        //! @details Operations on different data units may invoke callbacks concurrently. If multiple data units refer
        //! to the same mutable data object, the loader or caller is responsible for synchronizing access to that object.
        struct AssetLoaderDesc
        {
            //! The globally unique asset loader name.
            Name name;
            //! The userdata object. The object will be kept by the asset system and provided to every callback function.
            ObjRef userdata;
            //! Called when one asset data unit object with data loaded from the specified file is being requested. The
            //! user should create a new asset data object and load asset data from the specified file to the object.
            //! @details This function can be `nullptr`. If this function is `nullptr`, this asset loader does not support
            //! loading asset data from file, and such requests failed with @ref E_NOT_SUPPORTED.
            //! @param[in] userdata The userdata.
            //! @param[in] asset The asset handle of the asset being loaded.
            //! @param[in] data_unit The ID of the data unit being loaded. This is empty for the main data unit.
            //! @param[in] path The VFS path to load asset data from.
            //! @return Returns the loaded asset data object.
            R<ObjRef>(*on_load_asset_data_unit)(object_t userdata, asset_t asset, const Name& data_unit, const Path& path) = nullptr;
            //! Called when one asset data unit object with default asset data is being requested. The user should create a
            //! new asset data object and load default asset data to the object.
            //! @details This function can be `nullptr`. If this function is `nullptr`, this asset loader does not support
            //! loading default asset data, and such requests failed with @ref E_NOT_SUPPORTED.
            //! @param[in] userdata The userdata.
            //! @param[in] asset The asset handle of the asset being loaded.
            //! @param[in] data_unit The ID of the data unit being loaded. This is empty for the main data unit.
            //! @return Returns the created asset data object with default asset data.
            R<ObjRef>(*on_load_asset_data_unit_default_data)(object_t userdata, asset_t asset, const Name& data_unit) = nullptr;
            //! Called when one asset data unit is being saved.
            //! @param[in] userdata The userdata.
            //! @param[in] asset The asset handle of the asset being saved.
            //! @param[in] data_unit The ID of the data unit being saved. This is empty for the main data unit.
            //! @param[in] path The VFS path to save asset data to.
            //! @param[in] data The asset data object to save.
            RV(*on_save_asset_data_unit)(object_t userdata, asset_t asset,
                const Name& data_unit, const Path& path, object_t data) = nullptr;
            //! Called when a new data object is set to one asset data unit.
            //! @details This function is called before the set operation happens, so the user can call 
            //! @ref get_asset_data_unit_object on `asset` and `data_unit` to get the existing asset data object (if any).
            //!  
            //! If this function fails, the new asset data object will not be set, that the existing asset data object
            //! is not changed.
            //! 
            //! This function can be `nullptr`, in such case, the default callback function will be used, which simply
            //! does nothing and returns success directly.
            //! @param[in] userdata The userdata.
            //! @param[in] asset The asset handle of the asset to set new asset data object.
            //! @param[in] data_unit The ID of the data unit being set. This is empty for the main data unit.
            //! @param[in] data The new asset data object to set.
            //! This can be `nullptr` if the user calls @ref set_asset_data_unit_object with `data` equals to `nullptr`. In such case,
            //! this function behaves like unloading existing asset data object.
            RV(*on_set_asset_data_unit)(object_t userdata, asset_t asset, const Name& data_unit, object_t data) = nullptr;
            //! Called when assets referred by the specified asset data unit are required.
            //! @param[in] userdata The userdata.
            //! @param[in] asset The asset handle of the asset to be queried.
            //! @param[in] data_unit The ID of the data unit being queried. This is empty for the main data unit.
            //! @param[out] referred_assets Returns the assets referred by this asset.
            //! This vector may not be empty when this function is called. If this vector is not empty, the returned
            //! assets shall be pushed to the end of this vector, and existing elements shall not be modified.
            void(*on_get_referred_assets)(object_t userdata, asset_t asset,
                const Name& data_unit, Vector<asset_t>& referred_assets) = nullptr;
        };

        //! Describes one asset type.
        struct AssetTypeDesc
        {
            //! The asset type name.
            Name name;
            //! The name of the asset loader used by the main data unit of assets of this type.
            Name main_data_unit_loader;
        };

        //! Describes one named asset data unit.
        //! @details The main data unit is represented by an empty ID and is implicitly provided by the asset type.
        //! Only named data units are stored explicitly in asset metadata.
        struct [[Luna::struct("{8AE4D56B-E4ED-40D9-9652-DC0B19124720}")]] AssetDataUnitDesc
        {
            //! The data unit ID. This must not be empty for named data units.
            [[Luna::property]] Name id;
            //! The name of the asset loader used to manage the data unit.
            [[Luna::property]] Name loader;
        };

        //! Registers one asset loader so asset data units can use it.
        //! @details If one asset loader with the same name already exists, the existing descriptor is replaced.
        //! Operations that have already resolved a loader descriptor keep their own copy until the callback returns.
        //! @param[in] desc The asset loader descriptor.
        //! @par Valid Usage
        //! * `desc.name` must not be empty.
        LUNA_ASSET_API void register_asset_loader(const AssetLoaderDesc& desc);

        //! Registers one asset type so the asset system can handle the asset of that type.
        //! @details If one asset type with the same name already exists, the existing asset type
        //! will be replaced with the new asset type.
        //! @param[in] desc The asset type descriptor.
        //! @par Valid Usage
        //! * `desc.name` and `desc.main_data_unit_loader` must not be empty.
        LUNA_ASSET_API void register_asset_type(const AssetTypeDesc& desc);

        //! Gets the asset handle from one asset GUID. If the asset entry with the specified GUID does not exist, this 
        //! function creates one new asset entry with the specified GUID and returns the handle to the new created asset entry.
        //! @details Asset handles created by @ref get_asset are in unregistered state
        //! (@ref get_asset_data_unit_state returns @ref AssetDataUnitState::unregistered for their main data units).
        //! The user should call @ref register_asset on the handle to register the asset before she can really use the asset. The user can also 
        //! call @ref new_asset to create and register asset in one call.
        //! @param[in] guid The asset GUID to fetch. If this is (0, 0), the system generates a random asset GUID, so
        //! that one new asset entry is always created and returned.
        //! @return Returns the asset handle with the specified GUID.
        LUNA_ASSET_API asset_t get_asset(const Guid& guid = Guid(0, 0));

        //! Registers the specified asset.
        //! @param[in] asset The asset handle of the asset to register.
        //! @param[in] type The type of the asset.
        LUNA_ASSET_API RV register_asset(asset_t asset, const Name& type);

        //! Creates a new asset by specifying the path and type of the asset.
        //! @details Returns the existing registered asset when its path is already present.
        //! Otherwise generates a new GUID, saves metadata if requested, then publishes the registration.
        //! A metadata save failure does not publish a partial registration.
        //! @param[in] path The path to place the new created asset.
        //! If the asset with the specified path is already registered, this call does nothing and returns
        //! the registered asset directly.
        //! 
        //! This parameter can be empty (`Path()`), in such case, the asset is created as one dynamic asset that
        //! cannot be saved to files. If this is empty, path duplication check will be skipped.
        //! @param[in] type The type of the asset.
        //! @param[in] save_meta Whether to write metadata to the selected database (or a legacy sidecar).
        //! Single-file databases stage this record until their explicit flush.
        //! @return Returns the asset handle of the new created asset.
        //! @par Valid Usage
        //! * If `save_meta` is `true`, `path` must not be empty.
        LUNA_ASSET_API R<asset_t> new_asset(const Path& path, const Name& type, bool save_meta = true);

        //! Discovers asset metadata at a path or beneath a directory.
        //! @details Registered database roots use their logical record snapshots. Other paths use sidecars.
        //! The complete batch is validated before registration; failure leaves the registry unchanged.
        //! Centralized discovery does not scan directories or open payload files. Use reload_asset_database
        //! to refresh a centralized provider from external storage.
        //! @param[in] path The asset or directory VFS path.
        //! @param[in] allow_overwrite Whether existing compatible registrations may be updated.
        //! Ownership conflicts and duplicate batch GUIDs/paths are errors even when overwriting is disabled.
        //! @retval E_ASSET_DATA_UNIT_BUSY An affected data unit prevents metadata reconciliation.
        LUNA_ASSET_API RV load_assets_meta(const Path& path, bool allow_overwrite = true);

        //! Loads asset metadata from its database logical view, or from its legacy sidecar.
        //! @param[in] asset The asset handle of the asset to operate.
        //! @par Valid Usage
        //! * `asset` must have a valid VFS path.
        //! @retval E_ASSET_DATA_UNIT_BUSY The asset or one of its data units is busy.
        LUNA_ASSET_API RV load_asset_meta(asset_t asset);

        //! Saves runtime metadata to its database, or to its legacy sidecar.
        //! @details Single-file databases stage record updates until IAssetDatabase::flush. This function
        //! does not save any data unit or flush a VFS filesystem. An old metadata location is removed after a successful relocation.
        //! @param[in] asset The asset handle of the asset to operate.
        //! @par Valid Usage
        //! * `asset` must have a valid VFS path.
        //! @retval E_ASSET_DATA_UNIT_BUSY The asset or one of its data units is busy.
        LUNA_ASSET_API RV save_asset_meta(asset_t asset);

        //! Gets one asset by path.
        //! @param[in] path The path of the asset.
        //! @return Returns the fetched asset handle.
        LUNA_ASSET_API R<asset_t> get_asset_by_path(const Path& path);

        //! Gets the asset GUID.
        //! @param[in] asset The asset handle of the asset to query.
        //! @return Returns the asset GUID of the asset.
        LUNA_ASSET_API Guid get_asset_guid(asset_t asset);

        //! Gets the asset VFS path.
        //! @param[in] asset The asset handle of the asset to query.
        //! @return Returns the asset VFS path of the asset.
        LUNA_ASSET_API Path get_asset_path(asset_t asset);

        //! Sets the asset VFS path.
        //! @details Changes the runtime path only. Move payloads manually, then call save_asset_meta to relocate
        //! the persistent record from its last saved path, including between database providers.
        //! Use get_asset_files to enumerate payloads, or move_asset to perform the complete operation.
        //! @param[in] asset The asset handle of the asset to operate.
        //! @param[in] path The new asset VFS path to set.
        LUNA_ASSET_API RV set_asset_path(asset_t asset, const Path& path);

        //! Gets the asset name, which is the filename component of the asset VFS path, excluding the extension.
        //! @param[in] asset The asset handle of the asset to query.
        //! @return Returns the name of the asset.
        LUNA_ASSET_API Name get_asset_name(asset_t asset);

        //! Gets the asset type.
        //! @param[in] asset The asset handle of the asset to query.
        //! @return Returns the type of the asset.
        LUNA_ASSET_API Name get_asset_type(asset_t asset);

        //! Sets the asset type.
        //! @details Changes runtime metadata only. Call save_asset_meta to persist the modified type through its provider.
        //! Named data units and their loaded objects are not changed.
        //! @param[in] asset The asset handle of the asset to operate.
        //! @param[in] type The new asset type to set.
        //! @par Valid Usage
        //! * `type` must not be empty.
        //! * The main data unit must be unloaded and have no operation in progress.
        //! * The asset must not be undergoing maintenance.
        LUNA_ASSET_API void set_asset_type(asset_t asset, const Name& type);

        //! Get filenames of all files associated to the specified asset.
        //! @details Associated files follow the asset-file naming convention: the asset filename itself, or the asset
        //! filename followed by an extension. Files outside this convention are not returned.
        //! @param[in] asset The asset handle of the asset to query.
        //! @param[out] filenames Returns filenames of all files associated to the specified asset. 
        //! Existing elements in the array will be preserved. Metadata files (including sidecars and shared databases)
        //! are excluded; database implementations own their maintenance.
        LUNA_ASSET_API RV get_asset_files(asset_t asset, Vector<Name>& filenames);

        //! Deletes one asset and all of its associated files.
        //! @details This function performs the following tasks:
        //! 1. Delete all files fetched from @ref get_asset_files.
        //! 2. Remove the database record, then clear the path, type, descriptors and loaded objects.
        //! Read-only databases reject this operation before deleting payloads. Physical multi-file deletion is not transactional.
        //! The asset handle will still be valid after this operation, but the main data unit state will
        //! be set to @ref AssetDataUnitState::unregistered, and all operations on the asset are invalid.
        //! @param[in] asset The asset handle of the asset to operate.
        LUNA_ASSET_API RV delete_asset(asset_t asset);

        //! Moves all asset associated files to a new destination.
        //! @details Moves payloads from get_asset_files, transfers the persistent record, then updates the runtime path.
        //! The GUID is preserved. Read-only source or destination databases reject mutation before moving payloads.
        //! Completed file moves are rolled back where possible on error; rollback failures are reported.
        //! @param[in] asset The asset handle of the asset to operate.
        //! @param[in] new_path The new path of the asset.
        LUNA_ASSET_API RV move_asset(asset_t asset, const Path& new_path);

        //! Makes a duplication of the specified asset.
        //! @details Copies payloads from get_asset_files, writes metadata through the destination provider,
        //! then publishes a new registration. Completed copies are removed where possible after a later failure.
        //! @param[in] asset The asset handle of the asset to operate.
        //! @param[in] new_path The path of the new asset.
        //! @param[in] guid The GUID of the new asset. This will be provided to @ref get_asset as-is.
        //! If this is zero, a new GUID is generated. A nonzero GUID must refer to an unregistered placeholder.
        //! @return Returns the new created asset. Every data unit of a newly created copy is unloaded.
        LUNA_ASSET_API R<asset_t> copy_asset(asset_t asset, const Path& new_path, const Guid& guid = Guid(0, 0));

        //! Adds one named data unit to the asset.
        //! @details The loader name is stored without requiring the loader to be registered. The loader is resolved
        //! when an operation on the data unit is requested.
        //! @param[in] asset The asset handle of the asset to modify.
        //! @param[in] desc The named data unit descriptor. `desc.id` and `desc.loader` must not be empty.
        //! @retval E_BAD_ARGUMENTS `desc.id` or `desc.loader` is empty.
        //! @retval E_ASSET_DATA_UNIT_ALREADY_EXISTS A data unit with the same ID already exists.
        //! @retval E_ASSET_NOT_REGISTERED The asset is not registered.
        //! @retval E_ASSET_DATA_UNIT_BUSY The asset is undergoing maintenance.
        LUNA_ASSET_API RV add_asset_data_unit(asset_t asset, const AssetDataUnitDesc& desc);

        //! Removes one named data unit from the asset.
        //! @details The main data unit cannot be removed. A named data unit must be unloaded and idle before removal.
        //! This function modifies in-memory metadata only; it does not remove files.
        //! @param[in] asset The asset handle of the asset to modify.
        //! @param[in] data_unit The non-empty ID of the named data unit to remove.
        //! @retval E_ASSET_DATA_UNIT_NOT_FOUND The specified data unit does not exist.
        //! @retval E_ASSET_DATA_UNIT_BUSY The specified data unit is loaded or has an operation in progress,
        //! or the containing asset is undergoing maintenance.
        //! @retval E_BAD_ARGUMENTS `data_unit` is empty.
        //! @retval E_ASSET_NOT_REGISTERED The asset is not registered.
        LUNA_ASSET_API RV remove_asset_data_unit(asset_t asset, const Name& data_unit);

        //! Enumerates data units of one asset.
        //! @details The implicit main data unit is appended first with an empty ID. Named data units are then appended
        //! in ascending ID order. Existing elements in `out_data_units` are preserved.
        //! @param[in] asset The asset handle of the asset to query.
        //! @param[out] out_data_units The vector that receives data unit descriptors.
        //! @retval E_UNKNOWN_ASSET_TYPE The asset type required to resolve the main loader is not registered.
        //! @retval E_ASSET_NOT_REGISTERED The asset is not registered.
        LUNA_ASSET_API RV get_asset_data_units(asset_t asset, Vector<AssetDataUnitDesc>& out_data_units);

        //! Gets the loader name assigned to one asset data unit.
        //! @param[in] asset The asset handle of the asset to query.
        //! @param[in] data_unit The data unit ID, or an empty name for the main data unit.
        //! @return Returns the assigned loader name.
        //! @retval E_ASSET_DATA_UNIT_NOT_FOUND The specified named data unit does not exist.
        //! @retval E_UNKNOWN_ASSET_TYPE The asset type required to resolve the main loader is not registered.
        //! @retval E_ASSET_NOT_REGISTERED The asset is not registered.
        LUNA_ASSET_API R<Name> get_asset_data_unit_loader(asset_t asset, const Name& data_unit);

        //! Gets one asset data unit object.
        //! @param[in] asset The asset handle of the asset to query.
        //! @param[in] data_unit The data unit ID, or an empty name for the main data unit.
        //! @return Returns the data object, or `nullptr` if the data unit is not loaded.
        //! @retval E_ASSET_DATA_UNIT_NOT_FOUND The specified named data unit does not exist.
        //! @retval E_BAD_ARGUMENTS `asset` is null.
        LUNA_ASSET_API R<ObjRef> get_asset_data_unit_object(asset_t asset, const Name& data_unit);

        //! Gets one typed asset data unit object.
        //! @param[in] asset The asset handle of the asset to query.
        //! @param[in] data_unit The data unit ID, or an empty name for the main data unit.
        //! @return Returns the typed data unit object, or `nullptr` if it is not loaded or cannot be cast to `_Ty`.
        //! @retval E_ASSET_DATA_UNIT_NOT_FOUND The specified named data unit does not exist.
        //! @retval E_BAD_ARGUMENTS `asset` is null.
        template <typename _Ty>
        R<Ref<_Ty>> get_asset_data_unit_object(asset_t asset, const Name& data_unit)
        {
            auto data = get_asset_data_unit_object(asset, data_unit);
            if(failed(data)) return data.errcode();
            return Ref<_Ty>(data.get());
        }

        //! Sets one asset data unit object.
        //! @param[in] asset The asset handle of the asset to operate.
        //! @param[in] data_unit The data unit ID, or an empty name for the main data unit.
        //! @param[in] data The data object to set, or `nullptr` to unload the data unit.
        //! @retval E_ASSET_DATA_UNIT_NOT_FOUND The specified named data unit does not exist.
        //! @retval E_ASSET_DATA_UNIT_BUSY The data unit or containing asset is busy.
        //! @retval E_UNKNOWN_ASSET_LOADER The loader assigned to the data unit is not registered.
        //! @retval E_UNKNOWN_ASSET_TYPE The main data unit's asset type is not registered.
        //! @retval E_ASSET_NOT_REGISTERED The asset is not registered or the registry is closing.
        //! @retval E_BAD_ARGUMENTS `asset` is null.
        //! @return Errors returned by `on_set_asset_data_unit` are propagated without changing the data object.
        //! @par Valid Usage
        //! * If `data` is not `nullptr`, it must be a valid pointer to a boxed instance.
        LUNA_ASSET_API RV set_asset_data_unit_object(asset_t asset, const Name& data_unit, object_t data);

        //! Loads or reloads one asset data unit from the asset file.
        //! @details This function loads synchronously. To load asynchronously, call it from a background thread or job.
        //! If loading or the loader callback fails during forced reload, the previously loaded data object is preserved.
        //! @param[in] asset The asset handle of the asset to operate.
        //! @param[in] data_unit The data unit ID, or an empty name for the main data unit.
        //! @param[in] force_reload Whether to load again when the data unit is already loaded.
        //! @retval E_ASSET_DATA_UNIT_NOT_FOUND The specified named data unit does not exist.
        //! @retval E_ASSET_DATA_UNIT_BUSY The data unit or containing asset is busy.
        //! @retval E_UNKNOWN_ASSET_LOADER The loader assigned to the data unit is not registered.
        //! @retval E_UNKNOWN_ASSET_TYPE The main data unit's asset type is not registered.
        //! @retval E_EMPTY_ASSET_PATH The asset path is empty.
        //! @retval E_ASSET_NOT_REGISTERED The asset is not registered or the registry is closing.
        //! @retval E_BAD_ARGUMENTS `asset` is null.
        //! @retval E_NOT_SUPPORTED The loader does not provide a file-load callback.
        //! @retval E_BAD_DATA The loader returned a null data object.
        //! @return Other errors returned by `on_load_asset_data_unit` are propagated.
        LUNA_ASSET_API RV load_asset_data_unit(asset_t asset, const Name& data_unit, bool force_reload = false);

        //! Loads or reloads one asset data unit with default data.
        //! @details If loading or the loader callback fails during forced reload, the previously loaded data object is preserved.
        //! @param[in] asset The asset handle of the asset to operate.
        //! @param[in] data_unit The data unit ID, or an empty name for the main data unit.
        //! @param[in] force_reload Whether to load again when the data unit is already loaded.
        //! @retval E_ASSET_DATA_UNIT_NOT_FOUND The specified named data unit does not exist.
        //! @retval E_ASSET_DATA_UNIT_BUSY The data unit or containing asset is busy.
        //! @retval E_UNKNOWN_ASSET_LOADER The loader assigned to the data unit is not registered.
        //! @retval E_UNKNOWN_ASSET_TYPE The main data unit's asset type is not registered.
        //! @retval E_ASSET_NOT_REGISTERED The asset is not registered or the registry is closing.
        //! @retval E_BAD_ARGUMENTS `asset` is null.
        //! @retval E_NOT_SUPPORTED The loader does not provide a default-data callback.
        //! @retval E_BAD_DATA The loader returned a null data object.
        //! @return Other errors returned by `on_load_asset_data_unit_default_data` are propagated.
        LUNA_ASSET_API RV load_asset_data_unit_default_data(asset_t asset, const Name& data_unit, bool force_reload = false);

        //! Gets the state of one asset data unit.
        //! @param[in] asset The asset handle of the asset to query.
        //! @param[in] data_unit The data unit ID, or an empty name for the main data unit.
        //! @return Returns the state of the specified data unit.
        //! @retval E_ASSET_DATA_UNIT_NOT_FOUND The specified named data unit does not exist.
        //! @retval E_BAD_ARGUMENTS `asset` is null.
        LUNA_ASSET_API R<AssetDataUnitState> get_asset_data_unit_state(asset_t asset, const Name& data_unit);

        //! Saves one asset data unit to files.
        //! @details This function saves synchronously. To save asynchronously, call it from a background thread or job.
        //! Data units are saved independently; this function does not save metadata.
        //! @param[in] asset The asset handle of the asset to operate.
        //! @param[in] data_unit The data unit ID, or an empty name for the main data unit.
        //! @retval E_ASSET_DATA_UNIT_NOT_LOADED The specified data unit has no loaded data object.
        //! @retval E_ASSET_DATA_UNIT_NOT_FOUND The specified named data unit does not exist.
        //! @retval E_ASSET_DATA_UNIT_BUSY The data unit or containing asset is busy.
        //! @retval E_UNKNOWN_ASSET_LOADER The loader assigned to the data unit is not registered.
        //! @retval E_UNKNOWN_ASSET_TYPE The main data unit's asset type is not registered.
        //! @retval E_EMPTY_ASSET_PATH The asset path is empty.
        //! @retval E_ASSET_NOT_REGISTERED The asset is not registered or the registry is closing.
        //! @retval E_BAD_ARGUMENTS `asset` is null.
        //! @retval E_NOT_SUPPORTED The loader does not provide a save callback.
        //! @return Other errors returned by `on_save_asset_data_unit` are propagated.
        LUNA_ASSET_API RV save_asset_data_unit(asset_t asset, const Name& data_unit);

        //! Gets referred assets of one asset data unit.
        //! @param[in] asset The handle of the asset to query.
        //! @param[in] data_unit The data unit ID, or an empty name for the main data unit.
        //! @param[out] out_referred_assets Returns the referred assets. Existing elements are preserved.
        //! @details This function returns without changing `out_referred_assets` if the asset, data unit, type, or loader is invalid,
        //! or if another operation is in progress on the data unit.
        LUNA_ASSET_API void get_asset_data_unit_referred_assets(asset_t asset, const Name& data_unit,
            Vector<asset_t>& out_referred_assets);

        //! Closes the asset registry.
        //! @details This call removes all registered assets, asset types, and asset loaders, and invalidates all asset handles.
        //! Registered metadata databases are flushed before removal; failures are logged.
        //! Call flush_asset_databases before close to handle save failures explicitly.
        //! This does not implicitly save runtime metadata descriptors or data-unit objects.
        //! @par Valid Usage
        //! * Before calling this function, the caller must stop new Asset API submissions and wait until all Asset API
        //! calls and asset loader callbacks have returned.
        //! * The caller must not submit new Asset API calls until this function returns.
        LUNA_ASSET_API void close();

        //! @}
    }

    //! @addtogroup Asset
    //! @{
    //! @defgroup AssetResultCodes Asset Result Codes
    //! @}

    namespace Asset
    {
        //! @addtogroup AssetResultCodes
        //! @{

        //! The Asset error category identifier.
        inline constexpr errcat_t ERROR_CATEGORY = make_error_category(ErrorDomain::LUNA_SDK, LunaErrorCategory::ASSET);

        //! The asset metadata record was not found in its database or sidecar file.
        inline constexpr ResultCode E_META_FILE_NOT_FOUND = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::ASSET, -1);
        //! The asset type is not registered.
        inline constexpr ResultCode E_UNKNOWN_ASSET_TYPE = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::ASSET, -2);
        //! The asset is not registered.
        inline constexpr ResultCode E_ASSET_NOT_REGISTERED = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::ASSET, -3);
        //! The asset is already registered.
        inline constexpr ResultCode E_ASSET_ALREADY_REGISTERED = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::ASSET, -4);
        //! The asset path is empty.
        inline constexpr ResultCode E_EMPTY_ASSET_PATH = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::ASSET, -5);
        //! The specified asset data unit object has not been loaded.
        inline constexpr ResultCode E_ASSET_DATA_UNIT_NOT_LOADED =
            make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::ASSET, -6);
        //! The asset loader is not registered.
        inline constexpr ResultCode E_UNKNOWN_ASSET_LOADER = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::ASSET, -8);
        //! The specified asset data unit does not exist.
        inline constexpr ResultCode E_ASSET_DATA_UNIT_NOT_FOUND = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::ASSET, -9);
        //! An asset data unit with the specified ID already exists.
        inline constexpr ResultCode E_ASSET_DATA_UNIT_ALREADY_EXISTS =
            make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::ASSET, -10);
        //! The specified asset data unit is loaded, has an operation in progress, or the containing asset is undergoing maintenance.
        inline constexpr ResultCode E_ASSET_DATA_UNIT_BUSY = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::ASSET, -11);

        //! @}
    }
    
    struct Module;
    LUNA_ASSET_API Module* module_asset();

    template <> struct hash<Asset::asset_t>
    {
        usize operator()(Asset::asset_t val) const
        {
            return (usize)val.handle;
        }
    };
}
