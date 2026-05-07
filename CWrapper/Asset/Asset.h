#pragma once

#include "../Runtime/Runtime.h"

#include <stdint.h>

#if defined(_WIN32)
#define LUNA_ASSET_C_API __declspec(dllexport)
#else
#define LUNA_ASSET_C_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LunaAssetHandle
{
    void* handle;
} LunaAssetHandle;

typedef luna_errcode_t (*LunaAssetOnLoadAsset)(LunaAssetHandle asset, const char* path, luna_handle_t* out_data, void* userdata);
typedef luna_errcode_t (*LunaAssetOnLoadAssetDefaultData)(LunaAssetHandle asset, luna_handle_t* out_data, void* userdata);
typedef luna_errcode_t (*LunaAssetOnSaveAsset)(LunaAssetHandle asset, const char* path, luna_handle_t data, void* userdata);
typedef luna_errcode_t (*LunaAssetOnSetAssetData)(LunaAssetHandle asset, luna_handle_t data, void* userdata);
typedef void (*LunaAssetOnGetReferredAssets)(LunaAssetHandle asset, LunaAssetHandle* out_assets, uint64_t capacity, uint64_t* out_count, void* userdata);

LUNA_ASSET_C_API luna_errcode_t luna_asset_init_module(void);
LUNA_ASSET_C_API void luna_asset_free_string(const char* text);
LUNA_ASSET_C_API void luna_asset_register_asset_type(
    const char* type,
    LunaAssetOnLoadAsset on_load_asset,
    LunaAssetOnLoadAssetDefaultData on_load_asset_default_data,
    LunaAssetOnSaveAsset on_save_asset,
    LunaAssetOnSetAssetData on_set_asset_data,
    LunaAssetOnGetReferredAssets on_get_referred_assets,
    void* userdata);
LUNA_ASSET_C_API luna_errcode_t luna_asset_get_asset(const LunaGuid* guid, LunaAssetHandle* out_asset);
LUNA_ASSET_C_API luna_errcode_t luna_asset_register_asset(LunaAssetHandle asset, const char* type);
LUNA_ASSET_C_API luna_errcode_t luna_asset_new_asset(const char* path, const char* type, int32_t save_meta_to_file, LunaAssetHandle* out_asset);
LUNA_ASSET_C_API luna_errcode_t luna_asset_load_assets_meta(const char* path, int32_t allow_overwrite);
LUNA_ASSET_C_API luna_errcode_t luna_asset_load_asset_meta(LunaAssetHandle asset);
LUNA_ASSET_C_API luna_errcode_t luna_asset_save_asset_meta(LunaAssetHandle asset);
LUNA_ASSET_C_API luna_errcode_t luna_asset_get_asset_by_path(const char* path, LunaAssetHandle* out_asset);
LUNA_ASSET_C_API void luna_asset_get_asset_guid(LunaAssetHandle asset, LunaGuid* out_guid);
LUNA_ASSET_C_API luna_errcode_t luna_asset_get_asset_path(LunaAssetHandle asset, const char** out_path);
LUNA_ASSET_C_API luna_errcode_t luna_asset_set_asset_path(LunaAssetHandle asset, const char* path);
LUNA_ASSET_C_API luna_errcode_t luna_asset_get_asset_name(LunaAssetHandle asset, const char** out_name);
LUNA_ASSET_C_API luna_errcode_t luna_asset_get_asset_type(LunaAssetHandle asset, const char** out_type);
LUNA_ASSET_C_API void luna_asset_set_asset_type(LunaAssetHandle asset, const char* type);
LUNA_ASSET_C_API luna_errcode_t luna_asset_get_asset_files_count(LunaAssetHandle asset, uint64_t* out_count);
LUNA_ASSET_C_API luna_errcode_t luna_asset_get_asset_file(LunaAssetHandle asset, uint64_t index, const char** out_name);
LUNA_ASSET_C_API luna_errcode_t luna_asset_delete_asset(LunaAssetHandle asset);
LUNA_ASSET_C_API luna_errcode_t luna_asset_move_asset(LunaAssetHandle asset, const char* new_path);
LUNA_ASSET_C_API luna_errcode_t luna_asset_copy_asset(LunaAssetHandle asset, const char* new_path, const LunaGuid* guid, LunaAssetHandle* out_asset);
LUNA_ASSET_C_API luna_handle_t luna_asset_get_asset_data(LunaAssetHandle asset);
LUNA_ASSET_C_API luna_errcode_t luna_asset_set_asset_data(LunaAssetHandle asset, luna_handle_t data);
LUNA_ASSET_C_API luna_errcode_t luna_asset_load_asset(LunaAssetHandle asset, int32_t force_reload);
LUNA_ASSET_C_API luna_errcode_t luna_asset_load_asset_default_data(LunaAssetHandle asset, int32_t force_reload);
LUNA_ASSET_C_API uint32_t luna_asset_get_asset_state(LunaAssetHandle asset);
LUNA_ASSET_C_API luna_errcode_t luna_asset_save_asset(LunaAssetHandle asset);
LUNA_ASSET_C_API void luna_asset_get_referred_assets(LunaAssetHandle asset, LunaAssetHandle* out_assets, uint64_t capacity, uint64_t* out_count);

#ifdef __cplusplus
}
#endif
