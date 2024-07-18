/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Canvas.hpp
* @author JXMaster
* @date 2024/7/18
*/
#pragma once

#include "Widget.hpp"
#include "Container.hpp"

#ifndef LUNA_GUI_API
#define LUNA_GUI_API
#endif

namespace Luna
{
    namespace GUI
    {
        struct CanvasTransform
        {
            OffsetRectF anchor = OffsetRectF(0, 0, 0, 0);
            OffsetRectF offset = OffsetRectF(0, 0, 0, 0);
            f32 z_order = 0;
        };
        struct Canvas : Widget, virtual IContainer
        {
            lustruct("GUI::Canvas", "15c6b337-07b0-4760-91cd-cfaeeb398f37");

            //! The children of the canvas.
            Vector<Ref<IWidget>> children;

            CanvasTransform next_widget_transform;

            //! Internal states of the canvas.
            object_t state = nullptr;

            LUNA_GUI_API virtual RV update(IContext* ctx, const OffsetRectF& layout_rect) override;
            LUNA_GUI_API virtual RV draw(IContext* ctx, IDrawList* draw_list) override;

            LUNA_GUI_API virtual void add_child(IWidget* child) override;
            LUNA_GUI_API virtual Array<IWidget*> get_children() override;
        };
    }
}