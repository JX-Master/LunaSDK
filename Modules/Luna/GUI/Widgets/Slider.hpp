/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Slider.hpp
* @author JXMaster
* @date 2024/7/29
*/
#pragma once
#include "../Widget.hpp"

namespace Luna
{
    namespace GUI
    {
        enum class SliderStateType
        {
            normal = 0,
            hovered = 1,
            pressed = 2,
        };
        struct SliderState
        {
            lustruct("GUI::SliderState", "59386533-9a66-4910-908a-93f832721c7b");

            SliderStateType state_type = SliderStateType::normal;
            bool sliding = false;
        };
        struct Slider : Widget
        {
            lustruct("GUI::Slider", "27bf4cf3-9b55-4754-a865-0fdc885af26a");

            f32* value = nullptr;
            f32 min_value = 0.0f;
            f32 max_value = 100.0f;
            f32 slope_power = 1.0f;
            f32 percentage = 0.0f;
            f32 handle_radius = 5.0f;
            Function<RV(f32)> on_value_changed;

            SliderState* slider_state;
            
            LUNA_GUI_API virtual f32 get_desired_size_x(DesiredSizeType type, const f32* suggested_size_y) override;
            LUNA_GUI_API virtual f32 get_desired_size_y(DesiredSizeType type, const f32* suggested_size_x) override;
            LUNA_GUI_API virtual RV begin_update(IContext* ctx) override;
            LUNA_GUI_API virtual RV handle_event(IContext* ctx, object_t e, bool& handled) override;
            LUNA_GUI_API virtual RV update(IContext* ctx) override;
            LUNA_GUI_API virtual RV draw(IContext* ctx, IDrawList* draw_list, IDrawList* overlay_draw_list) override;
        };
    }
}