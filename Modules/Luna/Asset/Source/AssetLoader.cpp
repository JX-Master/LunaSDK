/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file AssetLoader.cpp
* @author JXMaster
* @date 2026/8/29
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_ASSET_API LUNA_EXPORT
#include "AssetLoader.hpp"
#include <Luna/Runtime/SelfIndexedHashMap.hpp>

namespace Luna
{
    namespace Asset
    {
        struct AssetLoaderDescExtractKey
        {
            Name operator()(const AssetLoaderDesc& v)
            {
                return v.name;
            }
        };

        Ref<IMutex> g_asset_loaders_mutex;
        SelfIndexedHashMap<Name, AssetLoaderDesc, AssetLoaderDescExtractKey> g_asset_loaders;

        void init_asset_loader()
        {
            g_asset_loaders_mutex = new_mutex();
        }
        void close_asset_loader()
        {
            g_asset_loaders.clear();
            g_asset_loaders.shrink_to_fit();
        }
        LUNA_ASSET_API void register_asset_loader(const AssetLoaderDesc& desc)
        {
            lucheck_msg(!desc.name.empty(), "Asset loader name must not be empty!");
            ObjRef old_userdata;
            {
                MutexGuard guard(g_asset_loaders_mutex);
                auto iter = g_asset_loaders.find(desc.name);
                if(iter != g_asset_loaders.end()) old_userdata = move(iter->userdata);
                g_asset_loaders.insert_or_assign(desc);
            }
        }
        R<AssetLoaderDesc> get_asset_loader_desc(const Name& name)
        {
            MutexGuard guard(g_asset_loaders_mutex);
            auto iter = g_asset_loaders.find(name);
            if(iter == g_asset_loaders.end()) return Asset::E_UNKNOWN_ASSET_LOADER;
            return *iter;
        }
    }
}
