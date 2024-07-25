/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file WidgetDraw.hpp
* @author JXMaster
* @date 2024/6/5
*/
#pragma once
#include "Context.hpp"
#include <Luna/VG/ShapeDrawList.hpp>
#include <Luna/VG/TextArranger.hpp>

#ifndef LUNA_GUI_API
#define LUNA_GUI_API
#endif

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API void draw_rectangle_filled(IContext* ctx, IDrawList* draw_list, 
            f32 min_x, f32 min_y, f32 max_x, f32 max_y, const Float4& color);

        LUNA_GUI_API void draw_rectangle_bordered(IContext* ctx, IDrawList* draw_list, 
            f32 min_x, f32 min_y, f32 max_x, f32 max_y, const Float4& color, f32 border_width);

        LUNA_GUI_API void draw_rounded_rectangle_filled(IContext* ctx, IDrawList* draw_list, 
            f32 min_x, f32 min_y, f32 max_x, f32 max_y, const Float4& color, f32 radius);

        LUNA_GUI_API void draw_rounded_rectangle_bordered(IContext* ctx, IDrawList* draw_list, 
            f32 min_x, f32 min_y, f32 max_x, f32 max_y, const Float4& color, f32 radius, f32 border_width);

        LUNA_GUI_API void draw_text(IContext* ctx, IDrawList* draw_list, 
            const c8* text, usize text_len, 
            const Float4U& text_color,
            f32 text_size, 
            f32 min_x, f32 min_y, f32 max_x, f32 max_y, 
            Font::IFontFile* font_file = nullptr,
            u32 font_index = 0,
            f32 char_span = 0,
            f32 line_span = 0,
            VG::TextAlignment vertical_alignment = VG::TextAlignment::begin, 
            VG::TextAlignment horizontal_alignment = VG::TextAlignment::begin);
    }
}