/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Text.cpp
* @author JXMaster
* @date 2024/5/8
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "../Context.hpp"
#include "../../Widgets.hpp"
#include "../../Widgets/Text.hpp"
#include "../../WidgetDraw.hpp"

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API f32 Text::get_desired_size_x(DesiredSizeType type, const f32* suggested_size_y)
        {
            if(type == DesiredSizeType::required || type == DesiredSizeType::filling) return 0;
            Font::IFontFile* font = cast_object<Font::IFontFile>(get_oattr(this, OATTR_FONT, true, Font::get_default_font()));
            u32 font_index = get_sattr(this, SATTR_FONT_INDEX, true, 0);
            f32 text_size = get_sattr(this, SATTR_TEXT_SIZE, true, DEFAULT_TEXT_SIZE);
            VG::TextArrangeSection section;
            section.font_file = font;
            section.font_index = font_index;
            section.font_size = text_size;
            section.num_chars = text.size();
            RectF rect{0, 0, F32_MAX, F32_MAX};
            VG::TextArrangeResult result = VG::arrange_text(text.c_str(), text.size(), {&section, 1}, rect, 
                VG::TextAlignment::begin, VG::TextAlignment::begin);
            return result.bounding_rect.width;
        }
        LUNA_GUI_API f32 Text::get_desired_size_y(DesiredSizeType type, const f32* suggested_size_x)
        {
            if(type == DesiredSizeType::required || type == DesiredSizeType::filling) return 0;
            Font::IFontFile* font = cast_object<Font::IFontFile>(get_oattr(this, OATTR_FONT, true, Font::get_default_font()));
            u32 font_index = get_sattr(this, SATTR_FONT_INDEX, true, 0);
            f32 text_size = get_sattr(this, SATTR_TEXT_SIZE, true, DEFAULT_TEXT_SIZE);
            VG::TextArrangeSection section;
            section.font_file = font;
            section.font_index = font_index;
            section.font_size = text_size;
            section.num_chars = text.size();
            RectF rect{0, 0, suggested_size_x ? *suggested_size_x : F32_MAX, F32_MAX};
            VG::TextArrangeResult result = VG::arrange_text(text.c_str(), text.size(), {&section, 1}, rect, 
                VG::TextAlignment::begin, VG::TextAlignment::begin);
            return result.bounding_rect.height;
        }
        LUNA_GUI_API RV Text::draw(IContext *ctx, IDrawList* draw_list)
        {
            Font::IFontFile* font = cast_object<Font::IFontFile>(get_oattr(this, OATTR_FONT, true, Font::get_default_font()));
            u32 font_index = get_sattr(this, SATTR_FONT_INDEX, true, 0);
            draw_text(ctx, draw_list, text.c_str(), text.size(), 
                get_vattr(this, VATTR_TEXT_COLOR, true, Float4U(1.0f)), 
                get_sattr(this, SATTR_TEXT_SIZE, true, DEFAULT_TEXT_SIZE), 
                bounding_rect.left, bounding_rect.top, bounding_rect.right, bounding_rect.bottom,
                font, font_index);
            return ok;
        }
        LUNA_GUI_API Text* text(IWidgetBuilder* builder, const Name& text)
        {
            Ref<Text> widget = new_object<Text>();
            builder->add_widget(widget);
            widget->text = text;
            return widget;
        }
    }
}