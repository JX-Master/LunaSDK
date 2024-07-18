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
        LUNA_GUI_API RV Text::update(IContext* ctx, const OffsetRectF& layout_rect)
        {
            // Calculate bounding rect.
            Float4U anthor = get_vattr(this, VATTR_ANTHOR, false, {0, 0, 1, 1});
            Float4U offset = get_vattr(this, VATTR_OFFSET, false, {0, 0, 0, 0});
            bounding_rect = calc_widget_bounding_rect(layout_rect, 
                    OffsetRectF{anthor.x, anthor.y, anthor.z, anthor.w}, 
                    OffsetRectF{offset.x, offset.y, offset.z, offset.w});
            return ok;
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