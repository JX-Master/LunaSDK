/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Window.hpp
* @author JXMaster
* @date 2024/6/5
*/
#pragma once
#include "../../Widget.hpp"
#include "../../Widgets.hpp"

namespace Luna
{
    namespace GUI
    {
        struct WindowState
        {
            lustruct("GUI::WindowState", "{be78880a-6299-46a2-8bbc-97f28cad5c26}");

            Int2U pos;
            UInt2U size;
            bool moving = false;
            bool resizing = false;
        };
        struct Window : Widget
        {
            lustruct("GUI::Window", "{cf4e9631-2669-4841-b244-3122da21d4af}");

            Name title;
            WindowFlag flags;
            Int2U pos;
            UInt2U size;
            Condition pos_condition;
            Condition size_condition;

            Window() :
                pos_condition(Condition::never),
                size_condition(Condition::never) {}

            virtual RV update(IContext* ctx, const OffsetRectF& layout_rect) override;
            virtual RV draw(IContext* ctx, VG::IShapeDrawList* draw_list) override;
        };
    }
}