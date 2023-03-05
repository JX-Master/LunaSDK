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
#include <Runtime/Result.hpp>
#include <Runtime/Ref.hpp>
#include <JobSystem/JobSystem.hpp>
#include <Runtime/Path.hpp>

#ifndef LUNA_ASSET_API
#define LUNA_ASSET_API
#endif

namespace Luna
{
	namespace Asset
	{
		//! The asset handle.
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

		enum class AssetState
		{
			//! The asset handle is not registered. This asset should be registered first by
			//! calling `register_asset` or `new_asset`.
			unregistered = 0,
			//! The asset data is not loaded.
			unloaded = 1,
			//! The asset data is loading.
			loading = 2,
			//! The asset data is loaded.
			loaded = 3
		};

		struct AssetTypeDesc
		{
			//! The name of the asset type.
			Name name;
			//! The userdata object. The object will be kept by the asset system and provided to every callback function.
			ObjRef userdata;
			//! Called when the asset data is being loaded.
			R<ObjRef>(*on_load_asset)(object_t userdata, asset_t asset, const Path& path);
			//! Called when the asset data is being saved.
			RV(*on_save_asset)(object_t userdata, asset_t asset, const Path& path, object_t data);
			//! Called when the asset data is being set.
			RV(*on_set_asset_data)(object_t userdata, asset_t asset, object_t data);
		};

		//! Registers one asset type so the asset system can handle the asset.
		LUNA_ASSET_API void register_asset_type(const AssetTypeDesc& desc);

		//! Creates a new asset by specifying the path and type of the asset.
		//! The system creates asset entry for the new asset, and generates the asset meta file 
		//! for the new asset.
		//! @param[in] path The path to place the new created asset.
		//! If the asset with the specified path is already registered, this call does nothing and returnes
		//! the registered asset directly.
		//! @param[in] type The type of the asset.
		LUNA_ASSET_API R<asset_t> new_asset(const Path& path, const Name& type);

		//! Registers one existing asset to the system.
		//! The system reads asset GUID and type by reading the asset meta file.
		//! @param[in] path The path of the asset to register.
		//! If the asset with the specified path is already registered, this call does nothing and returnes
		//! the registered asset directly.
		LUNA_ASSET_API R<asset_t> register_asset(const Path& path);

		//! Gets or creates one asset entry. An asset is one block of application data that is stored on one asset file.
		//! This function returns the asset entry corresponding to the specified Asset ID.
		//! If the entry is not found, the asset system creates one new entry and binds the asset ID to that entry.
		//! 
		//! The asset entry cannot be removed after creation, nor can it be rebound to another asset ID.
		LUNA_ASSET_API asset_t get_asset(const Guid& guid = Guid(0, 0));

		//! Gets one asset by path.
		//! @param[in] path The path of the asset.
		LUNA_ASSET_API R<asset_t> get_asset_by_path(const Path& path);

		LUNA_ASSET_API Guid get_asset_guid(asset_t asset);

		//! Gets the asset VFS path.
		LUNA_ASSET_API Path get_asset_path(asset_t asset);

		//! Gets the asset name, which is the last node of the asset VFS path, excluding the extension.
		LUNA_ASSET_API Name get_asset_name(asset_t asset);

		LUNA_ASSET_API Name get_asset_type(asset_t asset);

		LUNA_ASSET_API RV set_asset_type(asset_t asset, const Name& type);

		//! Get all files associated to the specified asset.
		LUNA_ASSET_API R<Vector<Name>> get_asset_files(asset_t asset);

		//! Deletes one asset and all its associated files.
		LUNA_ASSET_API RV delete_asset(asset_t asset);

		//! Moves all asset associated files to a new destination.
		LUNA_ASSET_API RV move_asset(asset_t asset, const Path& new_path);

		//! Gets the asset data object. All asset objects are boxed by the system.
		//! @param[in] asset The asset handle to operate on.
		//! @param[in] trigger_load If the asset is not loaded and this is `true`, this call calls `load_asset` to load the
		//! asset data and returns immediately with `nullptr`. The asset data may be ready at a future state.
		//! @param[in] block_until_loaded If the asset is not loaded and this is `true`, this call blocks the current thread 
		//! until the asset data is loaded, then returns the loaded asset data. Note that this may cause application stall 
		//! if improperly used.
		LUNA_ASSET_API ObjRef get_asset_data(asset_t asset, bool trigger_load = true, bool block_until_loaded = false);

		template <typename _Ty>
		Ref<_Ty> get_asset_data(asset_t asset, bool trigger_load = true, bool block_until_loaded = false)
		{
			return Ref<_Ty>(get_asset_data(asset, trigger_load, block_until_loaded));
		}

		//! Sets the asset data object.
		//! @param[in] asset The asset handle to operate on.
		//! @param[in] data The asset data to set. This must be `nullptr` or a valid pointer to a boxed instance.
		//! If this is `nullptr`, the asset data will be cleared, and the asset will be in `unloaded` state.
		LUNA_ASSET_API RV set_asset_data(asset_t asset, object_t data);

		LUNA_ASSET_API void load_asset(asset_t asset, bool force_reload = false);

		LUNA_ASSET_API AssetState get_asset_state(asset_t asset);

		LUNA_ASSET_API void wait_asset(asset_t asset);

		LUNA_ASSET_API const Error& get_asset_loading_result(asset_t asset);

		//! Saves the asset synchronously.
		LUNA_ASSET_API RV save_asset(asset_t asset);
	}

	namespace AssetError
	{
		LUNA_ASSET_API errcat_t errtype();
		LUNA_ASSET_API ErrCode meta_file_not_found();
		LUNA_ASSET_API ErrCode unknown_asset_type();
		LUNA_ASSET_API ErrCode asset_not_registered();
		LUNA_ASSET_API ErrCode asset_data_not_loaded();
	}
}