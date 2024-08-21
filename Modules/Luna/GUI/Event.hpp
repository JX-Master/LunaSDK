/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Event.hpp
* @author JXMaster
* @date 2024/7/23
*/
#pragma once
#include <Luna/Runtime/TypeInfo.hpp>
#include <Luna/HID/KeyCode.hpp>
#include <Luna/Runtime/Object.hpp>
#include <Luna/Runtime/Result.hpp>

#ifndef LUNA_GUI_API
#define LUNA_GUI_API
#endif

namespace Luna
{
    namespace GUI
    {
        struct MouseEvent
        {
            lustruct("GUI::MouseEvent", "0dd0858a-e607-4fe6-ad96-c10628edf90b");
            
            //! The X coordinate of the mouse event in screen space.
            f32 x;
            //! The Y coordinate of the mouse event in screen space.
            f32 y;
        };

        struct MouseMoveEvent : MouseEvent
        {
            lustruct("GUI::MouseMoveEvent", "211e4be8-5cac-4c85-8d56-7a45869d81a9");

            
        };

        struct MouseButtonEvent : MouseEvent
        {
            lustruct("GUI::MouseButtonEvent", "2e8d7037-c231-4cb1-82a0-a17ed28c749a");

            HID::MouseButton button;
            bool pressed;
        };

        struct IContext;
        struct IWidget;
        LUNA_GUI_API RV dispatch_event_by_pos(IContext* ctx, IWidget* widget, object_t e, f32 x, f32 y, bool& handled);
    }
}