/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file PaletteIcons.hpp
* @author JXMaster
* @date 2026/6/11
*/
#pragma once
#include <Luna/GUICore/GUICore.hpp>
#include <Luna/Runtime/HashMap.hpp>
#include <Luna/VG/ShapeBuffer.hpp>

namespace Luna
{
    namespace GUIEditor
    {
        struct PaletteIcons
        {
            Ref<VG::IShapeBuffer> shape_buffer;
            HashMap<Name, GUICore::ShapeDesc> icons;
            GUICore::ShapeDesc fallback_icon;
        };

        void init_palette_icons(PaletteIcons& icons);
        GUICore::ShapeDesc& palette_icon(PaletteIcons& icons, const Name& type);
    }
}
