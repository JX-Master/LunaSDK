/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file BlurSection.cpp
* @author JXMaster
* @date 2026/7/30
*/
#include "GUITest.hpp"

namespace Luna::GUITest
{
    namespace
    {
        constexpr f32 SECTION_WIDTH = SHEET_WIDTH - 128.0f;
        constexpr f32 SECTION_HEIGHT = SHEET_HEIGHT - 216.0f;
        constexpr f32 PANEL_WIDTH = 284.0f;
        constexpr f32 PANEL_HEIGHT = 241.0f;
        constexpr f32 PANEL_RADIUS = 24.0f;

        struct GlassMaterialSample
        {
            const c8* title;
            const c8* caption;
            const c8* parameters;
            f32 softness;
            u8 downsample_level;
            f32 grain;
            Float4U tint;
            Float4U border;
            Float4U light;
        };

        const GlassMaterialSample GLASS_MATERIALS[] = {
            {
                "Clear glass",
                "High transmission · crisp edges",
                "B 9  ·  T .94  ·  R .30  ·  G .04",
                9.0f, 0, 0.04f,
                Float4U(0.86f, 0.96f, 1.0f, 0.10f),
                Float4U(0.88f, 0.97f, 1.0f, 0.72f),
                Float4U(1.0f, 1.0f, 1.0f, 0.58f)
            },
            {
                "Acrylic",
                "Cool white · even diffusion",
                "B 20  ·  T .66  ·  R .42  ·  G .10",
                20.0f, 1, 0.10f,
                Float4U(0.74f, 0.88f, 1.0f, 0.25f),
                Float4U(0.82f, 0.94f, 1.0f, 0.66f),
                Float4U(1.0f, 1.0f, 1.0f, 0.54f)
            },
            {
                "Mica",
                "Warm tint · diagonal highlight",
                "B 15  ·  T .74  ·  R .62  ·  G .18",
                15.0f, 1, 0.18f,
                Float4U(1.0f, 0.70f, 0.30f, 0.24f),
                Float4U(1.0f, 0.88f, 0.62f, 0.68f),
                Float4U(1.0f, 0.86f, 0.56f, 0.62f)
            },
            {
                "Laminated",
                "Blue interlayer · clean contour",
                "B 18  ·  T .78  ·  R .52  ·  G .07",
                18.0f, 1, 0.07f,
                Float4U(0.40f, 0.76f, 1.0f, 0.22f),
                Float4U(0.68f, 0.90f, 1.0f, 0.68f),
                Float4U(0.76f, 0.94f, 1.0f, 0.58f)
            },
            {
                "Grain frost",
                "Dense diffusion · fine texture",
                "B 38  ·  T .42  ·  R .36  ·  G .56",
                38.0f, 2, 0.56f,
                Float4U(0.88f, 0.94f, 1.0f, 0.40f),
                Float4U(0.94f, 0.98f, 1.0f, 0.72f),
                Float4U(1.0f, 1.0f, 1.0f, 0.56f)
            },
            {
                "Smoked glass",
                "Deep tone · restrained contrast",
                "B 25  ·  T .52  ·  R .48  ·  G .16",
                25.0f, 1, 0.16f,
                Float4U(0.08f, 0.10f, 0.14f, 0.56f),
                Float4U(0.60f, 0.68f, 0.78f, 0.54f),
                Float4U(0.86f, 0.94f, 1.0f, 0.46f)
            },
            {
                "Ice frost",
                "Cold haze · thick medium",
                "B 54  ·  T .32  ·  R .40  ·  G .34",
                54.0f, 2, 0.34f,
                Float4U(0.66f, 0.84f, 1.0f, 0.48f),
                Float4U(0.82f, 0.94f, 1.0f, 0.74f),
                Float4U(0.72f, 0.90f, 1.0f, 0.64f)
            },
            {
                "Champagne resin",
                "Warm resin · saturated color",
                "B 30  ·  T .46  ·  R .68  ·  G .24",
                30.0f, 1, 0.24f,
                Float4U(1.0f, 0.46f, 0.10f, 0.36f),
                Float4U(1.0f, 0.78f, 0.42f, 0.72f),
                Float4U(1.0f, 0.78f, 0.38f, 0.68f)
            }
        };

        void draw_backdrop(GUI::IContext* context, const RectF& rect, f32 radius)
        {
            GUI::DrawCommand command;
            command.type = GUI::DrawCommandType::backdrop_blur;
            command.rect_reference = GUI::DrawCommandRectReference::element;
            command.rect = rect;
            command.radius = radius;
            context->draw(command);
        }

        RV draw_animated_background(GUI::IContext* context,
            const GUI::ElementHandle&, GUI::DrawPhase, void* userdata)
        {
            SheetState* state = (SheetState*)userdata;
            if(!state) return E_BAD_ARGUMENTS;
            f32 time = state->animation_time;

            draw_rect(context, RectF(0.0f, 0.0f, SECTION_WIDTH, SECTION_HEIGHT),
                Float4U(0.095f, 0.106f, 0.132f, 1.0f));

            struct BackdropShape
            {
                RectF rect;
                Float2U travel;
                Float4U color;
                f32 phase;
                f32 speed;
                bool circle;
            };
            const BackdropShape shapes[] = {
                {RectF(42.0f, 62.0f, 214.0f, 120.0f), Float2U(34.0f, 22.0f),
                    Float4U(1.0f, 0.05f, 0.16f, 1.0f), 0.15f, 0.86f, false},
                {RectF(192.0f, 116.0f, 152.0f, 152.0f), Float2U(46.0f, 34.0f),
                    Float4U(0.0f, 0.66f, 1.0f, 1.0f), 1.30f, 0.72f, true},
                {RectF(316.0f, 44.0f, 188.0f, 104.0f), Float2U(28.0f, 42.0f),
                    Float4U(1.0f, 0.74f, 0.0f, 1.0f), 2.20f, 0.95f, false},
                {RectF(358.0f, 232.0f, 132.0f, 132.0f), Float2U(52.0f, 24.0f),
                    Float4U(1.0f, 0.86f, 0.0f, 1.0f), 3.75f, 0.68f, true},
                {RectF(520.0f, 132.0f, 168.0f, 120.0f), Float2U(38.0f, 30.0f),
                    Float4U(1.0f, 0.30f, 0.0f, 1.0f), 4.80f, 1.08f, false},
                {RectF(626.0f, 56.0f, 184.0f, 184.0f), Float2U(48.0f, 38.0f),
                    Float4U(0.08f, 0.94f, 0.24f, 1.0f), 5.70f, 0.78f, true},
                {RectF(822.0f, 42.0f, 152.0f, 190.0f), Float2U(32.0f, 44.0f),
                    Float4U(0.66f, 0.08f, 1.0f, 1.0f), 6.60f, 0.84f, false},
                {RectF(950.0f, 96.0f, 164.0f, 164.0f), Float2U(54.0f, 30.0f),
                    Float4U(0.02f, 0.86f, 0.74f, 1.0f), 7.90f, 0.64f, true},
                {RectF(74.0f, 326.0f, 176.0f, 126.0f), Float2U(42.0f, 26.0f),
                    Float4U(0.0f, 0.46f, 1.0f, 1.0f), 8.40f, 0.92f, false},
                {RectF(226.0f, 428.0f, 186.0f, 112.0f), Float2U(36.0f, 36.0f),
                    Float4U(1.0f, 0.42f, 0.0f, 1.0f), 9.30f, 0.74f, false},
                {RectF(382.0f, 378.0f, 208.0f, 208.0f), Float2U(56.0f, 34.0f),
                    Float4U(0.0f, 0.84f, 0.76f, 1.0f), 10.40f, 0.70f, true},
                {RectF(606.0f, 314.0f, 172.0f, 124.0f), Float2U(30.0f, 48.0f),
                    Float4U(1.0f, 0.0f, 0.48f, 1.0f), 11.10f, 1.04f, false},
                {RectF(662.0f, 410.0f, 176.0f, 176.0f), Float2U(50.0f, 28.0f),
                    Float4U(0.98f, 0.10f, 0.72f, 1.0f), 12.30f, 0.80f, true},
                {RectF(844.0f, 368.0f, 208.0f, 156.0f), Float2U(38.0f, 44.0f),
                    Float4U(1.0f, 0.86f, 0.02f, 1.0f), 13.20f, 0.88f, false},
                {RectF(916.0f, 362.0f, 216.0f, 216.0f), Float2U(58.0f, 32.0f),
                    Float4U(1.0f, 0.0f, 0.52f, 1.0f), 14.45f, 0.62f, true},
                {RectF(1030.0f, 280.0f, 124.0f, 154.0f), Float2U(26.0f, 50.0f),
                    Float4U(0.14f, 0.96f, 0.34f, 1.0f), 15.20f, 0.98f, false}
            };
            for(const BackdropShape& shape : shapes)
            {
                f32 x = shape.rect.offset_x +
                    sin(time * shape.speed + shape.phase) * shape.travel.x;
                f32 y = shape.rect.offset_y +
                    cos(time * shape.speed * 0.73f + shape.phase * 1.41f) *
                    shape.travel.y;
                draw_rect(context, RectF(x, y, shape.rect.width, shape.rect.height),
                    shape.color, shape.circle ? shape.rect.width * 0.5f : 0.0f);
            }
            return ok;
        }

        RV draw_glass_panel(GUI::IContext* context,
            const GUI::ElementHandle&, GUI::DrawPhase, void* userdata)
        {
            const GlassMaterialSample* material =
                (const GlassMaterialSample*)userdata;
            if(!material) return E_BAD_ARGUMENTS;

            GUI::ShadowDesc shadow;
            shadow.offset = Float2U(0.0f, 10.0f);
            shadow.softness = 18.0f;
            shadow.spread = -2.0f;
            draw_shadow(context, RectF(0.0f, 0.0f, PANEL_WIDTH, PANEL_HEIGHT),
                Float4U(0.0f, 0.0f, 0.0f, 0.28f), PANEL_RADIUS, shadow);
            draw_rect(context, RectF(0.0f, 0.0f, PANEL_WIDTH, PANEL_HEIGHT),
                material->border, PANEL_RADIUS);
            draw_backdrop(context, RectF(1.0f, 1.0f,
                PANEL_WIDTH - 2.0f, PANEL_HEIGHT - 2.0f),
                PANEL_RADIUS - 1.0f);
            draw_rect(context, RectF(1.0f, 1.0f,
                PANEL_WIDTH - 2.0f, PANEL_HEIGHT - 2.0f),
                material->tint, PANEL_RADIUS - 1.0f);

            u32 grain_count = (u32)(material->grain * 32.0f);
            for(u32 i = 0; i < grain_count; ++i)
            {
                u32 hash = i * 747796405u + (u32)material->softness * 2891336453u;
                f32 x = 12.0f + (f32)(hash % 260u);
                f32 y = 12.0f + (f32)((hash >> 9u) % 132u);
                f32 alpha = 0.025f + (f32)(hash & 3u) * 0.012f;
                draw_rect(context, RectF(x, y, 1.5f, 1.5f),
                    Float4U(1.0f, 1.0f, 1.0f, alpha), 0.75f);
            }

            draw_rect(context, RectF(12.0f, 142.0f, PANEL_WIDTH - 24.0f, 87.0f),
                Float4U(0.015f, 0.025f, 0.045f, 0.24f), 16.0f);
            draw_text(context, RectF(24.0f, 151.0f, PANEL_WIDTH - 48.0f, 34.0f),
                material->title, 25.0f, Float4U(1.0f, 1.0f, 1.0f, 1.0f));
            draw_text(context, RectF(24.0f, 183.0f, PANEL_WIDTH - 48.0f, 24.0f),
                material->caption, 14.0f, Float4U(0.94f, 0.97f, 1.0f, 0.84f));
            draw_text(context, RectF(24.0f, 208.0f, PANEL_WIDTH - 48.0f, 18.0f),
                material->parameters, 11.5f,
                Float4U(0.90f, 0.95f, 1.0f, 0.72f));
            return ok;
        }

        void build_glass_panel(GUI::IContext* context, GUI::id_t id,
            const GlassMaterialSample& material)
        {
            GUI::ElementHandle panel = context->begin_element(id);
            context->set_layout_config(panel,
                fixed_layout(PANEL_WIDTH, PANEL_HEIGHT));
            GUI::BackdropBlurCaptureDesc capture;
            capture.softness = material.softness;
            capture.downsample_level = material.downsample_level;
            context->set_backdrop_blur_capture(panel, capture);
            GUI::DrawConfig draw;
            draw.name = Name("gui.test.blur.material");
            draw.callback = draw_glass_panel;
            draw.userdata = (void*)&material;
            context->set_draw_config(panel, draw);
            context->end_element();
        }
    }

    void add_blur_slice_items(SheetState& state)
    {
        state.blur_items.clear();
        add_canvas_item(state.blur_items, ID_BLUR_BACKGROUND, 0.0f, 0.0f);
        add_canvas_item(state.blur_items, ID_BLUR_CLEAR, 24.0f, 24.0f);
        add_canvas_item(state.blur_items, ID_BLUR_ACRYLIC, 326.0f, 24.0f);
        add_canvas_item(state.blur_items, ID_BLUR_MICA, 628.0f, 24.0f);
        add_canvas_item(state.blur_items, ID_BLUR_LAMINATED, 930.0f, 24.0f);
        add_canvas_item(state.blur_items, ID_BLUR_GRAIN_FROST, 24.0f, 287.0f);
        add_canvas_item(state.blur_items, ID_BLUR_SMOKED, 326.0f, 287.0f);
        add_canvas_item(state.blur_items, ID_BLUR_ICE_FROST, 628.0f, 287.0f);
        add_canvas_item(state.blur_items, ID_BLUR_CHAMPAGNE, 930.0f, 287.0f);
    }

    void build_blur_slice(GUI::IContext* context, SheetState& state)
    {
        GUI::ElementHandle section = context->begin_element(ID_BLUR);
        context->set_layout_config(section,
            fixed_layout(SECTION_WIDTH, SECTION_HEIGHT));

        GUI::ElementHandle background =
            context->begin_element(ID_BLUR_BACKGROUND);
        context->set_layout_config(background,
            fixed_layout(SECTION_WIDTH, SECTION_HEIGHT));
        GUI::DrawConfig background_draw;
        background_draw.name = Name("gui.test.blur.background");
        background_draw.callback = draw_animated_background;
        background_draw.userdata = &state;
        context->set_draw_config(background, background_draw);
        context->end_element();

        build_glass_panel(context, ID_BLUR_CLEAR, GLASS_MATERIALS[0]);
        build_glass_panel(context, ID_BLUR_ACRYLIC, GLASS_MATERIALS[1]);
        build_glass_panel(context, ID_BLUR_MICA, GLASS_MATERIALS[2]);
        build_glass_panel(context, ID_BLUR_LAMINATED, GLASS_MATERIALS[3]);
        build_glass_panel(context, ID_BLUR_GRAIN_FROST, GLASS_MATERIALS[4]);
        build_glass_panel(context, ID_BLUR_SMOKED, GLASS_MATERIALS[5]);
        build_glass_panel(context, ID_BLUR_ICE_FROST, GLASS_MATERIALS[6]);
        build_glass_panel(context, ID_BLUR_CHAMPAGNE, GLASS_MATERIALS[7]);

        state.blur_canvas.items = Span<const GUI::CanvasLayoutItem>(
            state.blur_items.data(), state.blur_items.size());
        state.blur_canvas.default_item = GUI::CanvasLayoutItem();
        state.blur_canvas.clip_children = true;
        set_canvas_layout(context, section, &state.blur_canvas);
        context->end_element();
    }
}
