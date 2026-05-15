#include "Asset.h"

#include <Luna/Asset/Asset.hpp>
#include <Luna/Runtime/Error.hpp>
#include <Luna/Runtime/Memory.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Result.hpp>

#include <cstring>
#include <string>

namespace
{
luna_errcode_t from_errcode(Luna::ErrCode code)
{
    return static_cast<luna_errcode_t>(code.code);
}

luna_errcode_t from_result(const Luna::RV& result)
{
    return from_errcode(result.errcode());
}

Luna::Guid to_guid(const LunaGuid& guid)
{
    return Luna::Guid(guid.high, guid.low);
}

LunaGuid from_guid(const Luna::Guid& guid)
{
    return LunaGuid{guid.high, guid.low};
}

Luna::Asset::asset_t to_asset(LunaAssetHandle asset)
{
    return Luna::Asset::asset_t(asset.handle);
}

LunaAssetHandle from_asset(Luna::Asset::asset_t asset)
{
    return LunaAssetHandle{asset.handle};
}

const char* duplicate_string(const char* source)
{
    if (!source)
    {
        return nullptr;
    }
    auto size = std::strlen(source);
    auto* buffer = static_cast<char*>(Luna::memalloc(size + 1));
    if (!buffer)
    {
        return nullptr;
    }
    std::memcpy(buffer, source, size + 1);
    return buffer;
}

struct ManagedAssetTypeCallbacks
{
    lustruct("AssetC.ManagedAssetTypeCallbacks", "{6C6A9D59-3B88-47BC-8FA5-C921F9F0D717}");
    LunaAssetOnLoadAsset on_load_asset = nullptr;
    LunaAssetOnLoadAssetDefaultData on_load_asset_default_data = nullptr;
    LunaAssetOnSaveAsset on_save_asset = nullptr;
    LunaAssetOnSetAssetData on_set_asset_data = nullptr;
    LunaAssetOnGetReferredAssets on_get_referred_assets = nullptr;
    void* userdata = nullptr;
    ManagedAssetTypeCallbacks() = default;
    ManagedAssetTypeCallbacks(
        LunaAssetOnLoadAsset on_load_asset,
        LunaAssetOnLoadAssetDefaultData on_load_asset_default_data,
        LunaAssetOnSaveAsset on_save_asset,
        LunaAssetOnSetAssetData on_set_asset_data,
        LunaAssetOnGetReferredAssets on_get_referred_assets,
        void* userdata) :
        on_load_asset(on_load_asset),
        on_load_asset_default_data(on_load_asset_default_data),
        on_save_asset(on_save_asset),
        on_set_asset_data(on_set_asset_data),
        on_get_referred_assets(on_get_referred_assets),
        userdata(userdata) {}
};

ManagedAssetTypeCallbacks* get_callbacks(Luna::object_t userdata)
{
    return reinterpret_cast<ManagedAssetTypeCallbacks*>(userdata);
}

Luna::Asset::asset_t to_asset_handle(LunaAssetHandle asset)
{
    return Luna::Asset::asset_t(asset.handle);
}

Luna::R<Luna::ObjRef> on_load_asset_trampoline(Luna::object_t userdata, Luna::Asset::asset_t asset, const Luna::Path& path)
{
    auto* callbacks = get_callbacks(userdata);
    if (!callbacks || !callbacks->on_load_asset)
    {
        return Luna::BasicError::not_supported();
    }
    luna_handle_t data = nullptr;
    auto result = callbacks->on_load_asset(from_asset(asset), path.encode().c_str(), &data, callbacks->userdata);
    if (result)
    {
        return Luna::ErrCode(static_cast<Luna::usize>(result));
    }
    return Luna::ObjRef(data);
}

Luna::R<Luna::ObjRef> on_load_asset_default_data_trampoline(Luna::object_t userdata, Luna::Asset::asset_t asset)
{
    auto* callbacks = get_callbacks(userdata);
    if (!callbacks || !callbacks->on_load_asset_default_data)
    {
        return Luna::BasicError::not_supported();
    }
    luna_handle_t data = nullptr;
    auto result = callbacks->on_load_asset_default_data(from_asset(asset), &data, callbacks->userdata);
    if (result)
    {
        return Luna::ErrCode(static_cast<Luna::usize>(result));
    }
    return Luna::ObjRef(data);
}

Luna::RV on_save_asset_trampoline(Luna::object_t userdata, Luna::Asset::asset_t asset, const Luna::Path& path, Luna::object_t data)
{
    auto* callbacks = get_callbacks(userdata);
    if (!callbacks || !callbacks->on_save_asset)
    {
        return Luna::BasicError::not_supported();
    }
    auto result = callbacks->on_save_asset(from_asset(asset), path.encode().c_str(), data, callbacks->userdata);
    return Luna::ErrCode(static_cast<Luna::usize>(result));
}

Luna::RV on_set_asset_data_trampoline(Luna::object_t userdata, Luna::Asset::asset_t asset, Luna::object_t data)
{
    auto* callbacks = get_callbacks(userdata);
    if (!callbacks || !callbacks->on_set_asset_data)
    {
        return Luna::ok;
    }
    auto result = callbacks->on_set_asset_data(from_asset(asset), data, callbacks->userdata);
    return Luna::ErrCode(static_cast<Luna::usize>(result));
}

void on_get_referred_assets_trampoline(Luna::object_t userdata, Luna::Asset::asset_t asset, Luna::Vector<Luna::Asset::asset_t>& out_referred_assets)
{
    auto* callbacks = get_callbacks(userdata);
    if (!callbacks || !callbacks->on_get_referred_assets)
    {
        return;
    }
    uint64_t count = 0;
    callbacks->on_get_referred_assets(from_asset(asset), nullptr, 0, &count, callbacks->userdata);
    if (!count)
    {
        return;
    }
    auto old_size = out_referred_assets.size();
    out_referred_assets.resize(old_size + static_cast<Luna::usize>(count));
    callbacks->on_get_referred_assets(from_asset(asset), reinterpret_cast<LunaAssetHandle*>(out_referred_assets.data() + old_size), count, &count, callbacks->userdata);
    if (count < static_cast<uint64_t>(out_referred_assets.size() - old_size))
    {
        out_referred_assets.resize(old_size + static_cast<Luna::usize>(count));
    }
}
}

extern "C"
{
LUNA_ASSET_C_API luna_errcode_t luna_asset_init_module(void)
{
    Luna::Module* module = Luna::module_asset();
    Luna::RV result = Luna::add_module(module);
    if (!result.valid())
    {
        return from_result(result);
    }
    result = Luna::init_module(module);
    return from_result(result);
}

LUNA_ASSET_C_API void luna_asset_free_string(const char* text)
{
    if (text)
    {
        Luna::memfree(const_cast<char*>(text));
    }
}

LUNA_ASSET_C_API void luna_asset_register_asset_type(
    const char* type,
    LunaAssetOnLoadAsset on_load_asset,
    LunaAssetOnLoadAssetDefaultData on_load_asset_default_data,
    LunaAssetOnSaveAsset on_save_asset,
    LunaAssetOnSetAssetData on_set_asset_data,
    LunaAssetOnGetReferredAssets on_get_referred_assets,
    void* userdata)
{
    if (!type)
    {
        return;
    }
    static bool s_callbacks_type_registered = []() { Luna::register_boxed_type<ManagedAssetTypeCallbacks>(); return true; }();
    (void)s_callbacks_type_registered;

    Luna::Asset::AssetTypeDesc desc;
    desc.name = type;
    desc.userdata = Luna::new_object<ManagedAssetTypeCallbacks>(
        on_load_asset,
        on_load_asset_default_data,
        on_save_asset,
        on_set_asset_data,
        on_get_referred_assets,
        userdata);
    desc.on_load_asset = on_load_asset ? on_load_asset_trampoline : nullptr;
    desc.on_load_asset_default_data = on_load_asset_default_data ? on_load_asset_default_data_trampoline : nullptr;
    desc.on_save_asset = on_save_asset ? on_save_asset_trampoline : nullptr;
    desc.on_set_asset_data = on_set_asset_data ? on_set_asset_data_trampoline : nullptr;
    desc.on_get_referred_assets = on_get_referred_assets ? on_get_referred_assets_trampoline : nullptr;
    Luna::Asset::register_asset_type(desc);
}

LUNA_ASSET_C_API luna_errcode_t luna_asset_get_asset(const LunaGuid* guid, LunaAssetHandle* out_asset)
{
    if (!guid || !out_asset)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_asset = from_asset(Luna::Asset::get_asset(to_guid(*guid)));
    return 0;
}

LUNA_ASSET_C_API luna_errcode_t luna_asset_register_asset(LunaAssetHandle asset, const char* type)
{
    if (!asset.handle || !type)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::Asset::register_asset(to_asset(asset), type));
}

LUNA_ASSET_C_API luna_errcode_t luna_asset_new_asset(const char* path, const char* type, int32_t save_meta_to_file, LunaAssetHandle* out_asset)
{
    if (!path || !type || !out_asset)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    auto result = Luna::Asset::new_asset(path, type, save_meta_to_file != 0);
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }
    *out_asset = from_asset(result.get());
    return 0;
}

LUNA_ASSET_C_API luna_errcode_t luna_asset_load_assets_meta(const char* path, int32_t allow_overwrite)
{
    if (!path)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::Asset::load_assets_meta(path, allow_overwrite != 0));
}

LUNA_ASSET_C_API luna_errcode_t luna_asset_load_asset_meta(LunaAssetHandle asset)
{
    if (!asset.handle)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::Asset::load_asset_meta(to_asset(asset)));
}

LUNA_ASSET_C_API luna_errcode_t luna_asset_save_asset_meta(LunaAssetHandle asset)
{
    if (!asset.handle)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::Asset::save_asset_meta(to_asset(asset)));
}

LUNA_ASSET_C_API luna_errcode_t luna_asset_get_asset_by_path(const char* path, LunaAssetHandle* out_asset)
{
    if (!path || !out_asset)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    auto result = Luna::Asset::get_asset_by_path(path);
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }
    *out_asset = from_asset(result.get());
    return 0;
}

LUNA_ASSET_C_API void luna_asset_get_asset_guid(LunaAssetHandle asset, LunaGuid* out_guid)
{
    if (!asset.handle || !out_guid)
    {
        return;
    }
    *out_guid = from_guid(Luna::Asset::get_asset_guid(to_asset(asset)));
}

LUNA_ASSET_C_API luna_errcode_t luna_asset_get_asset_path(LunaAssetHandle asset, const char** out_path)
{
    if (!asset.handle || !out_path)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_path = nullptr;
    auto encoded = Luna::Asset::get_asset_path(to_asset(asset)).encode();
    *out_path = duplicate_string(encoded.c_str());
    return *out_path ? 0 : from_errcode(Luna::BasicError::out_of_memory());
}

LUNA_ASSET_C_API luna_errcode_t luna_asset_set_asset_path(LunaAssetHandle asset, const char* path)
{
    if (!asset.handle || !path)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::Asset::set_asset_path(to_asset(asset), path));
}

LUNA_ASSET_C_API luna_errcode_t luna_asset_get_asset_name(LunaAssetHandle asset, const char** out_name)
{
    if (!asset.handle || !out_name)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_name = nullptr;
    *out_name = duplicate_string(Luna::Asset::get_asset_name(to_asset(asset)).c_str());
    return *out_name ? 0 : from_errcode(Luna::BasicError::out_of_memory());
}

LUNA_ASSET_C_API luna_errcode_t luna_asset_get_asset_type(LunaAssetHandle asset, const char** out_type)
{
    if (!asset.handle || !out_type)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_type = nullptr;
    *out_type = duplicate_string(Luna::Asset::get_asset_type(to_asset(asset)).c_str());
    return *out_type ? 0 : from_errcode(Luna::BasicError::out_of_memory());
}

LUNA_ASSET_C_API void luna_asset_set_asset_type(LunaAssetHandle asset, const char* type)
{
    if (!asset.handle || !type)
    {
        return;
    }
    Luna::Asset::set_asset_type(to_asset(asset), type);
}

LUNA_ASSET_C_API luna_errcode_t luna_asset_get_asset_files_count(LunaAssetHandle asset, uint64_t* out_count)
{
    if (!asset.handle || !out_count)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    Luna::Vector<Luna::Name> files;
    auto result = Luna::Asset::get_asset_files(to_asset(asset), files);
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }
    *out_count = static_cast<uint64_t>(files.size());
    return 0;
}

LUNA_ASSET_C_API luna_errcode_t luna_asset_get_asset_file(LunaAssetHandle asset, uint64_t index, const char** out_name)
{
    if (!asset.handle || !out_name)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_name = nullptr;
    Luna::Vector<Luna::Name> files;
    auto result = Luna::Asset::get_asset_files(to_asset(asset), files);
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }
    if (index >= files.size())
    {
        return from_errcode(Luna::BasicError::out_of_range());
    }
    *out_name = duplicate_string(files[static_cast<Luna::usize>(index)].c_str());
    return *out_name ? 0 : from_errcode(Luna::BasicError::out_of_memory());
}

LUNA_ASSET_C_API luna_errcode_t luna_asset_delete_asset(LunaAssetHandle asset)
{
    if (!asset.handle)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::Asset::delete_asset(to_asset(asset)));
}

LUNA_ASSET_C_API luna_errcode_t luna_asset_move_asset(LunaAssetHandle asset, const char* new_path)
{
    if (!asset.handle || !new_path)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::Asset::move_asset(to_asset(asset), new_path));
}

LUNA_ASSET_C_API luna_errcode_t luna_asset_copy_asset(LunaAssetHandle asset, const char* new_path, const LunaGuid* guid, LunaAssetHandle* out_asset)
{
    if (!asset.handle || !new_path || !guid || !out_asset)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    auto result = Luna::Asset::copy_asset(to_asset(asset), new_path, to_guid(*guid));
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }
    *out_asset = from_asset(result.get());
    return 0;
}

LUNA_ASSET_C_API luna_handle_t luna_asset_get_asset_data(LunaAssetHandle asset)
{
    if (!asset.handle)
    {
        return nullptr;
    }
    Luna::ObjRef data = Luna::Asset::get_asset_data(to_asset(asset));
    return data.detach();
}

LUNA_ASSET_C_API luna_errcode_t luna_asset_set_asset_data(LunaAssetHandle asset, luna_handle_t data)
{
    if (!asset.handle)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::Asset::set_asset_data(to_asset(asset), data));
}

LUNA_ASSET_C_API luna_errcode_t luna_asset_load_asset(LunaAssetHandle asset, int32_t force_reload)
{
    if (!asset.handle)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::Asset::load_asset(to_asset(asset), force_reload != 0));
}

LUNA_ASSET_C_API luna_errcode_t luna_asset_load_asset_default_data(LunaAssetHandle asset, int32_t force_reload)
{
    if (!asset.handle)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::Asset::load_asset_default_data(to_asset(asset), force_reload != 0));
}

LUNA_ASSET_C_API uint32_t luna_asset_get_asset_state(LunaAssetHandle asset)
{
    return asset.handle ? static_cast<uint32_t>(Luna::Asset::get_asset_state(to_asset(asset))) : 0;
}

LUNA_ASSET_C_API luna_errcode_t luna_asset_save_asset(LunaAssetHandle asset)
{
    if (!asset.handle)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::Asset::save_asset(to_asset(asset)));
}

LUNA_ASSET_C_API void luna_asset_get_referred_assets(LunaAssetHandle asset, LunaAssetHandle* out_assets, uint64_t capacity, uint64_t* out_count)
{
    if (!out_count)
    {
        return;
    }
    *out_count = 0;
    if (!asset.handle)
    {
        return;
    }

    Luna::Vector<Luna::Asset::asset_t> assets;
    Luna::Asset::get_referred_assets(to_asset(asset), assets);
    *out_count = static_cast<uint64_t>(assets.size());
    if (!out_assets || capacity < *out_count)
    {
        return;
    }
    for (Luna::usize i = 0; i < assets.size(); ++i)
    {
        out_assets[i] = from_asset(assets[i]);
    }
}
}
