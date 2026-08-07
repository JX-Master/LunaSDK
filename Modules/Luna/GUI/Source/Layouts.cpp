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
#define LUNA_GUI_API LUNA_EXPORT
#include "Internal.hpp"

namespace Luna
{
    namespace GUI
    {
        namespace Internal
        {
            static GUICore::ElementHandle begin_layout(GUICore::IContext* context, id_t id, const c8* label,
                const GUICore::LayoutConfig& layout)
            {
                luassert(context && id);
                return begin_element(context, id, label ? label : "Layout", layout);
            }

            void set_flex_layout(GUICore::IContext* context, const GUICore::ElementHandle& element,
                const GUICore::FlexLayoutDesc& source, GUICore::LayoutAxis axis)
            {
                GUICore::FlexLayoutDesc* desc = copy_frame(context, source);
                desc->axis = axis;
                GUICore::LayoutCallbackConfig callbacks;
                callbacks.algorithm = Name(axis == GUICore::LayoutAxis::x ? "gui.h_layout" : "gui.v_layout");
                callbacks.measure_callback = GUICore::measure_flex;
                callbacks.callback = GUICore::layout_flex;
                callbacks.userdata = desc;
                context->set_layout_callback_config(element, callbacks);
                context->end_element();
            }
        }

        LUNA_GUI_API GUICore::ElementHandle begin_h_layout(GUICore::IContext* context, id_t id, const c8* label,
            const GUICore::LayoutConfig& layout)
        {
            return Internal::begin_layout(context, id, label ? label : "Horizontal Layout", layout);
        }

        LUNA_GUI_API void end_h_layout(GUICore::IContext* context, const GUICore::ElementHandle& element,
            const GUICore::FlexLayoutDesc& desc)
        {
            Internal::set_flex_layout(context, element, desc, GUICore::LayoutAxis::x);
        }

        LUNA_GUI_API GUICore::ElementHandle begin_v_layout(GUICore::IContext* context, id_t id, const c8* label,
            const GUICore::LayoutConfig& layout)
        {
            return Internal::begin_layout(context, id, label ? label : "Vertical Layout", layout);
        }

        LUNA_GUI_API void end_v_layout(GUICore::IContext* context, const GUICore::ElementHandle& element,
            const GUICore::FlexLayoutDesc& desc)
        {
            Internal::set_flex_layout(context, element, desc, GUICore::LayoutAxis::y);
        }

        LUNA_GUI_API GUICore::ElementHandle begin_grid_layout(GUICore::IContext* context, id_t id, const c8* label,
            const GUICore::LayoutConfig& layout)
        {
            return Internal::begin_layout(context, id, label ? label : "Grid Layout", layout);
        }

        LUNA_GUI_API void end_grid_layout(GUICore::IContext* context, const GUICore::ElementHandle& element,
            const GUICore::GridLayoutDesc& source)
        {
            GUICore::GridLayoutDesc* desc = Internal::copy_frame(context, source);
            GUICore::LayoutCallbackConfig callbacks;
            callbacks.algorithm = Name("gui.grid_layout");
            callbacks.measure_callback = GUICore::measure_grid;
            callbacks.callback = GUICore::layout_grid;
            callbacks.userdata = desc;
            context->set_layout_callback_config(element, callbacks);
            context->end_element();
        }

        LUNA_GUI_API GUICore::ElementHandle begin_canvas_layout(GUICore::IContext* context, id_t id, const c8* label,
            const GUICore::LayoutConfig& layout)
        {
            return Internal::begin_layout(context, id, label ? label : "Canvas Layout", layout);
        }

        LUNA_GUI_API void end_canvas_layout(GUICore::IContext* context, const GUICore::ElementHandle& element,
            const GUICore::CanvasLayoutDesc& source)
        {
            GUICore::CanvasLayoutDesc* desc = Internal::copy_frame(context, source);
            GUICore::CanvasLayoutItem* items = Internal::allocate_frame_array<GUICore::CanvasLayoutItem>(context,
                source.items.size());
            for(usize i = 0; i < source.items.size(); ++i) items[i] = source.items[i];
            desc->items = Span<const GUICore::CanvasLayoutItem>(items, source.items.size());
            GUICore::LayoutCallbackConfig callbacks;
            callbacks.algorithm = Name("gui.canvas_layout");
            callbacks.callback = GUICore::layout_canvas;
            callbacks.userdata = desc;
            context->set_layout_callback_config(element, callbacks);
            context->end_element();
        }
    }
}
