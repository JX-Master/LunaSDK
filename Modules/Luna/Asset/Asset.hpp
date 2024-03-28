/*!
* This file is a portion of Luna SDK.
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
        //! @details An asset is one block of application data that is stored on one asset file. 
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
        struct asset_t
        {
            lustruct("Asset::asset_t", "{69A0F401-6B30-4C91-B790-07BD02E64C56}");
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

        //! Identifies the asset state.
        enum class AssetState
        {
            //! The asset handle is not registered. This asset should be registered first by
            //! calling @ref register_asset or @ref new_asset.
            unregistered = 0,
            //! The asset data is not loaded.
            unloaded = 1,
            //! The asset data is loading.
            loading = 2,
            //! The asset data is loaded.
            loaded = 3
        };

        //! Describes one asset type.
        struct AssetTypeDesc
        {
            //! The asset type name.
            Name name;
            //! The userdata object. The object will be kept by the asset system and provided to every callback function.
            ObjRef userdata;
            //! Called when one asset data object with data loaded from the specified file is being requested. The 
            //! user should create a new asset data object and load asset data from the specified file to the object.
            //! @details This function can be `nullptr`. If this function is `nullptr`, this asset type does not support 
            //! loading asset data from file, and such requests failed with @ref BasicError::not_supported.
            //! @param[in] userdata The userdata.
            //! @param[in] asset The asset handle of the asset being loaded.
            //! @param[in] path The VFS path to load asset data from.
            //! @return Returns the loaded asset data object.
            R<ObjRef>(*on_load_asset)(object_t userdata, asset_t asset, const Path& path) = nullptr;
            //! Called when one asset data object with default asset data is being requested. The user should create a 
            //! new asset data object and load default asset data to the object.
            //! @details This function can be `nullptr`. If this function is `nullptr`, this asset type does not support 
            //! loading default asset data, and such requests failed with @ref BasicError::not_supported.
            //! @param[in] userdata The userdata.
            //! @param[in] asset The asset handle of the asset being loaded.
            //! @return Returns the created asset data object with default asset data.
            R<ObjRef>(*on_load_asset_default_data)(object_t userdata, asset_t asset) = nullptr;
            //! Called when the asset data is being saved.
            //! @param[in] userdata The userdata.
            //! @param[in] asset The asset handle of the asset being saved.
            //! @param[in] path The VFS path to save asset data to.
            //! @param[in] data The asset data object to save.
            RV(*on_save_asset)(object_t userdata, asset_t asset, const Path& path, object_t data) = nullptr;
            //! Called when a new asset data object is set to the specified asset.
            //! @details This function is called before the set operation happens, so the user can call 
            //! @ref get_asset_data on `asset` to get the existing asset data object (if any).
            //!  
            //! If this function fails, the new asset data object will not be set, that the existing asset data object
            //! is not changed.
            //! 
            //! This function can be `nullptr`, in such case, the default callback function will be used, which simply
            //! does nothing and returns success directly.
            //! @param[in] userdata The userdata.
            //! @param[in] asset The asset handle of the asset to set new asset data object.
            //! @param[in] data The new asset data object to set.
            //! This can be `nullptr` if the user calls @ref set_asset_data with `data` equals to `nullptr`. In such case,
            //! this function behaves like unloading existing asset data object.
            RV(*on_set_asset_data)(object_t userdata, asset_t asset, object_t data) = nullptr;
        };

        //! Registers one asset type so the asset system can handle the asset of that type.
        //! @details If one asset type with the same name already exists, the existing asset type
        //! will be replaced with the new asset type.
        //! @param[in] desc The asset type descriptor.
        LUNA_ASSET_API void register_asset_type(const AssetTypeDesc& desc);

        //! Gets the asset handle from one asset GUID. If the asset entry with the specified GUID does not exist, this 
        //! function creates one new asset entry with the specified GUID and returns the handle to the new created asset entry.
        //! @details Asset handles created by @ref get_asset is in unregistered state ( @ref get_asset_state returns @ref AssetState::unregistered). 
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
        //! @details This function performs the following options to create a new asset:
        //! 1. Checks whether one asset with the specified path exists, and returns the existing asset if any.
        //! 2. Call @ref get_asset with GUID (0, 0) to generate a new asset entry.
        //! 3. Call @ref register_asset to register the asset with the specified type.
        //! 4. Sets the path of the asset.
        //! 5. If `save_meta_to_file` is `true`, saves asset metadata.
        //! 6. Returns the created asset.
        //! @param[in] path The path to place the new created asset.
        //! If the asset with the specified path is already registered, this call does nothing and returnes
        //! the registered asset directly.
        //! 
        //! This parameter can be empty (`Path()`), in such case, the asset is created as one dynamic asset that
        //! cannot be saved to files. If this is empty, path duplication check will be skipped.
        //! @param[in] type The type of the asset.
        //! @param[in] save_meta_to_file Whether to create one metadata file for the new asset and saves 
        //! metadta to the file.
        //! @return Returns the asset handle of the new created asset.
        //! @par Valid Usage
        //! * If `save_meta_to_file` is `true`, `path` must not be empty.
        LUNA_ASSET_API R<asset_t> new_asset(const Path& path, const Name& type, bool save_meta_to_file = true);

        //! Loads or reloads assets' metadata by reading asset metadata files.
        //! @param[in] path The path of the asset or direcotry.
        //! If `path` specifies one asset, the system loads the asset metadata by opening its meta file and reading from it.
        //! If `path` specifies one directory, the system loads assets metadata for all assets in the directory recursively, every
        //! asset metadata is loaded as if `load_assets_meta` is called for that particular asset.
        //! @param[in] allow_overwrite Specify the behavior when the specified asset already exists in the system.
        //! If `allow_overwrite` is `true`, the system will overwrite the asset metadata in the system using new asset metadata loaded
        //! from asset meta file; if `allow_overwrite` is `false`, the system discards the new asset metadata and does not change the 
        //! asset metadata in the system if the specified asset already exists in the system.
        //! If `path` specifies one directory, this parameter is applied to all assets in that directory.
        LUNA_ASSET_API RV load_assets_meta(const Path& path, bool allow_overwrite = true);

        //! Loads asset metadata from asset's metadata file.
        //! @param[in] asset The asset handle of the asset to operate.
        //! @par Valid Usage
        //! * `asset` must have a valid VFS path.
        LUNA_ASSET_API RV load_asset_meta(asset_t asset);

        //! Saves asset's metadata to asset's metadata file.
        //! @param[in] asset The asset handle of the asset to operate.
        //! @par Valid Usage
        //! * `asset` must have a valid VFS path.
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
        //! @details This function only changes the asset's metadata in the system, it will not save the modified metadata
        //! to metadata file, it will also not move any asset file on VFS. The user should move asset files manually before
        //! calling this function, and call @ref save_asset_meta manually after this function. 
        //! 
        //! The user can get files that should be moved by @ref get_asset_files. The user can also call @ref move_asset
        //! to move asset files, change asset path and save asset metadata to file in one call, which is preferred if the user
        //! does not need to perform custom copy tasks.
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
        //! @details This function will only changes asset's metadata in system, it will not save the modified metadata
        //! to metadata file. The user should call @ref save_asset_meta after this to save the modified metadata 
        //! back to file if needed.
        //! @param[in] asset The asset handle of the asset to operate.
        //! @param[in] type The new asset type to set.
        LUNA_ASSET_API void set_asset_type(asset_t asset, const Name& type);

        //! Get filenames of all files associated to the specified asset.
        //! @param[in] asset The asset handle of the asset to query.
        //! @param[out] filenames Returns filenames of all files associated to the specified asset. 
        //! Existing elements in the array will be preserved.
        LUNA_ASSET_API RV get_asset_files(asset_t asset, Vector<Name>& filenames);

        //! Deletes one asset and all of its associated files.
        //! @details This function performs the following tasks:
        //! 1. Delete all files fetched from @ref get_asset_files.
        //! 2. Set asset path, type and data object to empty.
        //! The asset handle will still be valid after this operation, but the asset state will 
        //! be set to @ref AssetState::unregistered, and all operations to the asset is invalid.
        //! @param[in] asset The asset handle of the asset to operate.
        LUNA_ASSET_API RV delete_asset(asset_t asset);

        //! Moves all asset associated files to a new destination.
        //! @details This function performs the following tasks:
        //! 1. Moves all files fetched from @ref get_asset_files from old path to new path.
        //! 2. Changes path in asset metadata.
        //! 3. Saves modified asset metadata to asset metadata file.
        //! @param[in] asset The asset handle of the asset to operate.
        //! @param[in] new_path The new path of the asset.
        LUNA_ASSET_API RV move_asset(asset_t asset, const Path& new_path);

        //! Gets the asset data object.
        //! @param[in] asset The asset handle of the asset to query.
        //! @return Returns the asset data, or `nullptr` if the asset data is not loaded or loading.
        LUNA_ASSET_API ObjRef get_asset_data(asset_t asset);

        //! Gets the asset data object.
        //! @param[in] asset The asset handle of the asset to query.
        //! @param[in] block_until_loaded If the asset is not loaded and this is `true`, this call blocks the current thread 
        //! until the asset data is loaded, then returns the loaded asset data. Note that this may cause application freeze 
        //! if improperly used.
        //! @return Returns the asset data, or `nullptr` if the asset data is not loaded or failed to load.
        template <typename _Ty>
        Ref<_Ty> get_asset_data(asset_t asset)
        {
            return Ref<_Ty>(get_asset_data(asset));
        }

        //! Sets the asset data object.
        //! @param[in] asset The asset handle of the asset to operate.
        //! @param[in] data The asset data to set. If this is `nullptr`, the asset data will be cleared, and the asset will be in 
        //! @ref AssetState::unloaded state.
        //! @par Valid Usage
        //! * If `data` is not `nullptr`, it must be a valid pointer to a boxed instance.
        LUNA_ASSET_API RV set_asset_data(asset_t asset, object_t data);

        //! Creates one asset data object for the asset by loading data from asset file.
        //! @details This function loads the asset data synchronously. To load asset data asynchronously, call
        //! this function in the thread you want to use for asset loading, like a background thread or a 
        //! worker thread in job system.
        //! @param[in] asset The asset handle of the asset to operate.
        //! @param[in] force_reload If this is `true`, this function always loads the asset data from file even if the asset is 
        //! in loaded state, and the existing asset data object will be replaced with new asset data object.
        //! If this is `false`, this function returns directly if the asset is already in loaded state, and the existing asset data 
        //! object will not be changed.
        LUNA_ASSET_API RV load_asset(asset_t asset, bool force_reload = false);

        //! Creates one asset data object for the asset by loading default asset data.
        //! @param[in] asset The asset handle of the asset to operate.
        //! @param[in] force_reload If this is `true`, this function always loads the asset data even if the asset is 
        //! in loaded state, and the existing asset data object will be replaced with new asset data object.
        //! If this is `false`, this function returns directly if the asset is already in loaded state, and the existing asset data 
        //! object will not be changed.
        LUNA_ASSET_API RV load_asset_default_data(asset_t asset, bool force_reload = false);

        //! Gets the asset state.
        //! @param[in] asset The asset handle of the asset to query.
        //! @return Returns the asset state of the specified asset.
        LUNA_ASSET_API AssetState get_asset_state(asset_t asset);

        //! Saves the asset data to files.
        //! @details This function saves the asset data synchronously. To save asset data asynchronously, call
        //! this function in the thread you want to use for asset saving, like a background thread or a 
        //! worker thread in job system.
        //! @param[in] asset The asset handle of the asset to operate.
        LUNA_ASSET_API RV save_asset(asset_t asset);

        //! Closes the asset registry.
        //! @details This call removes all registered assets and asset types, and invalidates all asset handles.
        LUNA_ASSET_API void close();

        //! @}
    }

    //! @addtogroup Asset
    //! @{
    //! @defgroup AssetError Asset Errors
    //! @}

    namespace AssetError
    {
        //! @addtogroup AssetError
        //! @{

        LUNA_ASSET_API errcat_t errtype();

        //! The metadata file of the specified asset is not found.
        LUNA_ASSET_API ErrCode meta_file_not_found();
        //! The asset type is not registered to the asset system.
        LUNA_ASSET_API ErrCode unknown_asset_type();
        //! The asset handle is not registered to the system (asset state is @ref AssetState::unregistered). 
        LUNA_ASSET_API ErrCode asset_not_registered();
        //! The asset handle is already registered to the system.
        LUNA_ASSET_API ErrCode asset_already_registered();
        //! The asset path is empty.
        LUNA_ASSET_API ErrCode empty_asset_path();
        //! The asset data is not loaded yet.
        LUNA_ASSET_API ErrCode asset_data_not_loaded();
        //! The asset data is currently loading by another thread.
        LUNA_ASSET_API ErrCode asset_data_loading();

        //! @}
    }
    
    struct Module;
    LUNA_ASSET_API Module* module_asset();
}