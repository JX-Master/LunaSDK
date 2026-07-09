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
#include <cstdio>
#include <cstring>

namespace Luna::GUICoreTest
{
    namespace
    {
        constexpr f32 BODY_X = 64.0f;
        constexpr f32 BODY_Y = 174.0f;
        constexpr f32 CASE_TOP = 260.0f;
        constexpr f32 CASE_LEFT = 82.0f;
        constexpr f32 CASE_GAP_X = 418.0f;
        constexpr f32 CASE_GAP_Y = 176.0f;

        enum LayoutSlice : u32
        {
            layout_overview,
            flex_axis,
            flex_reverse,
            flex_wrap,
            flex_main_alignment,
            flex_cross_alignment,
            flex_line_alignment,
            flex_gap,
            flex_item_sizing,
            flex_clipping,
            grid_layout,
            canvas_layout,
            scroll_viewport_layout,
            table_layout
        };

        struct DemoChild
        {
            const c8* label;
            GUICore::SizeKind main_kind = GUICore::SizeKind::fixed;
            f32 main_value = 70.0f;
            f32 cross_size = 42.0f;
            f32 min_main = 0.0f;
            f32 max_main = -1.0f;
            f32 grow = 0.0f;
            f32 shrink = 1.0f;
            Float4U color = Float4U(0.93f, 0.93f, 0.93f, 1.0f);
        };

        GUICore::id_t demo_id(u32 index)
        {
            return ID_LAYOUT_DEMO_BASE + index;
        }

        GUICore::id_t child_id(GUICore::id_t parent, u32 index)
        {
            return parent * 100 + index + 1;
        }

        GUICore::LayoutConfig flex_container_layout(f32 width, f32 height, GUICore::FlexLayoutDesc* desc)
        {
            GUICore::LayoutConfig config = fixed_layout(width, height);
            config.name = Name("guicore.test.flex");
            config.padding = Float4U(10.0f);
            config.measure_callback = GUICore::measure_flex;
            config.callback = GUICore::layout_flex;
            config.userdata = desc;
            return config;
        }

        GUICore::MeasureResult measure_label(
            GUICore::IContext*, const GUICore::ElementHandle&, const Float2U&, void* userdata)
        {
            const c8* label = (const c8*)userdata;
            f32 width = label ? (f32)strlen(label) * 8.5f + 24.0f : 24.0f;
            GUICore::MeasureResult result;
            result.minimum = Float2U(min(width, 72.0f), 28.0f);
            result.desired = Float2U(width, 34.0f);
            result.maximum = Float2U(F32_MAX, F32_MAX);
            return result;
        }

        GUICore::LayoutConfig flex_child_layout(GUICore::LayoutAxis axis, const DemoChild& child)
        {
            GUICore::LayoutConfig config;
            GUICore::SizeValue& main = axis == GUICore::LayoutAxis::x ? config.width : config.height;
            GUICore::SizeValue& cross = axis == GUICore::LayoutAxis::x ? config.height : config.width;
            main.kind = child.main_kind;
            main.value = child.main_value;
            main.min = child.min_main;
            main.max = child.max_main;
            cross.kind = GUICore::SizeKind::fixed;
            cross.value = child.cross_size;
            config.padding = Float4U(6.0f, 4.0f, 6.0f, 4.0f);
            config.flex_grow = child.grow;
            config.flex_shrink = child.shrink;
            if(child.main_kind == GUICore::SizeKind::fit)
            {
                config.measure_callback = measure_label;
                config.userdata = (void*)child.label;
            }
            return config;
        }

        void draw_element_text(GUICore::IContext* context, const c8* text, f32 size,
            const Float4U& color = Float4U(0.0f, 0.0f, 0.0f, 1.0f))
        {
            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::text;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = RectF(4.0f, 2.0f, -8.0f, -4.0f);
            command.rect_layout_scale = Float4U(0.0f, 0.0f, 1.0f, 1.0f);
            command.color = color;
            command.font = Name("default");
            command.font_size = size;
            command.horizontal_alignment = VG::TextAlignment::center;
            command.vertical_alignment = VG::TextAlignment::center;
            command.text = text ? text : "";
            context->draw(command);
        }

        void text_block(GUICore::IContext* context, const c8* text)
        {
            GUICore::ElementHandle body = context->begin_element(ID_LAYOUT, Name("Layout Slide Body"));
            context->set_layout_config(body, fixed_layout(SHEET_WIDTH - 128.0f, 66.0f));
            draw_text(context, RectF(0.0f, 0.0f, 1180.0f, 34.0f), text, 23.0f,
                Float4U(0.10f, 0.10f, 0.10f, 1.0f));
            context->end_element();
        }

        void demo_child(GUICore::IContext* context, GUICore::id_t id, const DemoChild& desc, GUICore::LayoutAxis axis)
        {
            GUICore::ElementHandle child = context->begin_element(id, Name(desc.label));
            context->set_layout_config(child, flex_child_layout(axis, desc));
            draw_rect(context, RectF(0.0f, 0.0f, 0.0f, 0.0f), desc.color, 0.0f);
            draw_element_text(context, desc.label, 15.0f);
            context->end_element();
        }

        void flex_case(GUICore::IContext* context, GUICore::id_t id, const c8* title, f32 width, f32 height,
            GUICore::FlexLayoutDesc* desc, Span<const DemoChild> children)
        {
            GUICore::ElementHandle container = context->begin_element(id, Name(title));
            context->set_layout_config(container, flex_container_layout(width, height, desc));
            draw_text(context, RectF(0.0f, -32.0f, width, 26.0f), title, 20.0f,
                Float4U(0.0f, 0.0f, 0.0f, 1.0f));
            draw_rect(context, RectF(0.0f, 0.0f, 0.0f, 0.0f), Float4U(0.97f, 0.97f, 0.97f, 1.0f), 0.0f);
            draw_outline(context, RectF(0.0f, 0.0f, width, height), Float4U(0.0f, 0.0f, 0.0f, 1.0f), 1.0f);
            for(u32 i = 0; i < children.size(); ++i)
            {
                demo_child(context, child_id(id, i), children[i], desc->axis);
            }
            context->end_element();
        }

        void plain_box(GUICore::IContext* context, GUICore::id_t id, const c8* label, f32 width, f32 height,
            const Float4U& color = Float4U(0.93f, 0.93f, 0.93f, 1.0f))
        {
            GUICore::ElementHandle child = context->begin_element(id, Name(label));
            context->set_layout_config(child, fixed_layout(width, height));
            draw_rect(context, RectF(0.0f, 0.0f, 0.0f, 0.0f), color, 0.0f);
            draw_element_text(context, label, 17.0f);
            context->end_element();
        }

        void concept_case(GUICore::IContext* context, GUICore::id_t id, const c8* title, const c8* detail)
        {
            GUICore::ElementHandle box = context->begin_element(id, Name(title));
            context->set_layout_config(box, fixed_layout(360.0f, 126.0f));
            draw_rect(context, RectF(0.0f, 0.0f, 0.0f, 0.0f), Float4U(0.97f, 0.97f, 0.97f, 1.0f), 0.0f);
            draw_outline(context, RectF(0.0f, 0.0f, 360.0f, 126.0f), Float4U(0.0f, 0.0f, 0.0f, 1.0f), 1.0f);
            draw_text(context, RectF(18.0f, 16.0f, 320.0f, 30.0f), title, 24.0f,
                Float4U(0.0f, 0.0f, 0.0f, 1.0f));
            draw_text(context, RectF(18.0f, 58.0f, 320.0f, 46.0f), detail, 17.0f,
                Float4U(0.22f, 0.22f, 0.22f, 1.0f));
            context->end_element();
        }

        void add_case_grid(CoreSheetState& state, u32 count)
        {
            for(u32 i = 0; i < count; ++i)
            {
                f32 x = CASE_LEFT + (f32)(i % 3) * CASE_GAP_X;
                f32 y = CASE_TOP + (f32)(i / 3) * CASE_GAP_Y;
                add_canvas_item(state.sheet_items, demo_id(i), x, y);
            }
        }

        void add_case_rows(CoreSheetState& state, u32 count)
        {
            for(u32 i = 0; i < count; ++i)
            {
                add_canvas_item(state.sheet_items, demo_id(i), 92.0f, 244.0f + (f32)i * 74.0f);
            }
        }

        void build_overview(GUICore::IContext* context)
        {
            text_block(context, "Each layout algorithm gets its own slice. Flex gets extra slices for every core parameter group.");
            concept_case(context, demo_id(0), "Flex", "Ordered row or column layout with grow, shrink, wrap and alignment.");
            concept_case(context, demo_id(1), "Grid", "Row-major tiled cells for browsers and icon palettes.");
            concept_case(context, demo_id(2), "Canvas", "Anchor, offset and pivot based placement.");
            concept_case(context, demo_id(3), "ScrollViewport", "Translate and clip scroll content.");
            concept_case(context, demo_id(4), "Table", "Explicit tracks and cell attachments.");
        }

        void build_flex_axis(GUICore::IContext* context)
        {
            text_block(context, "`axis` selects the main direction. The same children are arranged as a row or as a column.");
            static GUICore::FlexLayoutDesc row;
            row = GUICore::FlexLayoutDesc();
            row.axis = GUICore::LayoutAxis::x;
            row.main_axis_gap = 12.0f;
            row.cross_alignment = GUICore::FlexAlignment::center;
            static GUICore::FlexLayoutDesc column;
            column = row;
            column.axis = GUICore::LayoutAxis::y;
            DemoChild children[] = {
                { "A", GUICore::SizeKind::fixed, 86.0f, 48.0f },
                { "B", GUICore::SizeKind::fixed, 118.0f, 48.0f },
                { "C", GUICore::SizeKind::fixed, 72.0f, 48.0f }
            };
            flex_case(context, demo_id(0), "LayoutAxis::x", 360.0f, 120.0f, &row, Span<const DemoChild>(children, 3));
            flex_case(context, demo_id(1), "LayoutAxis::y", 360.0f, 166.0f, &column, Span<const DemoChild>(children, 3));
        }

        void build_flex_reverse(GUICore::IContext* context)
        {
            text_block(context, "`reverse` flips only placement on the main axis. Element order and focus order stay authored order.");
            DemoChild children[] = {
                { "A", GUICore::SizeKind::fixed, 72.0f, 42.0f },
                { "B", GUICore::SizeKind::fixed, 96.0f, 42.0f },
                { "C", GUICore::SizeKind::fixed, 72.0f, 42.0f }
            };
            static GUICore::FlexLayoutDesc descs[4];
            for(u32 i = 0; i < 4; ++i)
            {
                descs[i] = GUICore::FlexLayoutDesc();
                descs[i].axis = i < 2 ? GUICore::LayoutAxis::x : GUICore::LayoutAxis::y;
                descs[i].reverse = (i % 2) != 0;
                descs[i].main_axis_gap = 10.0f;
                descs[i].cross_alignment = GUICore::FlexAlignment::center;
            }
            flex_case(context, demo_id(0), "row normal", 340.0f, 94.0f, &descs[0], Span<const DemoChild>(children, 3));
            flex_case(context, demo_id(1), "row reverse", 340.0f, 94.0f, &descs[1], Span<const DemoChild>(children, 3));
            flex_case(context, demo_id(2), "column normal", 220.0f, 160.0f, &descs[2], Span<const DemoChild>(children, 3));
            flex_case(context, demo_id(3), "column reverse", 220.0f, 160.0f, &descs[3], Span<const DemoChild>(children, 3));
        }

        void build_flex_wrap(GUICore::IContext* context)
        {
            text_block(context, "`wrap` decides whether items stay on one line or create additional cross-axis lines.");
            DemoChild children[] = {
                { "A", GUICore::SizeKind::fixed, 124.0f, 40.0f },
                { "B", GUICore::SizeKind::fixed, 104.0f, 40.0f },
                { "C", GUICore::SizeKind::fixed, 116.0f, 40.0f },
                { "D", GUICore::SizeKind::fixed, 96.0f, 40.0f },
                { "E", GUICore::SizeKind::fixed, 112.0f, 40.0f }
            };
            static GUICore::FlexLayoutDesc descs[3];
            for(u32 i = 0; i < 3; ++i)
            {
                descs[i] = GUICore::FlexLayoutDesc();
                descs[i].axis = GUICore::LayoutAxis::x;
                descs[i].wrap = i == 0 ? GUICore::FlexWrap::none : (i == 1 ? GUICore::FlexWrap::wrap : GUICore::FlexWrap::wrap_reverse);
                descs[i].main_axis_gap = 10.0f;
                descs[i].cross_axis_gap = 8.0f;
            }
            flex_case(context, demo_id(0), "FlexWrap::none", 360.0f, 150.0f, &descs[0], Span<const DemoChild>(children, 5));
            flex_case(context, demo_id(1), "FlexWrap::wrap", 360.0f, 160.0f, &descs[1], Span<const DemoChild>(children, 5));
            flex_case(context, demo_id(2), "FlexWrap::wrap_reverse", 360.0f, 160.0f, &descs[2], Span<const DemoChild>(children, 5));
        }

        void build_flex_main_alignment(GUICore::IContext* context)
        {
            text_block(context, "`main_alignment` distributes free space on one flex line. Use flex_grow, not stretch, to resize items.");
            const GUICore::FlexAlignment alignments[] = {
                GUICore::FlexAlignment::start,
                GUICore::FlexAlignment::center,
                GUICore::FlexAlignment::end,
                GUICore::FlexAlignment::space_between,
                GUICore::FlexAlignment::space_around,
                GUICore::FlexAlignment::space_evenly
            };
            const c8* titles[] = { "start", "center", "end", "space_between", "space_around", "space_evenly" };
            DemoChild children[] = {
                { "A", GUICore::SizeKind::fixed, 58.0f, 38.0f },
                { "B", GUICore::SizeKind::fixed, 82.0f, 38.0f },
                { "C", GUICore::SizeKind::fixed, 58.0f, 38.0f }
            };
            static GUICore::FlexLayoutDesc descs[6];
            for(u32 i = 0; i < 6; ++i)
            {
                descs[i] = GUICore::FlexLayoutDesc();
                descs[i].axis = GUICore::LayoutAxis::x;
                descs[i].main_alignment = alignments[i];
                descs[i].cross_alignment = GUICore::FlexAlignment::center;
                descs[i].main_axis_gap = 8.0f;
                flex_case(context, demo_id(i), titles[i], 340.0f, 86.0f, &descs[i], Span<const DemoChild>(children, 3));
            }
        }

        void build_flex_cross_alignment(GUICore::IContext* context)
        {
            text_block(context, "`cross_alignment` aligns items inside one line on the cross axis.");
            const GUICore::FlexAlignment alignments[] = {
                GUICore::FlexAlignment::start,
                GUICore::FlexAlignment::center,
                GUICore::FlexAlignment::end,
                GUICore::FlexAlignment::stretch
            };
            const c8* titles[] = { "start", "center", "end", "stretch" };
            DemoChild children[] = {
                { "Tall", GUICore::SizeKind::fixed, 74.0f, 86.0f },
                { "Short", GUICore::SizeKind::fixed, 74.0f, 38.0f },
                { "Mid", GUICore::SizeKind::fixed, 74.0f, 62.0f }
            };
            static GUICore::FlexLayoutDesc descs[4];
            for(u32 i = 0; i < 4; ++i)
            {
                descs[i] = GUICore::FlexLayoutDesc();
                descs[i].axis = GUICore::LayoutAxis::x;
                descs[i].cross_alignment = alignments[i];
                descs[i].main_axis_gap = 12.0f;
                flex_case(context, demo_id(i), titles[i], 340.0f, 116.0f, &descs[i], Span<const DemoChild>(children, 3));
            }
        }

        void build_flex_line_alignment(GUICore::IContext* context)
        {
            text_block(context, "`line_alignment` distributes multiple wrapped lines on the cross axis.");
            const GUICore::FlexAlignment alignments[] = {
                GUICore::FlexAlignment::start,
                GUICore::FlexAlignment::center,
                GUICore::FlexAlignment::end,
                GUICore::FlexAlignment::stretch,
                GUICore::FlexAlignment::space_between,
                GUICore::FlexAlignment::space_around,
                GUICore::FlexAlignment::space_evenly
            };
            const c8* titles[] = { "start", "center", "end", "stretch", "space_between", "space_around", "space_evenly" };
            DemoChild children[] = {
                { "A", GUICore::SizeKind::fixed, 90.0f, 32.0f },
                { "B", GUICore::SizeKind::fixed, 96.0f, 32.0f },
                { "C", GUICore::SizeKind::fixed, 88.0f, 32.0f },
                { "D", GUICore::SizeKind::fixed, 92.0f, 32.0f },
                { "E", GUICore::SizeKind::fixed, 82.0f, 32.0f }
            };
            static GUICore::FlexLayoutDesc descs[7];
            for(u32 i = 0; i < 7; ++i)
            {
                descs[i] = GUICore::FlexLayoutDesc();
                descs[i].axis = GUICore::LayoutAxis::x;
                descs[i].wrap = GUICore::FlexWrap::wrap;
                descs[i].line_alignment = alignments[i];
                descs[i].main_axis_gap = 8.0f;
                descs[i].cross_axis_gap = 4.0f;
                flex_case(context, demo_id(i), titles[i], 310.0f, 128.0f, &descs[i], Span<const DemoChild>(children, 5));
            }
        }

        void build_flex_gap(GUICore::IContext* context)
        {
            text_block(context, "`main_axis_gap` separates items on a line. `cross_axis_gap` separates wrapped lines.");
            DemoChild children[] = {
                { "A", GUICore::SizeKind::fixed, 84.0f, 36.0f },
                { "B", GUICore::SizeKind::fixed, 84.0f, 36.0f },
                { "C", GUICore::SizeKind::fixed, 84.0f, 36.0f },
                { "D", GUICore::SizeKind::fixed, 84.0f, 36.0f },
                { "E", GUICore::SizeKind::fixed, 84.0f, 36.0f },
                { "F", GUICore::SizeKind::fixed, 84.0f, 36.0f }
            };
            static GUICore::FlexLayoutDesc descs[3];
            for(u32 i = 0; i < 3; ++i)
            {
                descs[i] = GUICore::FlexLayoutDesc();
                descs[i].axis = GUICore::LayoutAxis::x;
                descs[i].wrap = GUICore::FlexWrap::wrap;
            }
            descs[0].main_axis_gap = 6.0f;
            descs[1].main_axis_gap = 26.0f;
            descs[2].main_axis_gap = 10.0f;
            descs[2].cross_axis_gap = 28.0f;
            flex_case(context, demo_id(0), "main_axis_gap = 6", 350.0f, 120.0f, &descs[0], Span<const DemoChild>(children, 6));
            flex_case(context, demo_id(1), "main_axis_gap = 26", 350.0f, 120.0f, &descs[1], Span<const DemoChild>(children, 6));
            flex_case(context, demo_id(2), "main = 10, cross = 28", 350.0f, 150.0f, &descs[2], Span<const DemoChild>(children, 6));
        }

        void build_flex_item_sizing(GUICore::IContext* context)
        {
            text_block(context, "Children provide fixed, percent or fit sizes plus min/max, grow and shrink constraints.");
            static GUICore::FlexLayoutDesc descs[6];
            for(u32 i = 0; i < 6; ++i)
            {
                descs[i] = GUICore::FlexLayoutDesc();
                descs[i].axis = GUICore::LayoutAxis::x;
                descs[i].main_axis_gap = 8.0f;
                descs[i].cross_alignment = GUICore::FlexAlignment::center;
            }
            DemoChild fixed_children[] = {
                { "fixed 90", GUICore::SizeKind::fixed, 90.0f, 38.0f },
                { "fixed 130", GUICore::SizeKind::fixed, 130.0f, 38.0f }
            };
            DemoChild percent_children[] = {
                { "25%", GUICore::SizeKind::percent, 0.25f, 38.0f },
                { "40%", GUICore::SizeKind::percent, 0.40f, 38.0f }
            };
            DemoChild fit_children[] = {
                { "fit", GUICore::SizeKind::fit, 0.0f, 38.0f },
                { "fit longer text", GUICore::SizeKind::fit, 0.0f, 38.0f }
            };
            DemoChild grow_children[] = {
                { "grow 1", GUICore::SizeKind::fixed, 48.0f, 38.0f, 0.0f, -1.0f, 1.0f },
                { "grow 2", GUICore::SizeKind::fixed, 48.0f, 38.0f, 0.0f, -1.0f, 2.0f },
                { "grow 1", GUICore::SizeKind::fixed, 48.0f, 38.0f, 0.0f, -1.0f, 1.0f }
            };
            DemoChild shrink_children[] = {
                { "no shrink", GUICore::SizeKind::fixed, 150.0f, 38.0f, 0.0f, -1.0f, 0.0f, 0.0f },
                { "shrink", GUICore::SizeKind::fixed, 150.0f, 38.0f, 0.0f, -1.0f, 0.0f, 1.0f },
                { "shrink", GUICore::SizeKind::fixed, 150.0f, 38.0f, 0.0f, -1.0f, 0.0f, 1.0f }
            };
            DemoChild clamp_children[] = {
                { "min 96", GUICore::SizeKind::fixed, 42.0f, 38.0f, 96.0f },
                { "max 100 grow", GUICore::SizeKind::fixed, 62.0f, 38.0f, 0.0f, 100.0f, 1.0f },
                { "grow", GUICore::SizeKind::fixed, 62.0f, 38.0f, 0.0f, -1.0f, 1.0f }
            };
            flex_case(context, demo_id(0), "fixed", 350.0f, 86.0f, &descs[0], Span<const DemoChild>(fixed_children, 2));
            flex_case(context, demo_id(1), "percent", 350.0f, 86.0f, &descs[1], Span<const DemoChild>(percent_children, 2));
            flex_case(context, demo_id(2), "fit by measure callback", 350.0f, 86.0f, &descs[2], Span<const DemoChild>(fit_children, 2));
            flex_case(context, demo_id(3), "flex_grow", 350.0f, 86.0f, &descs[3], Span<const DemoChild>(grow_children, 3));
            flex_case(context, demo_id(4), "flex_shrink", 350.0f, 86.0f, &descs[4], Span<const DemoChild>(shrink_children, 3));
            flex_case(context, demo_id(5), "min / max", 350.0f, 86.0f, &descs[5], Span<const DemoChild>(clamp_children, 3));
        }

        void build_flex_clipping(GUICore::IContext* context)
        {
            text_block(context, "`clip_children` intersects child clip rects with the parent content rect. It affects hit testing and explicit clipping.");
            DemoChild children[] = {
                { "A", GUICore::SizeKind::fixed, 120.0f, 42.0f, 0.0f, -1.0f, 0.0f, 0.0f },
                { "B", GUICore::SizeKind::fixed, 120.0f, 42.0f, 0.0f, -1.0f, 0.0f, 0.0f },
                { "C overflow", GUICore::SizeKind::fixed, 150.0f, 42.0f, 0.0f, -1.0f, 0.0f, 0.0f }
            };
            static GUICore::FlexLayoutDesc descs[2];
            for(u32 i = 0; i < 2; ++i)
            {
                descs[i] = GUICore::FlexLayoutDesc();
                descs[i].axis = GUICore::LayoutAxis::x;
                descs[i].main_axis_gap = 10.0f;
                descs[i].clip_children = i == 0;
            }
            flex_case(context, demo_id(0), "clip_children = true", 330.0f, 86.0f, &descs[0], Span<const DemoChild>(children, 3));
            flex_case(context, demo_id(1), "clip_children = false", 330.0f, 86.0f, &descs[1], Span<const DemoChild>(children, 3));
        }

        void build_grid_layout(GUICore::IContext* context)
        {
            text_block(context, "Grid is row-major. It can use fixed cell size or derive cell width from a fixed column count.");
            static GUICore::GridLayoutDesc fixed_cell;
            fixed_cell = GUICore::GridLayoutDesc();
            fixed_cell.mode = GUICore::GridLayoutMode::fixed_cell_size;
            fixed_cell.cell_size = Float2U(92.0f, 54.0f);
            fixed_cell.gap = Float2U(10.0f, 10.0f);
            static GUICore::GridLayoutDesc columns;
            columns = fixed_cell;
            columns.mode = GUICore::GridLayoutMode::fixed_column_count;
            columns.column_count = 4;
            for(u32 c = 0; c < 2; ++c)
            {
                GUICore::ElementHandle grid = context->begin_element(demo_id(c), Name(c ? "Fixed Column Count" : "Fixed Cell Size"));
                context->set_layout_config(grid, fixed_layout(430.0f, 170.0f));
                GUICore::LayoutConfig layout = fixed_layout(430.0f, 170.0f);
                layout.padding = Float4U(10.0f);
                layout.callback = GUICore::layout_grid;
                layout.userdata = c ? &columns : &fixed_cell;
                context->set_layout_config(grid, layout);
                draw_text(context, RectF(0.0f, -32.0f, 360.0f, 26.0f), c ? "fixed_column_count = 4" : "fixed_cell_size = 92 x 54",
                    20.0f, Float4U(0.0f, 0.0f, 0.0f, 1.0f));
                draw_rect(context, RectF(0.0f, 0.0f, 0.0f, 0.0f), Float4U(0.97f, 0.97f, 0.97f, 1.0f));
                draw_outline(context, RectF(0.0f, 0.0f, 430.0f, 170.0f), Float4U(0.0f, 0.0f, 0.0f, 1.0f));
                for(u32 i = 0; i < 9; ++i)
                {
                    char label[8];
                    snprintf(label, sizeof(label), "%u", i + 1);
                    plain_box(context, child_id(demo_id(c), i), label, 48.0f, 36.0f);
                }
                context->end_element();
            }
        }

        void build_canvas_layout(GUICore::IContext* context)
        {
            text_block(context, "Canvas uses parent-owned anchor, offset and pivot records keyed by child element ID.");
            static GUICore::CanvasLayoutItem items[3];
            static GUICore::CanvasLayoutDesc desc;
            items[0].element_id = child_id(demo_id(0), 0);
            items[0].anchor_min = Float2U(0.0f, 0.0f);
            items[0].anchor_max = Float2U(0.0f, 0.0f);
            items[0].offset = Float4U(24.0f, 28.0f, 0.0f, 0.0f);
            items[0].pivot = Float2U(0.0f);
            items[1].element_id = child_id(demo_id(0), 1);
            items[1].anchor_min = Float2U(0.5f, 0.5f);
            items[1].anchor_max = Float2U(0.5f, 0.5f);
            items[1].offset = Float4U(0.0f);
            items[1].pivot = Float2U(0.5f, 0.5f);
            items[2].element_id = child_id(demo_id(0), 2);
            items[2].anchor_min = Float2U(0.0f, 1.0f);
            items[2].anchor_max = Float2U(1.0f, 1.0f);
            items[2].offset = Float4U(24.0f, -58.0f, 24.0f, 0.0f);
            items[2].pivot = Float2U(0.0f, 1.0f);
            desc.items = Span<const GUICore::CanvasLayoutItem>(items, 3);
            desc.clip_children = true;

            GUICore::ElementHandle canvas = context->begin_element(demo_id(0), Name("Canvas Demo"));
            GUICore::LayoutConfig canvas_layout = fixed_layout(680.0f, 280.0f);
            canvas_layout.padding = Float4U(10.0f);
            context->set_layout_config(canvas, canvas_layout);
            draw_rect(context, RectF(0.0f, 0.0f, 0.0f, 0.0f), Float4U(0.97f, 0.97f, 0.97f, 1.0f));
            draw_outline(context, RectF(0.0f, 0.0f, 680.0f, 280.0f), Float4U(0.0f, 0.0f, 0.0f, 1.0f));
            draw_text(context, RectF(0.0f, -32.0f, 420.0f, 26.0f), "anchors + offsets + pivots",
                20.0f, Float4U(0.0f, 0.0f, 0.0f, 1.0f));
            plain_box(context, child_id(demo_id(0), 0), "top-left offset", 150.0f, 54.0f);
            plain_box(context, child_id(demo_id(0), 1), "center pivot", 150.0f, 54.0f);
            plain_box(context, child_id(demo_id(0), 2), "stretched bottom strip", 120.0f, 42.0f);
            set_canvas_layout(context, canvas, &desc);
            context->end_element();
        }

        void build_scroll_viewport_layout(GUICore::IContext* context)
        {
            text_block(context, "ScrollViewport translates children by scroll_offset and writes a clipped layout result.");
            static GUICore::ScrollViewportLayoutDesc desc;
            desc.scroll_offset = Float2U(0.0f, 72.0f);
            desc.clip_children = true;
            GUICore::LayoutConfig layout = fixed_layout(430.0f, 170.0f);
            layout.padding = Float4U(10.0f);
            layout.callback = GUICore::layout_scroll_viewport;
            layout.userdata = &desc;
            GUICore::ElementHandle viewport = context->begin_element(demo_id(0), Name("Scroll Viewport Demo"));
            context->set_layout_config(viewport, layout);
            draw_text(context, RectF(0.0f, -32.0f, 420.0f, 26.0f), "scroll_offset.y = 72",
                20.0f, Float4U(0.0f, 0.0f, 0.0f, 1.0f));
            draw_rect(context, RectF(0.0f, 0.0f, 0.0f, 0.0f), Float4U(0.97f, 0.97f, 0.97f, 1.0f));
            draw_outline(context, RectF(0.0f, 0.0f, 430.0f, 170.0f), Float4U(0.0f, 0.0f, 0.0f, 1.0f));
            for(u32 i = 0; i < 5; ++i)
            {
                GUICore::LayoutConfig item = fixed_layout(390.0f, 42.0f);
                item.margin = Float4U(20.0f, (f32)i * 50.0f, 0.0f, 0.0f);
                char label[32];
                snprintf(label, sizeof(label), "content row %u", i);
                plain_box(context, child_id(demo_id(0), i), label, 390.0f, 42.0f);
                context->set_layout_config(context->find_element_handle(child_id(demo_id(0), i)), item);
            }
            context->end_element();
        }

        void build_table_layout(GUICore::IContext* context)
        {
            text_block(context, "Table uses explicit tracks and child-to-cell attachments, making virtualization and authored tables predictable.");
            static GUICore::TableTrackDesc columns[3];
            static GUICore::TableTrackDesc rows[3];
            static GUICore::TableLayoutCell cells[9];
            static GUICore::TableLayoutDesc desc;
            columns[0].kind = GUICore::TableTrackSizeKind::pixels;
            columns[0].value = 92.0f;
            columns[1].kind = GUICore::TableTrackSizeKind::ratio;
            columns[1].value = 1.0f;
            columns[2].kind = GUICore::TableTrackSizeKind::pixels;
            columns[2].value = 126.0f;
            for(u32 i = 0; i < 3; ++i)
            {
                rows[i].kind = GUICore::TableTrackSizeKind::pixels;
                rows[i].value = 48.0f;
            }
            for(u32 row = 0; row < 3; ++row)
            {
                for(u32 column = 0; column < 3; ++column)
                {
                    u32 index = row * 3 + column;
                    cells[index].element_id = child_id(demo_id(0), index);
                    cells[index].row = row;
                    cells[index].column = column;
                    cells[index].padding = Float4U(4.0f);
                }
            }
            desc.columns = Span<const GUICore::TableTrackDesc>(columns, 3);
            desc.rows = Span<const GUICore::TableTrackDesc>(rows, 3);
            desc.cells = Span<const GUICore::TableLayoutCell>(cells, 9);
            desc.gap = Float2U(6.0f, 6.0f);
            GUICore::LayoutConfig layout = fixed_layout(560.0f, 170.0f);
            layout.padding = Float4U(10.0f);
            layout.callback = GUICore::layout_table;
            layout.userdata = &desc;
            GUICore::ElementHandle table = context->begin_element(demo_id(0), Name("Table Demo"));
            context->set_layout_config(table, layout);
            draw_text(context, RectF(0.0f, -32.0f, 480.0f, 26.0f), "pixels | ratio | pixels",
                20.0f, Float4U(0.0f, 0.0f, 0.0f, 1.0f));
            draw_rect(context, RectF(0.0f, 0.0f, 0.0f, 0.0f), Float4U(0.97f, 0.97f, 0.97f, 1.0f));
            draw_outline(context, RectF(0.0f, 0.0f, 560.0f, 170.0f), Float4U(0.0f, 0.0f, 0.0f, 1.0f));
            const c8* labels[] = { "Name", "Value", "State", "Width", "ratio", "ok", "Rows", "fixed", "ok" };
            for(u32 i = 0; i < 9; ++i)
            {
                plain_box(context, child_id(demo_id(0), i), labels[i], 60.0f, 34.0f);
            }
            context->end_element();
        }
    }

    const c8* layout_slice_title(u32 layout_slice)
    {
        switch(layout_slice)
        {
        case layout_overview: return "GUICore Layout";
        case flex_axis: return "Flex: Axis";
        case flex_reverse: return "Flex: Reverse";
        case flex_wrap: return "Flex: Wrap";
        case flex_main_alignment: return "Flex: Main Alignment";
        case flex_cross_alignment: return "Flex: Cross Alignment";
        case flex_line_alignment: return "Flex: Line Alignment";
        case flex_gap: return "Flex: Gap";
        case flex_item_sizing: return "Flex: Item Sizing";
        case flex_clipping: return "Flex: Clipping";
        case grid_layout: return "Grid Layout";
        case canvas_layout: return "Canvas Layout";
        case scroll_viewport_layout: return "Scroll Viewport Layout";
        case table_layout: return "Table Layout";
        default: return "GUICore Layout";
        }
    }

    const c8* layout_slice_subtitle(u32 layout_slice)
    {
        switch(layout_slice)
        {
        case layout_overview: return "Every built-in algorithm has a dedicated slide; Flex is split by parameter.";
        case flex_axis: return "The main axis determines whether children form rows or columns.";
        case flex_reverse: return "Reverse changes visual placement without changing authored order.";
        case flex_wrap: return "Wrapping turns overflowing children into additional lines.";
        case flex_main_alignment: return "Main alignment distributes free space inside one flex line.";
        case flex_cross_alignment: return "Cross alignment positions items inside a line.";
        case flex_line_alignment: return "Line alignment distributes wrapped lines on the cross axis.";
        case flex_gap: return "Main and cross gaps reserve spacing between items and lines.";
        case flex_item_sizing: return "Children combine size kinds, constraints, grow and shrink.";
        case flex_clipping: return "Clipping controls child clip rect propagation.";
        case grid_layout: return "Grid places children in row-major tiles.";
        case canvas_layout: return "Canvas places children with anchors, offsets and pivots.";
        case scroll_viewport_layout: return "ScrollViewport translates and clips content.";
        case table_layout: return "Table uses explicit tracks and child-to-cell attachments.";
        default: return "Layout is data on elements, evaluated by context-level algorithms.";
        }
    }

    void add_layout_slice_items(CoreSheetState& state, u32 layout_slice)
    {
        add_canvas_item(state.sheet_items, ID_LAYOUT, BODY_X, BODY_Y);
        switch(layout_slice)
        {
        case layout_overview:
            add_case_grid(state, 5);
            break;
        case flex_main_alignment:
        case flex_item_sizing:
            add_case_grid(state, 6);
            break;
        case flex_axis:
        case flex_clipping:
        case grid_layout:
            add_case_grid(state, 2);
            break;
        case flex_reverse:
        case flex_cross_alignment:
            add_case_grid(state, 4);
            break;
        case flex_wrap:
        case flex_gap:
            add_case_grid(state, 3);
            break;
        case flex_line_alignment:
            add_case_grid(state, 7);
            break;
        case canvas_layout:
        case scroll_viewport_layout:
        case table_layout:
            add_canvas_item(state.sheet_items, demo_id(0), 112.0f, 282.0f);
            break;
        default:
            add_case_rows(state, 1);
            break;
        }
    }

    void build_layout_slice(GUICore::IContext* context, u32 layout_slice)
    {
        switch(layout_slice)
        {
        case layout_overview:
            build_overview(context);
            break;
        case flex_axis:
            build_flex_axis(context);
            break;
        case flex_reverse:
            build_flex_reverse(context);
            break;
        case flex_wrap:
            build_flex_wrap(context);
            break;
        case flex_main_alignment:
            build_flex_main_alignment(context);
            break;
        case flex_cross_alignment:
            build_flex_cross_alignment(context);
            break;
        case flex_line_alignment:
            build_flex_line_alignment(context);
            break;
        case flex_gap:
            build_flex_gap(context);
            break;
        case flex_item_sizing:
            build_flex_item_sizing(context);
            break;
        case flex_clipping:
            build_flex_clipping(context);
            break;
        case grid_layout:
            build_grid_layout(context);
            break;
        case canvas_layout:
            build_canvas_layout(context);
            break;
        case scroll_viewport_layout:
            build_scroll_viewport_layout(context);
            break;
        case table_layout:
            build_table_layout(context);
            break;
        default:
            build_overview(context);
            break;
        }
    }
}
