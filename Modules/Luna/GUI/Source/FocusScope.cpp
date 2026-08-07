/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file FocusScope.cpp
* @author JXMaster
* @date 2026/7/16
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "Internal.hpp"

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API GUICore::ElementHandle begin_focus_scope(GUICore::IContext* context, id_t id,
            const c8* label, const GUICore::LayoutConfig& layout)
        {
            luassert(context && id);
            GUICore::ElementHandle element = Internal::begin_element(context, id,
                label ? label : "Focus Scope", layout);
            GUICore::Interactable interactable;
            interactable.focus_scope = id;
            context->set_interactable(element, interactable);
            return element;
        }

        LUNA_GUI_API void end_focus_scope(GUICore::IContext* context,
            const GUICore::ElementHandle& element)
        {
            luassert(context);
            GUICore::FlexLayoutDesc desc;
            desc.axis = GUICore::LayoutAxis::y;
            desc.cross_alignment = GUICore::FlexAlignment::stretch;
            Internal::set_flex_layout(context, element, desc, GUICore::LayoutAxis::y);
        }
    }
}
