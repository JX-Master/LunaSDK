/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file WidgetDraw.cpp
* @author JXMaster
* @date 2024/6/5
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "../WidgetDraw.hpp"
#include <Luna/VG/Shapes.hpp>
#include <Luna/Runtime/Math/Color.hpp>

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API void draw_rectangle_filled(IContext* ctx, VG::IShapeDrawList* draw_list, f32 min_x, f32 min_y, f32 max_x, f32 max_y, const Float4& color)
        {
            auto& points = draw_list->get_shape_buffer()->get_shape_points();
            u32 begin_command = (u32)points.size();
            auto& io = ctx->get_io();
            f32 screen_min_y = io.height - max_y;
            f32 screen_max_y = io.height - min_y;
            VG::ShapeBuilder::add_rectangle_filled(points, min_x, screen_min_y, max_x, screen_max_y);
            u32 num_commands = (u32)points.size() - begin_command;
            draw_list->draw_shape(begin_command, num_commands, 
                {min_x, screen_min_y}, {max_x, screen_max_y}, 
                {min_x, screen_min_y}, {max_x, screen_max_y},
                color);
        }
        LUNA_GUI_API void draw_text(IContext* ctx, VG::IShapeDrawList* draw_list, 
            const c8* text, usize text_len, 
            const Float4U& text_color,
            f32 text_size, 
            f32 min_x, f32 min_y, f32 max_x, f32 max_y, 
            Font::IFontFile* font_file,
            u32 font_index,
            f32 char_span,
            f32 line_span,
            VG::TextAlignment vertical_alignment, 
            VG::TextAlignment horizontal_alignment)
        {
            if(text_len == USIZE_MAX) text_len = strlen(text);
            if(!font_file) font_file = Font::get_default_font();
            VG::TextArrangeSection section;
            section.font_file = font_file;
            section.font_index = font_index;
            section.font_size = text_size;
            section.color = text_color;
            section.char_span = char_span;
            section.line_span = line_span;
            section.num_chars = text_len;
            auto& io = ctx->get_io();
            RectF rect{min_x, io.height - max_y, max_x - min_x, max_y - min_y};
            VG::TextArrangeResult result = VG::arrange_text(text, text_len, {&section, 1}, rect, vertical_alignment, horizontal_alignment);
            VG::commit_text_arrange_result(result, {&section, 1}, ctx->get_font_altas(), draw_list);
        }
    }
}