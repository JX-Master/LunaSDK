/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file LayoutSection.cpp
* @author JXMaster
* @date 2026/7/1
*/
#include "GUICoreTest.hpp"

namespace Luna::GUICoreTest
{
    namespace
    {
        constexpr GUICore::id_t ID_ROW_FIXED = 1000;
        constexpr GUICore::id_t ID_ROW_PERCENT = 1001;
        constexpr GUICore::id_t ID_ROW_FIT = 1002;
        constexpr GUICore::id_t ID_COLUMN_FIXED = 1010;
        constexpr GUICore::id_t ID_COLUMN_PERCENT = 1011;
        constexpr GUICore::id_t ID_COLUMN_FIT = 1012;

        GUICore::LayoutConfig flex_container_layout(f32 width, f32 height, GUICore::FlexLayoutDesc* desc)
        {
            GUICore::LayoutConfig config = fixed_layout(width, height);
            config.name = Name("guicore.test.flex");
            config.callback = GUICore::layout_flex;
            config.userdata = desc;
            return config;
        }

        GUICore::LayoutConfig fixed_child_layout(f32 width, f32 height)
        {
            return fixed_layout(width, height);
        }

        GUICore::LayoutConfig percent_child_layout(f32 width_percent, f32 height)
        {
            GUICore::LayoutConfig config;
            config.width.kind = GUICore::SizeKind::percent;
            config.width.value = width_percent;
            config.height.kind = GUICore::SizeKind::fixed;
            config.height.value = height;
            config.flex_shrink = 1.0f;
            return config;
        }

        GUICore::LayoutConfig fit_child_layout()
        {
            GUICore::LayoutConfig config;
            config.width.kind = GUICore::SizeKind::fit;
            config.height.kind = GUICore::SizeKind::fit;
            config.padding = Float4U(20.0f, 12.0f, 20.0f, 12.0f);
            return config;
        }

        void flex_child(GUICore::IContext* context, GUICore::id_t id, const c8* text,
            const GUICore::LayoutConfig& layout, f32 font_size = 20.0f)
        {
            GUICore::ElementHandle child = context->begin_element(id, Name(text));
            context->set_layout_config(child, layout);
            draw_rect(context, RectF(0.0f, 0.0f, 0.0f, 0.0f), Float4U(0.94f, 0.94f, 0.94f, 1.0f), 0.0f);
            draw_outline(context, RectF(0.0f, 0.0f, 0.0f, 0.0f), Float4U(0.0f, 0.0f, 0.0f, 1.0f), 1.25f);
            draw_text(context, RectF(12.0f, 12.0f, 220.0f, 40.0f), text, font_size, Float4U(0.0f, 0.0f, 0.0f, 1.0f),
                VG::TextAlignment::center);
            context->end_element();
        }

        void concept_box(GUICore::IContext* context, GUICore::id_t id, const c8* title, const c8* detail)
        {
            GUICore::ElementHandle box = context->begin_element(id, Name(title));
            context->set_layout_config(box, fixed_layout(348.0f, 124.0f));
            draw_rect(context, RectF(0.0f, 0.0f, 348.0f, 124.0f), Float4U(1.0f, 1.0f, 1.0f, 1.0f), 0.0f);
            draw_outline(context, RectF(0.0f, 0.0f, 348.0f, 124.0f), Float4U(0.0f, 0.0f, 0.0f, 1.0f), 1.25f);
            draw_text(context, RectF(18.0f, 16.0f, 300.0f, 30.0f), title, 24.0f, Float4U(0.0f, 0.0f, 0.0f, 1.0f));
            draw_text(context, RectF(18.0f, 58.0f, 300.0f, 42.0f), detail, 17.0f, Float4U(0.20f, 0.20f, 0.20f, 1.0f));
            context->end_element();
        }
    }

    void build_layout_slice(GUICore::IContext* context)
    {
        GUICore::ElementHandle body = context->begin_element(ID_LAYOUT, Name("Layout Slice Body"));
        context->set_layout_config(body, fixed_layout(SHEET_WIDTH - 128.0f, 108.0f));
        draw_text(context, RectF(0.0f, 0.0f, 1180.0f, 38.0f),
            "The parent owns the layout algorithm. Children only provide size constraints and content.",
            25.0f, Float4U(0.0f, 0.0f, 0.0f, 1.0f));
        draw_text(context, RectF(0.0f, 48.0f, 1120.0f, 30.0f),
            "Flex containers perform local measure + arrange; other elements can stay leaf-like data records.",
            20.0f, Float4U(0.22f, 0.22f, 0.22f, 1.0f));
        context->end_element();

        static GUICore::FlexLayoutDesc row_desc;
        row_desc.axis = GUICore::LayoutAxis::x;
        row_desc.main_axis_gap = 18.0f;
        row_desc.cross_alignment = GUICore::FlexAlignment::stretch;
        row_desc.main_alignment = GUICore::FlexAlignment::start;
        GUICore::ElementHandle row = context->begin_element(ID_LAYOUT_FLEX_ROW, Name("Horizontal Flex"));
        context->set_layout_config(row, flex_container_layout(760.0f, 122.0f, &row_desc));
        draw_text(context, RectF(0.0f, -34.0f, 420.0f, 28.0f), "horizontal flex", 20.0f,
            Float4U(0.0f, 0.0f, 0.0f, 1.0f));
        draw_outline(context, RectF(0.0f, 0.0f, 760.0f, 122.0f), Float4U(0.0f, 0.0f, 0.0f, 1.0f), 1.25f);
        flex_child(context, ID_ROW_FIXED, "fixed 220", fixed_child_layout(220.0f, 76.0f));
        flex_child(context, ID_ROW_PERCENT, "35 percent", percent_child_layout(0.35f, 76.0f));
        flex_child(context, ID_ROW_FIT, "fit text", fit_child_layout());
        context->end_element();

        static GUICore::FlexLayoutDesc column_desc;
        column_desc.axis = GUICore::LayoutAxis::y;
        column_desc.main_axis_gap = 14.0f;
        column_desc.cross_alignment = GUICore::FlexAlignment::stretch;
        column_desc.main_alignment = GUICore::FlexAlignment::start;
        GUICore::ElementHandle column = context->begin_element(ID_LAYOUT_FLEX_COLUMN, Name("Vertical Flex"));
        context->set_layout_config(column, flex_container_layout(420.0f, 180.0f, &column_desc));
        draw_text(context, RectF(0.0f, -34.0f, 320.0f, 28.0f), "vertical flex", 20.0f,
            Float4U(0.0f, 0.0f, 0.0f, 1.0f));
        draw_outline(context, RectF(0.0f, 0.0f, 420.0f, 180.0f), Float4U(0.0f, 0.0f, 0.0f, 1.0f), 1.25f);
        flex_child(context, ID_COLUMN_FIXED, "fixed height", fixed_child_layout(220.0f, 44.0f), 17.0f);
        flex_child(context, ID_COLUMN_PERCENT, "percent width", percent_child_layout(0.60f, 44.0f), 17.0f);
        flex_child(context, ID_COLUMN_FIT, "fit text", fit_child_layout(), 18.0f);
        context->end_element();

        concept_box(context, ID_LAYOUT_STACK, "Stack", "Children share one rectangle and align inside it.");
        concept_box(context, ID_LAYOUT_CANVAS, "Canvas", "Children use anchors, offsets and pivots.");
        concept_box(context, ID_LAYOUT_SCROLL, "ScrollViewport", "Content is arranged, then clipped.");
    }
}
