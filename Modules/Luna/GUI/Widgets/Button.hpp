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
#include "../Widget.hpp"
#include <Luna/Runtime/Functional.hpp>

namespace Luna
{
    namespace GUI
    {
        enum class ButtonStateType
        {
            normal = 0,
            hovered = 1,
            pressed = 2,
        };

        struct ButtonState
        {
            lustruct("GUI::ButtonState", "9ebc5d25-7386-4229-a77e-2368321193b5");
            
            ButtonStateType state_type = ButtonStateType::normal;
            bool capture_mouse_event = false;
            bool triggered = false;
        };

        struct Button : Widget
        {
            lustruct("GUI::Button", "1ba55eff-b981-42a8-bb7a-d21c8cbfbe0e");

            Function<RV(void)> m_on_click;
            ButtonState* m_button_state;

            LUNA_GUI_API virtual f32 get_desired_size_x(DesiredSizeType type, const f32* suggested_size_y) override;
            LUNA_GUI_API virtual f32 get_desired_size_y(DesiredSizeType type, const f32* suggested_size_x) override;
            LUNA_GUI_API virtual RV begin_update(IContext* ctx) override;
            LUNA_GUI_API virtual RV layout(IContext* ctx, const OffsetRectF& layout_rect) override;
            LUNA_GUI_API virtual RV handle_event(IContext* ctx, object_t e, bool& handled) override;
            LUNA_GUI_API virtual RV update(IContext* ctx) override;
            LUNA_GUI_API virtual RV draw(IContext* ctx, IDrawList* draw_list, IDrawList* overlay_draw_list) override;
        };
    }
}