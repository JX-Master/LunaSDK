/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file LayoutSection.cpp
* @author JXMaster
* @date 2026/7/1
*/
#include "GUITest.hpp"
#include <cstdio>
#include <cstring>

namespace Luna::GUITest
{
    namespace
    {
        constexpr f32 BODY_X = 64.0f;
        constexpr f32 BODY_Y = 174.0f;
        constexpr f32 CASE_TOP = 260.0f;
        constexpr f32 CASE_LEFT = 82.0f;
        constexpr f32 CASE_GAP_X = 418.0f;
        constexpr f32 CASE_GAP_Y = 176.0f;
        constexpr f32 SCROLL_VIEWPORT_WIDTH = 920.0f;
        constexpr f32 SCROLL_VIEWPORT_HEIGHT = 340.0f;
        constexpr f32 SCROLL_VIEWPORT_PADDING = 12.0f;
        constexpr f32 SCROLL_ROW_WIDTH = 860.0f;
        constexpr f32 SCROLL_ROW_HEIGHT = 46.0f;
        constexpr f32 SCROLL_ROW_PITCH = 58.0f;
        constexpr f32 SCROLL_WHEEL_SCALE = 32.0f;
        constexpr u32 SCROLL_ROW_COUNT = 12;

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
            canvas_non_stretch,
            canvas_stretch,
            scroll_viewport_layout,
            table_layout
        };

        struct DemoChild
        {
            const c8* label;
            GUI::SizeKind main_kind = GUI::SizeKind::fixed;
            f32 main_value = 70.0f;
            f32 cross_size = 42.0f;
            f32 min_main = 0.0f;
            f32 max_main = -1.0f;
            f32 grow = 0.0f;
            f32 shrink = 1.0f;
            Float4U color = Float4U(0.93f, 0.93f, 0.93f, 1.0f);
        };

        GUI::id_t demo_id(u32 index)
        {
            return ID_LAYOUT_DEMO_BASE + index;
        }

        GUI::id_t child_id(GUI::id_t parent, u32 index)
        {
            return parent * 100 + index + 1;
        }

        Float2U scroll_viewport_max_offset()
        {
            f32 viewport_width = SCROLL_VIEWPORT_WIDTH - SCROLL_VIEWPORT_PADDING * 2.0f;
            f32 viewport_height = SCROLL_VIEWPORT_HEIGHT - SCROLL_VIEWPORT_PADDING * 2.0f;
            f32 content_width = 20.0f + SCROLL_ROW_WIDTH;
            f32 content_height = (f32)(SCROLL_ROW_COUNT - 1) * SCROLL_ROW_PITCH + SCROLL_ROW_HEIGHT;
            return Float2U(max(content_width - viewport_width, 0.0f),
                max(content_height - viewport_height, 0.0f));
        }

        Float2U clamp_scroll_viewport_offset(const Float2U& offset)
        {
            Float2U maximum = scroll_viewport_max_offset();
            return Float2U(clamp(offset.x, 0.0f, maximum.x), clamp(offset.y, 0.0f, maximum.y));
        }

        GUI::LayoutConfig flex_container_layout(f32 width, f32 height)
        {
            GUI::LayoutConfig config = fixed_layout(width, height);
            config.padding = Float4U(10.0f);
            return config;
        }

        GUI::LayoutCallbackConfig flex_container_callbacks(GUI::FlexLayoutDesc* desc)
        {
            GUI::LayoutCallbackConfig config;
            config.algorithm = Name("gui.test.flex");
            config.measure_callback = GUI::measure_flex;
            config.callback = GUI::layout_flex;
            config.userdata = desc;
            return config;
        }

        GUI::MeasureResult measure_label(
            GUI::IContext*, const GUI::ElementHandle&, const Float2U&, void* userdata)
        {
            const c8* label = (const c8*)userdata;
            f32 width = label ? (f32)strlen(label) * 8.5f + 24.0f : 24.0f;
            GUI::MeasureResult result;
            result.minimum = Float2U(min(width, 72.0f), 28.0f);
            result.desired = Float2U(width, 34.0f);
            result.maximum = Float2U(F32_MAX, F32_MAX);
            return result;
        }

        GUI::LayoutConfig flex_child_layout(GUI::LayoutAxis axis, const DemoChild& child)
        {
            GUI::LayoutConfig config;
            GUI::SizeValue& main = axis == GUI::LayoutAxis::x ? config.width : config.height;
            GUI::SizeValue& cross = axis == GUI::LayoutAxis::x ? config.height : config.width;
            main.kind = child.main_kind;
            main.value = child.main_value;
            main.min = child.min_main;
            main.max = child.max_main;
            cross.kind = GUI::SizeKind::fixed;
            cross.value = child.cross_size;
            config.padding = Float4U(6.0f, 4.0f, 6.0f, 4.0f);
            config.flex_grow = child.grow;
            config.flex_shrink = child.shrink;
            return config;
        }

        void draw_element_text(GUI::IContext* context, const c8* text, f32 size,
            const Float4U& color = Float4U(0.0f, 0.0f, 0.0f, 1.0f))
        {
            GUI::DrawCommand command;
            command.type = GUI::DrawCommandType::text;
            command.rect_reference = GUI::DrawCommandRectReference::element;
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

        void text_block(GUI::IContext* context, const c8* text)
        {
            GUI::ElementHandle body = context->begin_element(ID_LAYOUT);
            context->set_layout_config(body, fixed_layout(SHEET_WIDTH - 128.0f, 66.0f));
            draw_text(context, RectF(0.0f, 0.0f, 1180.0f, 34.0f), text, 23.0f,
                Float4U(0.10f, 0.10f, 0.10f, 1.0f));
            context->end_element();
        }

        void demo_child(GUI::IContext* context, GUI::id_t id, const DemoChild& desc, GUI::LayoutAxis axis)
        {
            GUI::ElementHandle child = context->begin_element(id);
            context->set_layout_config(child, flex_child_layout(axis, desc));
            if(desc.main_kind == GUI::SizeKind::fit)
            {
                GUI::LayoutCallbackConfig callbacks;
                callbacks.algorithm = Name("gui.test.label");
                callbacks.measure_callback = measure_label;
                callbacks.userdata = (void*)desc.label;
                context->set_layout_callback_config(child, callbacks);
            }
            draw_rect(context, RectF(0.0f, 0.0f, 0.0f, 0.0f), desc.color, 0.0f);
            draw_element_text(context, desc.label, 15.0f);
            context->end_element();
        }

        void flex_case(GUI::IContext* context, GUI::id_t id, const c8* title, f32 width, f32 height,
            GUI::FlexLayoutDesc* desc, Span<const DemoChild> children)
        {
            GUI::ElementHandle container = context->begin_element(id);
            context->set_layout_config(container, flex_container_layout(width, height));
            context->set_layout_callback_config(container, flex_container_callbacks(desc));
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

        void plain_box(GUI::IContext* context, GUI::id_t id, const c8* label, f32 width, f32 height,
            const Float4U& color = Float4U(0.93f, 0.93f, 0.93f, 1.0f))
        {
            GUI::ElementHandle child = context->begin_element(id);
            context->set_layout_config(child, fixed_layout(width, height));
            draw_rect(context, RectF(0.0f, 0.0f, 0.0f, 0.0f), color, 0.0f);
            draw_element_text(context, label, 17.0f);
            context->end_element();
        }

        void marker_box(GUI::IContext* context, GUI::id_t id, const c8* label, f32 width, f32 height,
            const Float4U& color, bool outline)
        {
            GUI::ElementHandle marker = context->begin_element(id);
            context->set_layout_config(marker, fixed_layout(width, height));
            if(outline)
            {
                draw_outline(context, RectF(0.0f, 0.0f, width, height), color, 2.0f);
                draw_line(context, Float2U(0.0f, height * 0.5f), Float2U(width, height * 0.5f), color, 1.25f);
                draw_line(context, Float2U(width * 0.5f, 0.0f), Float2U(width * 0.5f, height), color, 1.25f);
            }
            else
            {
                draw_rect(context, RectF(0.0f, 0.0f, width, height), color, min(width, height) * 0.5f);
            }
            context->end_element();
        }

        void concept_case(GUI::IContext* context, GUI::id_t id, const c8* title, const c8* detail)
        {
            GUI::ElementHandle box = context->begin_element(id);
            context->set_layout_config(box, fixed_layout(360.0f, 126.0f));
            draw_rect(context, RectF(0.0f, 0.0f, 0.0f, 0.0f), Float4U(0.97f, 0.97f, 0.97f, 1.0f), 0.0f);
            draw_outline(context, RectF(0.0f, 0.0f, 360.0f, 126.0f), Float4U(0.0f, 0.0f, 0.0f, 1.0f), 1.0f);
            draw_text(context, RectF(18.0f, 16.0f, 320.0f, 30.0f), title, 24.0f,
                Float4U(0.0f, 0.0f, 0.0f, 1.0f));
            draw_text(context, RectF(18.0f, 58.0f, 320.0f, 46.0f), detail, 17.0f,
                Float4U(0.22f, 0.22f, 0.22f, 1.0f));
            context->end_element();
        }

        void add_case_grid(SheetState& state, u32 count)
        {
            for(u32 i = 0; i < count; ++i)
            {
                f32 x = CASE_LEFT + (f32)(i % 3) * CASE_GAP_X;
                f32 y = CASE_TOP + (f32)(i / 3) * CASE_GAP_Y;
                add_canvas_item(state.sheet_items, demo_id(i), x, y);
            }
        }

        void add_case_rows(SheetState& state, u32 count)
        {
            for(u32 i = 0; i < count; ++i)
            {
                add_canvas_item(state.sheet_items, demo_id(i), 92.0f, 244.0f + (f32)i * 74.0f);
            }
        }

        void build_overview(GUI::IContext* context)
        {
            text_block(context, "Each layout algorithm gets its own slice. Flex gets extra slices for every core parameter group.");
            concept_case(context, demo_id(0), "Flex", "Ordered row or column layout with grow, shrink, wrap and alignment.");
            concept_case(context, demo_id(1), "Grid", "Row-major tiled cells for browsers and icon palettes.");
            concept_case(context, demo_id(2), "Canvas", "Anchor, offset and pivot based placement.");
            concept_case(context, demo_id(3), "ScrollViewport", "Translate and clip scroll content.");
            concept_case(context, demo_id(4), "Table", "Explicit tracks and cell attachments.");
        }

        void build_flex_axis(GUI::IContext* context)
        {
            text_block(context, "`axis` selects the main direction. The same children are arranged as a row or as a column.");
            static GUI::FlexLayoutDesc row;
            row = GUI::FlexLayoutDesc();
            row.axis = GUI::LayoutAxis::x;
            row.main_axis_gap = 12.0f;
            row.cross_alignment = GUI::FlexAlignment::center;
            static GUI::FlexLayoutDesc column;
            column = row;
            column.axis = GUI::LayoutAxis::y;
            DemoChild children[] = {
                { "A", GUI::SizeKind::fixed, 86.0f, 48.0f },
                { "B", GUI::SizeKind::fixed, 118.0f, 48.0f },
                { "C", GUI::SizeKind::fixed, 72.0f, 48.0f }
            };
            flex_case(context, demo_id(0), "LayoutAxis::x", 360.0f, 120.0f, &row, Span<const DemoChild>(children, 3));
            flex_case(context, demo_id(1), "LayoutAxis::y", 360.0f, 166.0f, &column, Span<const DemoChild>(children, 3));
        }

        void build_flex_reverse(GUI::IContext* context)
        {
            text_block(context, "`reverse` flips only placement on the main axis. Element order and focus order stay authored order.");
            DemoChild children[] = {
                { "A", GUI::SizeKind::fixed, 72.0f, 42.0f },
                { "B", GUI::SizeKind::fixed, 96.0f, 42.0f },
                { "C", GUI::SizeKind::fixed, 72.0f, 42.0f }
            };
            static GUI::FlexLayoutDesc descs[4];
            for(u32 i = 0; i < 4; ++i)
            {
                descs[i] = GUI::FlexLayoutDesc();
                descs[i].axis = i < 2 ? GUI::LayoutAxis::x : GUI::LayoutAxis::y;
                descs[i].reverse = (i % 2) != 0;
                descs[i].main_axis_gap = 10.0f;
                descs[i].cross_alignment = GUI::FlexAlignment::center;
            }
            flex_case(context, demo_id(0), "row normal", 340.0f, 94.0f, &descs[0], Span<const DemoChild>(children, 3));
            flex_case(context, demo_id(1), "row reverse", 340.0f, 94.0f, &descs[1], Span<const DemoChild>(children, 3));
            flex_case(context, demo_id(2), "column normal", 220.0f, 160.0f, &descs[2], Span<const DemoChild>(children, 3));
            flex_case(context, demo_id(3), "column reverse", 220.0f, 160.0f, &descs[3], Span<const DemoChild>(children, 3));
        }

        void build_flex_wrap(GUI::IContext* context)
        {
            text_block(context, "`wrap` decides whether items stay on one line or create additional cross-axis lines.");
            DemoChild children[] = {
                { "A", GUI::SizeKind::fixed, 124.0f, 40.0f },
                { "B", GUI::SizeKind::fixed, 104.0f, 40.0f },
                { "C", GUI::SizeKind::fixed, 116.0f, 40.0f },
                { "D", GUI::SizeKind::fixed, 96.0f, 40.0f },
                { "E", GUI::SizeKind::fixed, 112.0f, 40.0f }
            };
            static GUI::FlexLayoutDesc descs[3];
            for(u32 i = 0; i < 3; ++i)
            {
                descs[i] = GUI::FlexLayoutDesc();
                descs[i].axis = GUI::LayoutAxis::x;
                descs[i].wrap = i == 0 ? GUI::FlexWrap::none : (i == 1 ? GUI::FlexWrap::wrap : GUI::FlexWrap::wrap_reverse);
                descs[i].main_axis_gap = 10.0f;
                descs[i].cross_axis_gap = 8.0f;
            }
            flex_case(context, demo_id(0), "FlexWrap::none", 360.0f, 150.0f, &descs[0], Span<const DemoChild>(children, 5));
            flex_case(context, demo_id(1), "FlexWrap::wrap", 360.0f, 160.0f, &descs[1], Span<const DemoChild>(children, 5));
            flex_case(context, demo_id(2), "FlexWrap::wrap_reverse", 360.0f, 160.0f, &descs[2], Span<const DemoChild>(children, 5));
        }

        void build_flex_main_alignment(GUI::IContext* context)
        {
            text_block(context, "`main_alignment` distributes free space on one flex line. Use flex_grow, not stretch, to resize items.");
            const GUI::FlexAlignment alignments[] = {
                GUI::FlexAlignment::start,
                GUI::FlexAlignment::center,
                GUI::FlexAlignment::end,
                GUI::FlexAlignment::space_between,
                GUI::FlexAlignment::space_around,
                GUI::FlexAlignment::space_evenly
            };
            const c8* titles[] = { "start", "center", "end", "space_between", "space_around", "space_evenly" };
            DemoChild children[] = {
                { "A", GUI::SizeKind::fixed, 58.0f, 38.0f },
                { "B", GUI::SizeKind::fixed, 82.0f, 38.0f },
                { "C", GUI::SizeKind::fixed, 58.0f, 38.0f }
            };
            static GUI::FlexLayoutDesc descs[6];
            for(u32 i = 0; i < 6; ++i)
            {
                descs[i] = GUI::FlexLayoutDesc();
                descs[i].axis = GUI::LayoutAxis::x;
                descs[i].main_alignment = alignments[i];
                descs[i].cross_alignment = GUI::FlexAlignment::center;
                descs[i].main_axis_gap = 8.0f;
                flex_case(context, demo_id(i), titles[i], 340.0f, 86.0f, &descs[i], Span<const DemoChild>(children, 3));
            }
        }

        void build_flex_cross_alignment(GUI::IContext* context)
        {
            text_block(context, "`cross_alignment` aligns items inside one line on the cross axis.");
            const GUI::FlexAlignment alignments[] = {
                GUI::FlexAlignment::start,
                GUI::FlexAlignment::center,
                GUI::FlexAlignment::end,
                GUI::FlexAlignment::stretch
            };
            const c8* titles[] = { "start", "center", "end", "stretch" };
            DemoChild children[] = {
                { "Tall", GUI::SizeKind::fixed, 74.0f, 86.0f },
                { "Short", GUI::SizeKind::fixed, 74.0f, 38.0f },
                { "Mid", GUI::SizeKind::fixed, 74.0f, 62.0f }
            };
            static GUI::FlexLayoutDesc descs[4];
            for(u32 i = 0; i < 4; ++i)
            {
                descs[i] = GUI::FlexLayoutDesc();
                descs[i].axis = GUI::LayoutAxis::x;
                descs[i].cross_alignment = alignments[i];
                descs[i].main_axis_gap = 12.0f;
                flex_case(context, demo_id(i), titles[i], 340.0f, 116.0f, &descs[i], Span<const DemoChild>(children, 3));
            }
        }

        void build_flex_line_alignment(GUI::IContext* context)
        {
            text_block(context, "`line_alignment` distributes multiple wrapped lines on the cross axis.");
            const GUI::FlexAlignment alignments[] = {
                GUI::FlexAlignment::start,
                GUI::FlexAlignment::center,
                GUI::FlexAlignment::end,
                GUI::FlexAlignment::stretch,
                GUI::FlexAlignment::space_between,
                GUI::FlexAlignment::space_around,
                GUI::FlexAlignment::space_evenly
            };
            const c8* titles[] = { "start", "center", "end", "stretch", "space_between", "space_around", "space_evenly" };
            DemoChild children[] = {
                { "A", GUI::SizeKind::fixed, 90.0f, 32.0f },
                { "B", GUI::SizeKind::fixed, 96.0f, 32.0f },
                { "C", GUI::SizeKind::fixed, 88.0f, 32.0f },
                { "D", GUI::SizeKind::fixed, 92.0f, 32.0f },
                { "E", GUI::SizeKind::fixed, 82.0f, 32.0f }
            };
            static GUI::FlexLayoutDesc descs[7];
            for(u32 i = 0; i < 7; ++i)
            {
                descs[i] = GUI::FlexLayoutDesc();
                descs[i].axis = GUI::LayoutAxis::x;
                descs[i].wrap = GUI::FlexWrap::wrap;
                descs[i].line_alignment = alignments[i];
                descs[i].main_axis_gap = 8.0f;
                descs[i].cross_axis_gap = 4.0f;
                flex_case(context, demo_id(i), titles[i], 310.0f, 128.0f, &descs[i], Span<const DemoChild>(children, 5));
            }
        }

        void build_flex_gap(GUI::IContext* context)
        {
            text_block(context, "`main_axis_gap` separates items on a line. `cross_axis_gap` separates wrapped lines.");
            DemoChild children[] = {
                { "A", GUI::SizeKind::fixed, 84.0f, 36.0f },
                { "B", GUI::SizeKind::fixed, 84.0f, 36.0f },
                { "C", GUI::SizeKind::fixed, 84.0f, 36.0f },
                { "D", GUI::SizeKind::fixed, 84.0f, 36.0f },
                { "E", GUI::SizeKind::fixed, 84.0f, 36.0f },
                { "F", GUI::SizeKind::fixed, 84.0f, 36.0f }
            };
            static GUI::FlexLayoutDesc descs[3];
            for(u32 i = 0; i < 3; ++i)
            {
                descs[i] = GUI::FlexLayoutDesc();
                descs[i].axis = GUI::LayoutAxis::x;
                descs[i].wrap = GUI::FlexWrap::wrap;
            }
            descs[0].main_axis_gap = 6.0f;
            descs[1].main_axis_gap = 26.0f;
            descs[2].main_axis_gap = 10.0f;
            descs[2].cross_axis_gap = 28.0f;
            flex_case(context, demo_id(0), "main_axis_gap = 6", 350.0f, 120.0f, &descs[0], Span<const DemoChild>(children, 6));
            flex_case(context, demo_id(1), "main_axis_gap = 26", 350.0f, 120.0f, &descs[1], Span<const DemoChild>(children, 6));
            flex_case(context, demo_id(2), "main = 10, cross = 28", 350.0f, 150.0f, &descs[2], Span<const DemoChild>(children, 6));
        }

        void build_flex_item_sizing(GUI::IContext* context)
        {
            text_block(context, "Children provide fixed, percent or fit sizes plus min/max, grow and shrink constraints.");
            static GUI::FlexLayoutDesc descs[6];
            for(u32 i = 0; i < 6; ++i)
            {
                descs[i] = GUI::FlexLayoutDesc();
                descs[i].axis = GUI::LayoutAxis::x;
                descs[i].main_axis_gap = 8.0f;
                descs[i].cross_alignment = GUI::FlexAlignment::center;
            }
            DemoChild fixed_children[] = {
                { "fixed 90", GUI::SizeKind::fixed, 90.0f, 38.0f },
                { "fixed 130", GUI::SizeKind::fixed, 130.0f, 38.0f }
            };
            DemoChild percent_children[] = {
                { "25%", GUI::SizeKind::percent, 0.25f, 38.0f },
                { "40%", GUI::SizeKind::percent, 0.40f, 38.0f }
            };
            DemoChild fit_children[] = {
                { "fit", GUI::SizeKind::fit, 0.0f, 38.0f },
                { "fit longer text", GUI::SizeKind::fit, 0.0f, 38.0f }
            };
            DemoChild grow_children[] = {
                { "grow 1", GUI::SizeKind::fixed, 48.0f, 38.0f, 0.0f, -1.0f, 1.0f },
                { "grow 2", GUI::SizeKind::fixed, 48.0f, 38.0f, 0.0f, -1.0f, 2.0f },
                { "grow 1", GUI::SizeKind::fixed, 48.0f, 38.0f, 0.0f, -1.0f, 1.0f }
            };
            DemoChild shrink_children[] = {
                { "no shrink", GUI::SizeKind::fixed, 150.0f, 38.0f, 0.0f, -1.0f, 0.0f, 0.0f },
                { "shrink", GUI::SizeKind::fixed, 150.0f, 38.0f, 0.0f, -1.0f, 0.0f, 1.0f },
                { "shrink", GUI::SizeKind::fixed, 150.0f, 38.0f, 0.0f, -1.0f, 0.0f, 1.0f }
            };
            DemoChild clamp_children[] = {
                { "min 96", GUI::SizeKind::fixed, 42.0f, 38.0f, 96.0f },
                { "max 100 grow", GUI::SizeKind::fixed, 62.0f, 38.0f, 0.0f, 100.0f, 1.0f },
                { "grow", GUI::SizeKind::fixed, 62.0f, 38.0f, 0.0f, -1.0f, 1.0f }
            };
            flex_case(context, demo_id(0), "fixed", 350.0f, 86.0f, &descs[0], Span<const DemoChild>(fixed_children, 2));
            flex_case(context, demo_id(1), "percent", 350.0f, 86.0f, &descs[1], Span<const DemoChild>(percent_children, 2));
            flex_case(context, demo_id(2), "fit by measure callback", 350.0f, 86.0f, &descs[2], Span<const DemoChild>(fit_children, 2));
            flex_case(context, demo_id(3), "flex_grow", 350.0f, 86.0f, &descs[3], Span<const DemoChild>(grow_children, 3));
            flex_case(context, demo_id(4), "flex_shrink", 350.0f, 86.0f, &descs[4], Span<const DemoChild>(shrink_children, 3));
            flex_case(context, demo_id(5), "min / max", 350.0f, 86.0f, &descs[5], Span<const DemoChild>(clamp_children, 3));
        }

        void build_flex_clipping(GUI::IContext* context)
        {
            text_block(context, "`clip_children` intersects child clip rects with the parent content rect. It affects hit testing and explicit clipping.");
            DemoChild children[] = {
                { "A", GUI::SizeKind::fixed, 120.0f, 42.0f, 0.0f, -1.0f, 0.0f, 0.0f },
                { "B", GUI::SizeKind::fixed, 120.0f, 42.0f, 0.0f, -1.0f, 0.0f, 0.0f },
                { "C overflow", GUI::SizeKind::fixed, 150.0f, 42.0f, 0.0f, -1.0f, 0.0f, 0.0f }
            };
            static GUI::FlexLayoutDesc descs[2];
            for(u32 i = 0; i < 2; ++i)
            {
                descs[i] = GUI::FlexLayoutDesc();
                descs[i].axis = GUI::LayoutAxis::x;
                descs[i].main_axis_gap = 10.0f;
                descs[i].clip_children = i == 0;
            }
            flex_case(context, demo_id(0), "clip_children = true", 330.0f, 86.0f, &descs[0], Span<const DemoChild>(children, 3));
            flex_case(context, demo_id(1), "clip_children = false", 330.0f, 86.0f, &descs[1], Span<const DemoChild>(children, 3));
        }

        void build_grid_layout(GUI::IContext* context)
        {
            text_block(context, "Grid is row-major. It can use fixed cell size or derive cell width from a fixed column count.");
            static GUI::GridLayoutDesc fixed_cell;
            fixed_cell = GUI::GridLayoutDesc();
            fixed_cell.mode = GUI::GridLayoutMode::fixed_cell_size;
            fixed_cell.cell_size = Float2U(92.0f, 54.0f);
            fixed_cell.gap = Float2U(10.0f, 10.0f);
            static GUI::GridLayoutDesc columns;
            columns = fixed_cell;
            columns.mode = GUI::GridLayoutMode::fixed_column_count;
            columns.column_count = 4;
            for(u32 c = 0; c < 2; ++c)
            {
                GUI::ElementHandle grid = context->begin_element(demo_id(c));
                context->set_layout_config(grid, fixed_layout(430.0f, 170.0f));
                GUI::LayoutConfig layout = fixed_layout(430.0f, 170.0f);
                layout.padding = Float4U(10.0f);
                context->set_layout_config(grid, layout);
                GUI::LayoutCallbackConfig callbacks;
                callbacks.algorithm = Name("gui.test.grid");
                callbacks.callback = GUI::layout_grid;
                callbacks.userdata = c ? (void*)&columns : (void*)&fixed_cell;
                context->set_layout_callback_config(grid, callbacks);
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

        void build_canvas_non_stretch_case(GUI::IContext* context, u32 case_index, const c8* title,
            const Float2U& anchor, const Float4U& offset, const Float2U& pivot)
        {
            constexpr f32 WIDTH = 340.0f;
            constexpr f32 HEIGHT = 168.0f;
            constexpr f32 PADDING = 12.0f;
            GUI::id_t id = demo_id(case_index);
            GUI::id_t box_id = child_id(id, 0);
            GUI::id_t anchor_id = child_id(id, 1);
            GUI::id_t pivot_id = child_id(id, 2);

            static GUI::CanvasLayoutItem items[3][3];
            static GUI::CanvasLayoutDesc descs[3];
            GUI::CanvasLayoutItem& box_item = items[case_index][0];
            box_item = GUI::CanvasLayoutItem();
            box_item.element_id = box_id;
            box_item.anchor_min = anchor;
            box_item.anchor_max = anchor;
            box_item.offset = offset;
            box_item.pivot = pivot;

            GUI::CanvasLayoutItem& anchor_item = items[case_index][1];
            anchor_item = GUI::CanvasLayoutItem();
            anchor_item.element_id = anchor_id;
            anchor_item.anchor_min = anchor;
            anchor_item.anchor_max = anchor;
            anchor_item.offset = offset;
            anchor_item.pivot = Float2U(0.5f, 0.5f);

            GUI::CanvasLayoutItem& pivot_item = items[case_index][2];
            pivot_item = anchor_item;
            pivot_item.element_id = pivot_id;

            descs[case_index] = GUI::CanvasLayoutDesc();
            descs[case_index].items = Span<const GUI::CanvasLayoutItem>(items[case_index], 3);
            descs[case_index].clip_children = true;

            GUI::ElementHandle canvas = context->begin_element(id);
            GUI::LayoutConfig layout = fixed_layout(WIDTH, HEIGHT);
            layout.padding = Float4U(PADDING);
            context->set_layout_config(canvas, layout);
            draw_text(context, RectF(0.0f, -32.0f, WIDTH, 26.0f), title, 20.0f,
                Float4U(0.0f, 0.0f, 0.0f, 1.0f));
            draw_rect(context, RectF(0.0f, 0.0f, 0.0f, 0.0f), Float4U(0.97f, 0.97f, 0.97f, 1.0f));
            draw_outline(context, RectF(0.0f, 0.0f, WIDTH, HEIGHT), Float4U(0.0f, 0.0f, 0.0f, 1.0f));
            draw_outline(context, RectF(PADDING, PADDING, WIDTH - PADDING * 2.0f, HEIGHT - PADDING * 2.0f),
                Float4U(0.68f, 0.68f, 0.68f, 1.0f));
            draw_text(context, RectF(14.0f, HEIGHT - 36.0f, WIDTH - 28.0f, 24.0f),
                "pos = anchor + offset - measured_size * pivot", 14.0f, Float4U(0.22f, 0.22f, 0.22f, 1.0f),
                VG::TextAlignment::center);
            plain_box(context, box_id, "fixed child", 112.0f, 54.0f);
            marker_box(context, anchor_id, "anchor point", 18.0f, 18.0f, Float4U(0.0f, 0.36f, 0.95f, 1.0f), true);
            marker_box(context, pivot_id, "pivot point", 8.0f, 8.0f, Float4U(0.95f, 0.12f, 0.12f, 1.0f), false);
            set_canvas_layout(context, canvas, &descs[case_index]);
            context->end_element();
        }

        void build_canvas_non_stretch(GUI::IContext* context)
        {
            text_block(context, "Non-stretch axes use anchor point, offset and pivot. The child keeps its measured fixed size.");
            build_canvas_non_stretch_case(context, 0, "top-left anchor, top-left pivot",
                Float2U(0.0f, 0.0f), Float4U(28.0f, 28.0f, 0.0f, 0.0f), Float2U(0.0f, 0.0f));
            build_canvas_non_stretch_case(context, 1, "center anchor, center pivot",
                Float2U(0.5f, 0.5f), Float4U(0.0f), Float2U(0.5f, 0.5f));
            build_canvas_non_stretch_case(context, 2, "bottom-right anchor, bottom-right pivot",
                Float2U(1.0f, 1.0f), Float4U(-28.0f, -28.0f, 0.0f, 0.0f), Float2U(1.0f, 1.0f));
        }

        void build_canvas_stretch_case(GUI::IContext* context, u32 case_index, const c8* title,
            f32 width, f32 height, const Float2U& anchor_min, const Float2U& anchor_max, const Float4U& offset)
        {
            constexpr f32 PADDING = 12.0f;
            GUI::id_t id = demo_id(case_index);
            GUI::id_t child = child_id(id, 0);
            GUI::id_t anchor_rect = child_id(id, 1);

            static GUI::CanvasLayoutItem items[3][2];
            static GUI::CanvasLayoutDesc descs[3];
            GUI::CanvasLayoutItem& child_item = items[case_index][0];
            child_item = GUI::CanvasLayoutItem();
            child_item.element_id = child;
            child_item.anchor_min = anchor_min;
            child_item.anchor_max = anchor_max;
            child_item.offset = offset;

            GUI::CanvasLayoutItem& anchor_item = items[case_index][1];
            anchor_item = GUI::CanvasLayoutItem();
            anchor_item.element_id = anchor_rect;
            anchor_item.anchor_min = anchor_min;
            anchor_item.anchor_max = anchor_max;

            descs[case_index] = GUI::CanvasLayoutDesc();
            descs[case_index].items = Span<const GUI::CanvasLayoutItem>(items[case_index], 2);
            descs[case_index].clip_children = true;

            f32 content_width = width - PADDING * 2.0f;
            f32 content_height = height - PADDING * 2.0f;
            f32 anchor_width = content_width * (anchor_max.x - anchor_min.x);
            f32 anchor_height = content_height * (anchor_max.y - anchor_min.y);

            GUI::ElementHandle canvas = context->begin_element(id);
            GUI::LayoutConfig layout = fixed_layout(width, height);
            layout.padding = Float4U(PADDING);
            context->set_layout_config(canvas, layout);
            draw_text(context, RectF(0.0f, -32.0f, width, 26.0f), title, 20.0f,
                Float4U(0.0f, 0.0f, 0.0f, 1.0f));
            draw_rect(context, RectF(0.0f, 0.0f, 0.0f, 0.0f), Float4U(0.97f, 0.97f, 0.97f, 1.0f));
            draw_outline(context, RectF(0.0f, 0.0f, width, height), Float4U(0.0f, 0.0f, 0.0f, 1.0f));
            draw_outline(context, RectF(PADDING, PADDING, content_width, content_height), Float4U(0.68f, 0.68f, 0.68f, 1.0f));
            draw_text(context, RectF(14.0f, height - 30.0f, width - 28.0f, 22.0f),
                "begin = anchor_min + offset_begin, end = anchor_max + offset_end", 13.0f,
                Float4U(0.22f, 0.22f, 0.22f, 1.0f), VG::TextAlignment::center);

            plain_box(context, child, "stretched child", 80.0f, 40.0f, Float4U(0.90f, 0.92f, 0.96f, 0.92f));
            GUI::ElementHandle marker = context->begin_element(anchor_rect);
            context->set_layout_config(marker, fixed_layout(anchor_width, anchor_height));
            draw_outline(context, RectF(0.0f, 0.0f, anchor_width, anchor_height), Float4U(0.95f, 0.55f, 0.0f, 1.0f), 2.0f);
            context->end_element();

            set_canvas_layout(context, canvas, &descs[case_index]);
            context->end_element();
        }

        void build_canvas_stretch(GUI::IContext* context)
        {
            text_block(context, "Stretch axes use anchor rect plus left/top/right/bottom offsets. Pivot and measured size do not control that axis.");
            build_canvas_stretch_case(context, 0, "small canvas, same anchor rect", 330.0f, 170.0f,
                Float2U(0.15f, 0.20f), Float2U(0.85f, 0.70f), Float4U(0.0f));
            build_canvas_stretch_case(context, 1, "larger canvas, same anchor rect", 390.0f, 210.0f,
                Float2U(0.15f, 0.20f), Float2U(0.85f, 0.70f), Float4U(0.0f));
            build_canvas_stretch_case(context, 2, "anchor rect with inward offsets", 390.0f, 210.0f,
                Float2U(0.10f, 0.16f), Float2U(0.92f, 0.78f), Float4U(24.0f, 18.0f, -28.0f, -22.0f));
        }

        void build_scroll_viewport_layout(GUI::IContext* context, SheetState& state)
        {
            text_block(context, "ScrollViewport receives routed wheel input, updates scroll_offset and clips translated content.");
            GUI::ScrollViewportLayoutDesc& desc = state.scroll_viewport_layout;
            desc.scroll_offset = clamp_scroll_viewport_offset(desc.scroll_offset);
            desc.max_scroll_delta = Float2U(96.0f, 96.0f);
            desc.clip_children = true;
            GUI::LayoutConfig layout = fixed_layout(SCROLL_VIEWPORT_WIDTH, SCROLL_VIEWPORT_HEIGHT);
            layout.padding = Float4U(SCROLL_VIEWPORT_PADDING);
            GUI::ElementHandle viewport = context->begin_element(demo_id(0));
            context->set_layout_config(viewport, layout);
            GUI::LayoutCallbackConfig callbacks;
            callbacks.algorithm = Name("gui.test.scroll_viewport");
            callbacks.callback = GUI::layout_scroll_viewport;
            callbacks.userdata = &desc;
            context->set_layout_callback_config(viewport, callbacks);
            set_interactable(context, viewport, GUI::PointerHitBehavior::target,
                GUI::InteractableFlag::hoverable | GUI::InteractableFlag::scrollable);
            RectF visible_rect = GUI::get_scroll_viewport_visible_rect(context, viewport);
            char visibility_label[192];
            snprintf(visibility_label, sizeof(visibility_label),
                "offset = (%.0f, %.0f) | previous visible rect = (%.0f, %.0f, %.0f, %.0f) | max delta = (%.0f, %.0f)",
                desc.scroll_offset.x, desc.scroll_offset.y,
                visible_rect.offset_x, visible_rect.offset_y, visible_rect.width, visible_rect.height,
                desc.max_scroll_delta.x, desc.max_scroll_delta.y);
            draw_text(context, RectF(0.0f, -58.0f, 1100.0f, 24.0f),
                "Move the pointer over the viewport and use the mouse wheel or trackpad to scroll.",
                18.0f, Float4U(0.18f, 0.18f, 0.18f, 1.0f));
            draw_text(context, RectF(0.0f, -30.0f, 1100.0f, 24.0f), visibility_label,
                16.0f, Float4U(0.28f, 0.28f, 0.28f, 1.0f));
            draw_rect(context, RectF(0.0f, 0.0f, 0.0f, 0.0f), Float4U(0.97f, 0.97f, 0.97f, 1.0f));
            GUI::InteractionState interaction = context->get_interaction_state(viewport.id);
            Float4U outline_color = interaction.hovered ?
                Float4U(0.0f, 0.48f, 0.86f, 1.0f) : Float4U(0.0f, 0.0f, 0.0f, 1.0f);
            draw_outline(context, RectF(0.0f, 0.0f, SCROLL_VIEWPORT_WIDTH, SCROLL_VIEWPORT_HEIGHT),
                outline_color, interaction.hovered ? 2.0f : 1.0f);
            for(u32 i = 0; i < SCROLL_ROW_COUNT; ++i)
            {
                GUI::LayoutConfig item = fixed_layout(SCROLL_ROW_WIDTH, SCROLL_ROW_HEIGHT);
                item.margin = Float4U(20.0f, (f32)i * SCROLL_ROW_PITCH, 0.0f, 0.0f);
                char label[32];
                snprintf(label, sizeof(label), "content row %02u", i);
                Float4U row_color = (i & 1) ?
                    Float4U(0.90f, 0.93f, 0.96f, 1.0f) : Float4U(0.94f, 0.94f, 0.94f, 1.0f);
                plain_box(context, child_id(demo_id(0), i), label,
                    SCROLL_ROW_WIDTH, SCROLL_ROW_HEIGHT, row_color);
                context->set_layout_config(context->find_element_handle(child_id(demo_id(0), i)), item);
            }
            context->end_element();
        }

        void build_table_layout(GUI::IContext* context)
        {
            text_block(context, "Table uses explicit tracks and child-to-cell attachments, making virtualization and authored tables predictable.");
            static GUI::TableTrackDesc columns[3];
            static GUI::TableTrackDesc rows[3];
            static GUI::TableLayoutCell cells[9];
            static GUI::TableLayoutDesc desc;
            columns[0].kind = GUI::TableTrackSizeKind::pixels;
            columns[0].value = 92.0f;
            columns[1].kind = GUI::TableTrackSizeKind::ratio;
            columns[1].value = 1.0f;
            columns[2].kind = GUI::TableTrackSizeKind::pixels;
            columns[2].value = 126.0f;
            for(u32 i = 0; i < 3; ++i)
            {
                rows[i].kind = GUI::TableTrackSizeKind::pixels;
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
            desc.columns = Span<const GUI::TableTrackDesc>(columns, 3);
            desc.rows = Span<const GUI::TableTrackDesc>(rows, 3);
            desc.cells = Span<const GUI::TableLayoutCell>(cells, 9);
            desc.gap = Float2U(6.0f, 6.0f);
            GUI::LayoutConfig layout = fixed_layout(560.0f, 170.0f);
            layout.padding = Float4U(10.0f);
            GUI::ElementHandle table = context->begin_element(demo_id(0));
            context->set_layout_config(table, layout);
            GUI::LayoutCallbackConfig callbacks;
            callbacks.algorithm = Name("gui.test.table");
            callbacks.callback = GUI::layout_table;
            callbacks.userdata = &desc;
            context->set_layout_callback_config(table, callbacks);
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
        case layout_overview: return "GUI Layout";
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
        case canvas_non_stretch: return "Canvas: Non-Stretch";
        case canvas_stretch: return "Canvas: Stretch";
        case scroll_viewport_layout: return "Scroll Viewport Layout";
        case table_layout: return "Table Layout";
        default: return "GUI Layout";
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
        case canvas_non_stretch: return "Non-stretch axes use measured child size, anchor point, offset and pivot.";
        case canvas_stretch: return "Stretch axes derive child size from an anchor rectangle and edge offsets.";
        case scroll_viewport_layout: return "ScrollViewport translates and clips content.";
        case table_layout: return "Table uses explicit tracks and child-to-cell attachments.";
        default: return "Layout is data on elements, evaluated by context-level algorithms.";
        }
    }

    void add_layout_slice_items(SheetState& state, u32 layout_slice)
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
        case canvas_non_stretch:
        case canvas_stretch:
            add_case_grid(state, 3);
            break;
        case flex_line_alignment:
            add_case_grid(state, 7);
            break;
        case scroll_viewport_layout:
            add_canvas_item(state.sheet_items, demo_id(0), 112.0f, 318.0f);
            break;
        case table_layout:
            add_canvas_item(state.sheet_items, demo_id(0), 112.0f, 282.0f);
            break;
        default:
            add_case_rows(state, 1);
            break;
        }
    }

    void build_layout_slice(GUI::IContext* context, SheetState& state, u32 layout_slice)
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
        case canvas_non_stretch:
            build_canvas_non_stretch(context);
            break;
        case canvas_stretch:
            build_canvas_stretch(context);
            break;
        case scroll_viewport_layout:
            build_scroll_viewport_layout(context, state);
            break;
        case table_layout:
            build_table_layout(context);
            break;
        default:
            build_overview(context);
            break;
        }
    }

    bool process_layout_slice_input(GUI::IContext* context, SheetState& state, u32 layout_slice)
    {
        if(!context || layout_slice != scroll_viewport_layout)
        {
            return false;
        }

        Float2U scroll_delta(0.0f);
        Span<const GUI::RoutedInputEvent> events = context->get_routed_input_events(demo_id(0));
        for(const GUI::RoutedInputEvent& routed : events)
        {
            if(routed.event.type == GUI::InputEventType::pointer_wheel)
            {
                scroll_delta.x -= routed.event.wheel_delta.x * SCROLL_WHEEL_SCALE;
                scroll_delta.y -= routed.event.wheel_delta.y * SCROLL_WHEEL_SCALE;
            }
        }

        const Float2U& max_delta = state.scroll_viewport_layout.max_scroll_delta;
        scroll_delta.x = clamp(scroll_delta.x, -max_delta.x, max_delta.x);
        scroll_delta.y = clamp(scroll_delta.y, -max_delta.y, max_delta.y);
        Float2U old_offset = state.scroll_viewport_layout.scroll_offset;
        Float2U new_offset = clamp_scroll_viewport_offset(Float2U(
            old_offset.x + scroll_delta.x, old_offset.y + scroll_delta.y));
        if(new_offset.x == old_offset.x && new_offset.y == old_offset.y)
        {
            return false;
        }
        state.scroll_viewport_layout.scroll_offset = new_offset;
        return true;
    }
}
