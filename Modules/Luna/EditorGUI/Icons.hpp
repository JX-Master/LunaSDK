/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Icons.hpp
* @author JXMaster
* @date 2026/7/22
*/
#pragma once
#include "Base.hpp"

namespace Luna
{
    namespace EditorGUI
    {
        //! Identifies one icon in the built-in GUI icon pack.
        enum class IconName : u16
        {
#include "IconNames.inl"
            count
        };

        //! Identifies one visual weight of an icon.
        enum class IconWeight : u8
        {
            //! The default outline weight.
            regular,
            //! A heavier outline used by compact action glyphs.
            bold,
            //! The solid filled variant.
            fill,
            //! The two-layer variant with a translucent secondary layer.
            duotone,
            //! The number of supported icon weights.
            count
        };

        //! Describes one icon element.
        struct IconDesc
        {
            //! Visual weight. If the requested weight is not embedded, the regular weight is used.
            IconWeight weight = IconWeight::regular;
            //! Icon tint. A negative alpha uses the current `gui.icon.color` style value.
            Float4U tint = Float4U(0.0f, 0.0f, 0.0f, -1.0f);
            //! Preferred square size. A non-positive value uses the current `gui.icon.size` style value.
            f32 size = 0.0f;
            //! Opacity multiplier applied to the secondary layer of duotone icons.
            f32 secondary_opacity = 1.0f;
        };

        //! Adds one built-in icon as an ordinary, non-interactive child element.
        //! @param[in] context The GUI context receiving the element.
        //! @param[in] id Element ID.
        //! @param[in] value Icon name.
        //! @param[in] layout Layout configuration. Fit sizing uses the icon size resolved from @p desc and Style.
        //! @param[in] desc Icon appearance.
        //! @return Returns the created element.
        //! @remark The icon preserves its square aspect ratio and is centered inside its resolved layout rectangle.
        LUNA_EDITOR_GUI_API GUI::ElementHandle icon(GUI::IContext* context, id_t id, IconName value,
            const GUI::LayoutConfig& layout = GUI::LayoutConfig(), const IconDesc& desc = IconDesc());

        //! Checks whether the requested icon weight is embedded in the built-in pack.
        //! @param[in] value Icon name.
        //! @param[in] weight Requested visual weight.
        //! @return Returns `true` if the exact icon-weight pair is embedded. Rendering falls back to regular weight
        //! when this function returns `false` for another valid weight.
        LUNA_EDITOR_GUI_API bool has_icon(IconName value, IconWeight weight);
    }
}
