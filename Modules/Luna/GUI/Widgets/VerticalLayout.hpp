/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file VerticalLayout.hpp
* @author JXMaster
* @date 2024/7/13
*/
#pragma once

#include "Widget.hpp"
#include "Container.hpp"

namespace Luna
{
    namespace GUI
    {
        struct VerticalLayout : Widget, virtual IContainer
        {
            lustruct("GUI::VerticalLayout", "0017598f-95cf-4ce9-8ec2-238289f3e8ed");

            //! The children of the layout.
            Vector<Ref<IWidget>> children;

            LUNA_GUI_API virtual f32 get_desired_size_x(DesiredSizeType type, const f32* suggested_size_y) override;
            LUNA_GUI_API virtual f32 get_desired_size_y(DesiredSizeType type, const f32* suggested_size_x) override;

            LUNA_GUI_API virtual RV begin_update(IContext* ctx) override;
            LUNA_GUI_API virtual RV layout(IContext* ctx, const OffsetRectF& layout_rect) override;
            LUNA_GUI_API virtual RV update(IContext* ctx) override;
            LUNA_GUI_API virtual RV draw(IContext* ctx, IDrawList* draw_list) override;

            LUNA_GUI_API virtual void add_child(IWidget* child) override;
            LUNA_GUI_API virtual void get_children(Vector<IWidget*>& out_children) override;
            LUNA_GUI_API virtual usize get_num_children() override;
        };
    }
}