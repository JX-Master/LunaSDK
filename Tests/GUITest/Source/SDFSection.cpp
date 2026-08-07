/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file SDFSection.cpp
* @author JXMaster
* @date 2026/7/17
*/
#include "GUITest.hpp"

namespace Luna::GUITest
{
    namespace
    {
        using namespace GUI;

        constexpr f32 TILE_WIDTH = 232.0f;
        constexpr f32 TILE_HEIGHT = 116.0f;
        constexpr f32 TILE_GAP_X = 18.0f;
        constexpr f32 TILE_GAP_Y = 14.0f;
        const RectF SAMPLE_RECT = RectF(8.0f, 39.0f, 216.0f, 68.0f);

        enum class SDFSample : u32
        {
            rectangle,
            rounded_rectangle,
            circle,
            ellipse,
            capsule,
            union_op,
            intersection_op,
            difference_op,
            xor_op,
            solid,
            linear_gradient,
            radial_gradient,
            conic_gradient,
            bilinear_gradient,
            shadow,
            no_clip,
            inner_clip,
            outer_clip,
            inner_outer_clip,
            count
        };

        constexpr const c8* SAMPLE_TITLES[] = {
            "Rectangle", "Rounded rectangle", "Circle", "Ellipse", "Capsule",
            "Union", "Intersection", "Difference", "Exclusive or",
            "Solid color", "Linear gradient", "Radial gradient", "Conic gradient", "Bilinear gradient",
            "Outer + inner shadow", "No clip", "Inner clip", "Outer clip", "Inner + outer clip"
        };

        constexpr const c8* SAMPLE_OPCODES[] = {
            "rectangle", "rounded_rectangle", "circle", "ellipse", "capsule",
            "union_op", "intersection_op", "difference_op", "xor_op",
            "solid", "linear_gradient", "radial_gradient", "conic_gradient", "bilinear_gradient",
            "3 effects / 1 color span", "00b", "01b  inner=7", "10b  outer=8",
            "11b  inner=7  outer=8"
        };

        GUI::id_t sample_id(SDFSample sample)
        {
            return ID_SDF_SAMPLE_BASE + (u32)sample;
        }

        void sample_grid_position(SDFSample sample, u32& column, u32& row)
        {
            u32 index = (u32)sample;
            if(index < 5)
            {
                column = index;
                row = 0;
            }
            else if(index < 9)
            {
                column = index - 5;
                row = 1;
            }
            else if(index < 14)
            {
                column = index - 9;
                row = 2;
            }
            else
            {
                column = index - 14;
                row = 3;
            }
        }

        void build_sample_shape(Vector<f32>& floats, SDFSample sample)
        {
            const Float2U center(SAMPLE_RECT.width * 0.5f, SAMPLE_RECT.height * 0.5f);
            const RectF shape_rect(17.0f, 10.0f, SAMPLE_RECT.width - 34.0f, SAMPLE_RECT.height - 20.0f);
            switch(sample)
            {
            case SDFSample::rectangle:
                sdf_shape_add_rectangle(floats, shape_rect);
                break;
            case SDFSample::rounded_rectangle:
                sdf_shape_add_rounded_rectangle(floats, shape_rect, Float4U(16.0f, 5.0f, 16.0f, 5.0f));
                break;
            case SDFSample::circle:
                sdf_shape_add_circle(floats, center, 23.0f);
                break;
            case SDFSample::ellipse:
                sdf_shape_add_ellipse(floats, center, Float2U(78.0f, 22.0f));
                break;
            case SDFSample::capsule:
                sdf_shape_add_capsule(floats, Float2U(28.0f, center.y),
                    Float2U(SAMPLE_RECT.width - 28.0f, center.y), 18.0f);
                break;
            case SDFSample::union_op:
            case SDFSample::intersection_op:
            case SDFSample::difference_op:
            case SDFSample::xor_op:
            {
                SDFShapeInstruction operation = SDFShapeInstruction::union_op;
                if(sample == SDFSample::intersection_op) operation = SDFShapeInstruction::intersection_op;
                else if(sample == SDFSample::difference_op) operation = SDFShapeInstruction::difference_op;
                else if(sample == SDFSample::xor_op) operation = SDFShapeInstruction::xor_op;
                sdf_shape_add_operation(floats, operation);
                sdf_shape_add_circle(floats, Float2U(center.x - 18.0f, center.y), 25.0f);
                sdf_shape_add_circle(floats, Float2U(center.x + 18.0f, center.y), 25.0f);
                break;
            }
            case SDFSample::no_clip:
            case SDFSample::inner_clip:
            case SDFSample::outer_clip:
            case SDFSample::inner_outer_clip:
                sdf_shape_add_circle(floats, center, 22.0f);
                break;
            default:
                sdf_shape_add_rounded_rectangle(floats, shape_rect, Float4U(13.0f));
                break;
            }
        }

        RV draw_sdf_sample(GUI::IContext* context, const GUI::ElementHandle& element,
            GUI::DrawPhase, void* userdata)
        {
            SDFSample sample = (SDFSample)(u32)(usize)userdata;
            u32 sample_index = (u32)sample;
            if(sample_index >= (u32)SDFSample::count) return ok;

            draw_rect(context, RectF(), Float4U(0.975f, 0.975f, 0.975f, 1.0f), 8.0f);
            draw_rect(context, SAMPLE_RECT, Float4U(0.91f, 0.91f, 0.91f, 1.0f), 7.0f);

            Vector<f32> shape_floats;
            build_sample_shape(shape_floats, sample);
            auto shape = context->append_sdf_shape_program(shape_floats.cspan());
            if(failed(shape)) return shape.errcode();

            const Float4U accent(0.94f, 0.16f, 0.28f, 1.0f);
            const Float4U accent_dark(0.55f, 0.02f, 0.10f, 1.0f);
            const Float4U highlight(1.0f, 0.72f, 0.76f, 1.0f);
            const Float4U accent_soft(0.98f, 0.42f, 0.50f, 0.72f);
            const Float2U center(SAMPLE_RECT.width * 0.5f, SAMPLE_RECT.height * 0.5f);
            const RectF shape_rect(17.0f, 10.0f, SAMPLE_RECT.width - 34.0f, SAMPLE_RECT.height - 20.0f);
            SDFGradientStop stops[] = {
                {0.0f, highlight, 0.42f}, {0.5f, accent, 0.58f}, {1.0f, accent_dark, 0.5f}
            };

            auto submit_color = [&](Vector<f32>& color_floats) -> RV
            {
                auto color = context->append_sdf_color_program(color_floats.cspan());
                if(failed(color)) return color.errcode();
                DrawCommand command;
                command.type = DrawCommandType::sdf;
                command.rect_reference = DrawCommandRectReference::element;
                command.rect = SAMPLE_RECT;
                command.sdf.shape = shape.get();
                command.sdf.color = color.get();
                context->draw(command);
                return ok;
            };

            Vector<f32> color_floats;
            if(sample == SDFSample::solid)
            {
                sdf_color_add_solid(color_floats, accent);
            }
            else if(sample == SDFSample::radial_gradient)
            {
                sdf_color_add_radial_gradient(color_floats, Float2U(center.x - 18.0f, center.y - 10.0f),
                    Float2U(92.0f, 38.0f), Span<const SDFGradientStop>(stops, 3));
            }
            else if(sample == SDFSample::conic_gradient)
            {
                sdf_color_add_conic_gradient(color_floats, center, -PI_DIV_TWO,
                    Span<const SDFGradientStop>(stops, 3));
            }
            else if(sample == SDFSample::bilinear_gradient)
            {
                sdf_color_add_bilinear_gradient(color_floats, shape_rect,
                    highlight, accent, accent_dark, accent_soft);
            }
            else if(sample == SDFSample::shadow)
            {
                sdf_color_add_shadow(color_floats, Float4U(0.0f, 0.0f, 0.0f, 0.42f),
                    Float2U(4.0f, 5.0f), 5.0f, 0.0f, SDFClipDesc::inner(0.0f));
                sdf_color_add_linear_gradient(color_floats, Float2U(0.0f, 0.0f),
                    Float2U(SAMPLE_RECT.width, SAMPLE_RECT.height), Span<const SDFGradientStop>(stops, 3));
                sdf_color_add_shadow(color_floats, Float4U(0.30f, 0.0f, 0.03f, 0.55f),
                    Float2U(3.0f, 3.0f), 4.0f, 1.0f, SDFClipDesc::outer(0.0f));
            }
            else if(sample == SDFSample::no_clip)
            {
                sdf_color_add_solid(color_floats, accent_soft, SDFClipDesc::no_clip());
            }
            else if(sample == SDFSample::inner_clip)
            {
                sdf_color_add_solid(color_floats, accent_soft, SDFClipDesc::inner(7.0f));
            }
            else if(sample == SDFSample::outer_clip)
            {
                sdf_color_add_solid(color_floats, accent, SDFClipDesc::outer(8.0f));
            }
            else if(sample == SDFSample::inner_outer_clip)
            {
                sdf_color_add_solid(color_floats, accent, SDFClipDesc::inner_outer(7.0f, 8.0f));
            }
            else
            {
                sdf_color_add_linear_gradient(color_floats, Float2U(0.0f, 0.0f),
                    Float2U(SAMPLE_RECT.width, SAMPLE_RECT.height), Span<const SDFGradientStop>(stops, 3));
            }

            RV color_result = submit_color(color_floats);
            if(failed(color_result)) return color_result;
            if(sample >= SDFSample::no_clip)
            {
                Vector<f32> boundary_floats;
                sdf_color_add_solid(boundary_floats, accent_dark, SDFClipDesc::stroke(1.5f));
                RV result = submit_color(boundary_floats);
                if(failed(result)) return result;
            }

            draw_outline(context, RectF(0.0f, 0.0f, TILE_WIDTH, TILE_HEIGHT),
                Float4U(0.77f, 0.77f, 0.77f, 1.0f), 1.0f);
            draw_text(context, RectF(10.0f, 6.0f, TILE_WIDTH - 20.0f, 18.0f),
                SAMPLE_TITLES[sample_index], 15.0f, Float4U(0.03f, 0.03f, 0.03f, 1.0f));
            draw_text(context, RectF(10.0f, 23.0f, TILE_WIDTH - 20.0f, 14.0f),
                SAMPLE_OPCODES[sample_index], 11.0f, Float4U(0.38f, 0.38f, 0.38f, 1.0f));
            return ok;
        }

        void build_sdf_sample(GUI::IContext* context, SDFSample sample)
        {
            GUI::ElementHandle element = context->begin_element(sample_id(sample));
            context->set_layout_config(element, fixed_layout(TILE_WIDTH, TILE_HEIGHT));
            GUI::DrawConfig draw;
            draw.name = Name("gui.test.sdf.sample");
            draw.callback = draw_sdf_sample;
            draw.userdata = (void*)(usize)(u32)sample;
            context->set_draw_config(element, draw);
            context->end_element();
        }
    }

    void add_sdf_slice_items(SheetState& state)
    {
        state.sdf_items.clear();
        for(u32 i = 0; i < (u32)SDFSample::count; ++i)
        {
            SDFSample sample = (SDFSample)i;
            u32 column;
            u32 row;
            sample_grid_position(sample, column, row);
            add_canvas_item(state.sdf_items, sample_id(sample),
                (TILE_WIDTH + TILE_GAP_X) * (f32)column, (TILE_HEIGHT + TILE_GAP_Y) * (f32)row);
        }
    }

    void build_sdf_slice(GUI::IContext* context, SheetState& state)
    {
        GUI::ElementHandle section = context->begin_element(ID_SDF);
        context->set_layout_config(section, fixed_layout(SHEET_WIDTH - 128.0f, SHEET_HEIGHT - 216.0f));
        for(u32 i = 0; i < (u32)SDFSample::count; ++i)
        {
            build_sdf_sample(context, (SDFSample)i);
        }
        state.sdf_canvas.items = Span<const GUI::CanvasLayoutItem>(state.sdf_items.data(), state.sdf_items.size());
        state.sdf_canvas.default_item = GUI::CanvasLayoutItem();
        state.sdf_canvas.clip_children = false;
        set_canvas_layout(context, section, &state.sdf_canvas);
        context->end_element();
    }
}
