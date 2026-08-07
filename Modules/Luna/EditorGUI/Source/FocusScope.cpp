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
#define LUNA_EDITOR_GUI_API LUNA_EXPORT
#include "Internal.hpp"

namespace Luna
{
    namespace EditorGUI
    {
        LUNA_EDITOR_GUI_API GUI::ElementHandle begin_focus_scope(GUI::IContext* context, id_t id,
            const c8* label, const GUI::LayoutConfig& layout)
        {
            luassert(context && id);
            GUI::ElementHandle element = Internal::begin_element(context, id,
                label ? label : "Focus Scope", layout);
            GUI::Interactable interactable;
            interactable.focus_scope = id;
            context->set_interactable(element, interactable);
            return element;
        }

        LUNA_EDITOR_GUI_API void end_focus_scope(GUI::IContext* context,
            const GUI::ElementHandle& element)
        {
            luassert(context);
            GUI::FlexLayoutDesc desc;
            desc.axis = GUI::LayoutAxis::y;
            desc.cross_alignment = GUI::FlexAlignment::stretch;
            Internal::set_flex_layout(context, element, desc, GUI::LayoutAxis::y);
        }
    }
}
