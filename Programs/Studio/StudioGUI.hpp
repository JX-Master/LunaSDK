/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file StudioGUI.hpp
* @author JXMaster
* @date 2026/5/22
*/
#pragma once
#include <Luna/Asset/Asset.hpp>

namespace Luna
{
    namespace GUICore
    {
        struct IContext;
    }

    bool gui_edit_asset_path(GUICore::IContext* context, const c8* label, Asset::asset_t& asset, String& path_text, const c8* failure_title = "Failed to set asset reference");
}
