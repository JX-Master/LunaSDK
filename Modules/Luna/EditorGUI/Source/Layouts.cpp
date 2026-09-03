/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Layouts.cpp
* @author JXMaster
* @date 2026/7/13
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_EDITOR_GUI_API LUNA_EXPORT
#include "Internal.hpp"

namespace Luna
{
    namespace EditorGUI
    {
        namespace Internal
        {
            static GUI::ElementHandle begin_layout(GUI::IContext* context, id_t id, const c8* label,
                const GUI::LayoutConfig& layout)
            {
                luassert(context && id);
                GUI::ElementHandle element = begin_element(context, id, label ? label : "Layout", layout);
                context->set_child_paint_order_mode(element, GUI::ChildPaintOrderMode::shared);
                return element;
            }

            void set_flex_layout(GUI::IContext* context, const GUI::ElementHandle& element,
                const GUI::FlexLayoutDesc& source, GUI::LayoutAxis axis)
            {
                GUI::FlexLayoutDesc* desc = copy_frame(context, source);
                desc->axis = axis;
                GUI::LayoutCallbackConfig callbacks;
                callbacks.algorithm = Name(axis == GUI::LayoutAxis::x ? "gui.h_layout" : "gui.v_layout");
                callbacks.measure_callback = GUI::measure_flex;
                callbacks.callback = GUI::layout_flex;
                callbacks.userdata = desc;
                context->set_layout_callback_config(element, callbacks);
                context->end_element();
            }
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle begin_h_layout(GUI::IContext* context, id_t id, const c8* label,
            const GUI::LayoutConfig& layout)
        {
            return Internal::begin_layout(context, id, label ? label : "Horizontal Layout", layout);
        }

        LUNA_EDITOR_GUI_API void end_h_layout(GUI::IContext* context, const GUI::ElementHandle& element,
            const GUI::FlexLayoutDesc& desc)
        {
            Internal::set_flex_layout(context, element, desc, GUI::LayoutAxis::x);
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle begin_v_layout(GUI::IContext* context, id_t id, const c8* label,
            const GUI::LayoutConfig& layout)
        {
            return Internal::begin_layout(context, id, label ? label : "Vertical Layout", layout);
        }

        LUNA_EDITOR_GUI_API void end_v_layout(GUI::IContext* context, const GUI::ElementHandle& element,
            const GUI::FlexLayoutDesc& desc)
        {
            Internal::set_flex_layout(context, element, desc, GUI::LayoutAxis::y);
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle begin_grid_layout(GUI::IContext* context, id_t id, const c8* label,
            const GUI::LayoutConfig& layout)
        {
            return Internal::begin_layout(context, id, label ? label : "Grid Layout", layout);
        }

        LUNA_EDITOR_GUI_API void end_grid_layout(GUI::IContext* context, const GUI::ElementHandle& element,
            const GUI::GridLayoutDesc& source)
        {
            GUI::GridLayoutDesc* desc = Internal::copy_frame(context, source);
            GUI::LayoutCallbackConfig callbacks;
            callbacks.algorithm = Name("gui.grid_layout");
            callbacks.measure_callback = GUI::measure_grid;
            callbacks.callback = GUI::layout_grid;
            callbacks.userdata = desc;
            context->set_layout_callback_config(element, callbacks);
            context->end_element();
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle begin_canvas_layout(GUI::IContext* context, id_t id, const c8* label,
            const GUI::LayoutConfig& layout)
        {
            return Internal::begin_layout(context, id, label ? label : "Canvas Layout", layout);
        }

        LUNA_EDITOR_GUI_API void end_canvas_layout(GUI::IContext* context, const GUI::ElementHandle& element,
            const GUI::CanvasLayoutDesc& source)
        {
            GUI::CanvasLayoutDesc* desc = Internal::copy_frame(context, source);
            GUI::CanvasLayoutItem* items = Internal::allocate_frame_array<GUI::CanvasLayoutItem>(context,
                source.items.size());
            for(usize i = 0; i < source.items.size(); ++i) items[i] = source.items[i];
            desc->items = Span<const GUI::CanvasLayoutItem>(items, source.items.size());
            GUI::LayoutCallbackConfig callbacks;
            callbacks.algorithm = Name("gui.canvas_layout");
            callbacks.callback = GUI::layout_canvas;
            callbacks.userdata = desc;
            context->set_layout_callback_config(element, callbacks);
            context->end_element();
        }
    }
}
