/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Texture.hpp
* @author JXMaster
* @date 2020/5/9
*/
#pragma once
#include "../StudioHeader.hpp"

namespace Luna
{
    RV register_static_texture_asset_type();
    void register_texture_editor();
    void register_texture_importer();
    Name get_static_texture_asset_type();
}