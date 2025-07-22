/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Layout.hpp
* @author JXMaster
* @date 2024/7/18
*/
#pragma once

#include "Widgets/Canvas.hpp"
#include "Widgets/HorizontalLayout.hpp"
#include "Widgets/VerticalLayout.hpp"
#include "Widgets/Spacer.hpp"
#include "Widgets/Dockspace.hpp"

#ifndef LUNA_GUI_API
#define LUNA_GUI_API
#endif

namespace Luna
{
    namespace GUI
    {
        struct IWidgetBuilder;
        LUNA_GUI_API Canvas* begin_canvas(IWidgetBuilder* builder, const Name& id = Name());
        LUNA_GUI_API void end_canvas(IWidgetBuilder* builder);
        LUNA_GUI_API void set_canvas_anthor(IWidgetBuilder* builder, f32 left, f32 top, f32 right, f32 bottom);
        LUNA_GUI_API void set_canvas_offset(IWidgetBuilder* builder, f32 left, f32 top, f32 right, f32 bottom);

        LUNA_GUI_API HorizontalLayout* begin_hlayout(IWidgetBuilder* builder, const Name& id = Name());
        LUNA_GUI_API void end_hlayout(IWidgetBuilder* builder);

        LUNA_GUI_API VerticalLayout* begin_vlayout(IWidgetBuilder* builder, const Name& id = Name());
        LUNA_GUI_API void end_vlayout(IWidgetBuilder* builder);

        LUNA_GUI_API Spacer* spacer(IWidgetBuilder* builder);

        LUNA_GUI_API Dockspace* begin_dockspace(IWidgetBuilder* builder, const Name& id = Name());
        LUNA_GUI_API void end_dockspace(IWidgetBuilder* builder);
    }
}