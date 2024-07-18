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

#include <Widgets/Canvas.hpp>

#ifndef LUNA_GUI_API
#define LUNA_GUI_API
#endif

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API Canvas* begin_canvas(IContext* ctx);
        LUNA_GUI_API void end_canvas(IContext* ctx);
        LUNA_GUI_API void set_anthor(IContext* ctx, f32 left, f32 top, f32 right, f32 bottom);
        LUNA_GUI_API void set_offset(IContext* ctx, f32 left, f32 top, f32 right, f32 bottom);


    }
}