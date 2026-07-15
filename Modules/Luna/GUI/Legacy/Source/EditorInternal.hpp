/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorInternal.hpp
* @author JXMaster
* @date 2026/7/12
*/
#pragma once
#include <Luna/GUICore/Context.hpp>

namespace Luna
{
    namespace GUI
    {
        namespace Internal
        {
            inline GUICore::ElementHandle begin_element(GUICore::IContext* context, GUICore::id_t id,
                const c8* debug_name)
            {
                GUICore::ElementHandle element = context->begin_element(id);
                context->set_element_debug_name(element, Name(debug_name));
                return element;
            }

            inline void push_layer(GUICore::IContext* context, GUICore::id_t id, const Float2U& screen_position,
                const c8* debug_name)
            {
                context->push_layer(id, screen_position);
                context->set_layer_debug_name(id, Name(debug_name));
            }
        }
    }
}
