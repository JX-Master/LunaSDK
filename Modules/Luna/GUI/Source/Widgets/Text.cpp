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
#include "Text.hpp"
#include "../Context.hpp"
#include "../../Widgets.hpp"
#include "../../Widgets/Text.hpp"

namespace Luna
{
    namespace GUI
    {
        RV TextBuildData::build(IContext* ctx)
        {
            // Calculate bounding rect.
            if(parent)
            {
                Float4U anthor = widget->get_vattr(VATTR_ANTHOR, false, {0, 0, 1, 1});
                Float4U offset = widget->get_vattr(VATTR_OFFSET, false, {0, 0, 0, 0});
                bounding_rect = calc_widget_bounding_rect(parent->bounding_rect, 
                    OffsetRectF{anthor.x, anthor.y, anthor.z, anthor.w}, 
                    OffsetRectF{offset.x, offset.y, offset.z, offset.w});
            }
            // Build font.
            VG::TextArrangeSection section;
            section.font_file = Font::get_default_font();
            section.font_index = 0;
            section.font_size = widget->get_sattr(SATTR_TEXT_SIZE, true, 18.0f);
            section.color = Color::to_rgba8(widget->get_vattr(VATTR_TEXT_COLOR, true, Float4U(1.0f)));
            section.char_span = 0;
            section.line_span = 0;
            Name text = widget->get_tattr(TATTR_TEXT, false);
            section.num_chars = text.size();
            text_arrange_sections.clear();
            text_arrange_sections.push_back(section);
            auto& io = ctx->get_io();
            RectF rect(bounding_rect.left, io.height - bounding_rect.bottom, bounding_rect.right - bounding_rect.left, bounding_rect.bottom - bounding_rect.top);
            arrange_result = VG::arrange_text(text.c_str(), text.size(), 
                text_arrange_sections.cspan(), rect, VG::TextAlignment::center, VG::TextAlignment::begin);
            dirty = false;
            return ok;
        }
        RV TextBuildData::render(IContext *ctx, VG::IShapeDrawList *draw_list)
        {
            return VG::commit_text_arrange_result(arrange_result, text_arrange_sections.cspan(), ctx->get_font_altas(), draw_list);
        }
        LUNA_GUI_API Ref<WidgetBuildData> Text::new_build_data()
        {
            return new_object<TextBuildData>();
        }
    }
}