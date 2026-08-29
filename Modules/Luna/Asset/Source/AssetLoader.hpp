/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file AssetLoader.hpp
* @author JXMaster
* @date 2026/8/29
*/
#pragma once
#include "../Asset.hpp"
#include <Luna/Runtime/Mutex.hpp>

namespace Luna
{
    namespace Asset
    {
        R<AssetLoaderDesc> get_asset_loader_desc(const Name& name);
        void init_asset_loader();
        void close_asset_loader();
        extern Ref<IMutex> g_asset_loaders_mutex;
    }
}
