/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Button.hpp
* @author JXMaster
* @date 2024/7/25
*/
#pragma once
#include "Widget.hpp"
#include "Container.hpp"
#include <Luna/Runtime/Event.hpp>

namespace Luna
{
    namespace GUI
    {
        struct Button : Widget, virtual IContainer
        {
            lustruct("GUI::Button", "1ba55eff-b981-42a8-bb7a-d21c8cbfbe0e");

            Ref<IWidget> body;
            Event<void(void)> on_click;
            ObjRef button_state;

            LUNA_GUI_API virtual RV begin_update(IContext* ctx) override;
            LUNA_GUI_API virtual RV draw(IContext* ctx, IDrawList* draw_list) override;
        };
    }
}