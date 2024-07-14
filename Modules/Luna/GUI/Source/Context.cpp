/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Context.cpp
* @author JXMaster
* @date 2024/6/28
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "Context.hpp"
#include <Luna/Runtime/Memory.hpp>
#include <Luna/VG/TextArranger.hpp>
#include <Luna/VG/Shapes.hpp>

namespace Luna
{
    namespace GUI
    {
        static void* nk_alloc(nk_handle userdata, void* old, nk_size new_size)
        {
            void* ptr = memalloc(new_size);
#ifdef LUNA_MEMORY_PROFILER_ENABLED
            memory_profiler_set_memory_domain(ptr, "Nuklear", 7);
#endif
            return ptr;
        }
        static void nk_free(nk_handle, void* old)
        {
            memfree(old);
        }
        static float nk_calc_font_width(nk_handle handle, float height, const char *text, int len)
        {
            Context* ctx = (Context*)handle.ptr;
            VG::TextArrangeSection section;
            section.font_file = ctx->m_font_atlas->get_font(&section.font_index);
            section.num_chars = (usize)len;
            section.font_size = height;
            VG::TextArrangeResult result = VG::arrange_text(text, len, {&section, 1}, RectF(0, 0, F32_MAX, F32_MAX), VG::TextAlignment::begin, VG::TextAlignment::begin);
            return result.bounding_rect.width;
        }
        void Context::init()
        {
            memzero(&m_allocator);
            m_allocator.alloc = nk_alloc;
            m_allocator.free = nk_free;
            memzero(&m_font);
            m_font.userdata.ptr = this;
            m_font.height = 18.0f;
            m_font.width = nk_calc_font_width;
            memzero(&m_ctx);
            nk_bool ret = nk_init(&m_ctx, &m_allocator, &m_font);
            luassert_always(ret);
        }
        Context::~Context()
        {
            nk_free(&m_ctx);
        }
        static void add_scissor_rect(Context* ctx, VG::IShapeDrawList* draw_list, struct nk_command_scissor* cmd)
        {
            if(cmd->x == -8192 && cmd->y == -8192 && cmd->w == 16384 && cmd->h == 16384)
            {
                draw_list->set_clip_rect(RectF{0, 0, 0, 0});
            }
            else
            {
                draw_list->set_clip_rect(RectF{(f32)cmd->x, (f32)(ctx->m_viewport_size.y - cmd->y - cmd->h), (f32)cmd->w, (f32)cmd->h});
            }
        }
        inline Float4U encode_color(const struct nk_color& color)
        {
            return Float4U((f32)color.r / 255.0f, (f32)color.g / 255.0f, (f32)color.b / 255.0f, (f32)color.a / 255.0f);
        }
        static void add_line(Context* ctx, VG::IShapeDrawList* draw_list, struct nk_command_line* cmd)
        {
            // convert to canvas coordinates.
            f32 x1 = (f32)cmd->begin.x;
            f32 y1 = (f32)ctx->m_viewport_size.y - (f32)cmd->begin.y;
            f32 x2 = (f32)cmd->end.x;
            f32 y2 = (f32)ctx->m_viewport_size.y - (f32)cmd->end.y;
            OffsetRectF bounding_rect {
                min(x1, x2),
                max(y1, y2),
                max(x1, x2),
                min(y1, y2)
            };
            x1 -= bounding_rect.left;
            x2 -= bounding_rect.left;
            y1 -= bounding_rect.bottom;
            y2 -= bounding_rect.bottom;
            auto& points = draw_list->get_shape_buffer()->get_shape_points(true);
            u32 offset = (u32)points.size();
            VG::ShapeBuilder::add_line(points, x1, y1, x2, y2, (f32)cmd->line_thickness, 0.0f);
            u32 size = (u32)points.size() - offset;
            draw_list->draw_shape(offset, size, 
                {bounding_rect.left, bounding_rect.bottom}, {bounding_rect.right, bounding_rect.top},
                {0, 0}, {bounding_rect.right - bounding_rect.left, bounding_rect.top - bounding_rect.bottom},
                encode_color(cmd->color));
        }
        static void add_curve(Context* ctx, VG::IShapeDrawList* draw_list, struct nk_command_curve* cmd)
        {
            // not supported by VG yet. As far as I know, no built-in Nuklear widgets draw curves.
        }
        static void add_rect(Context* ctx, VG::IShapeDrawList* draw_list, struct nk_command_rect* cmd)
        {
            auto& points = draw_list->get_shape_buffer()->get_shape_points(true);
            u32 offset = (u32)points.size();
            if(cmd->rounding)
            {
                VG::ShapeBuilder::add_rounded_rectangle_bordered(points, 0, 0, cmd->w, cmd->h, cmd->rounding, cmd->line_thickness);
            }
            else
            {
                VG::ShapeBuilder::add_rectangle_bordered(points, 0, 0, cmd->w, cmd->h, cmd->line_thickness);
            }
            u32 size = (u32)points.size() - offset;
            draw_list->draw_shape(offset, size, 
                {(f32)cmd->x, (f32)ctx->m_viewport_size.y - ((f32)cmd->y + (f32)cmd->h)}, 
                {(f32)cmd->x + (f32)cmd->w, (f32)ctx->m_viewport_size.y - (f32)cmd->y},
                {0, 0}, {(f32)cmd->w, (f32)cmd->h}, encode_color(cmd->color));
        }
        static void add_rect_filled(Context* ctx, VG::IShapeDrawList* draw_list, struct nk_command_rect_filled* cmd)
        {
            auto& points = draw_list->get_shape_buffer()->get_shape_points(true);
            u32 offset = (u32)points.size();
            if(cmd->rounding)
            {
                VG::ShapeBuilder::add_rounded_rectangle_filled(points, 0, 0, cmd->w, cmd->h, cmd->rounding);
            }
            else
            {
                VG::ShapeBuilder::add_rectangle_filled(points, 0, 0, cmd->w, cmd->h);
            }
            u32 size = (u32)points.size() - offset;
            draw_list->draw_shape(offset, size, 
                {(f32)cmd->x, (f32)ctx->m_viewport_size.y - ((f32)cmd->y + (f32)cmd->h)}, 
                {(f32)cmd->x + (f32)cmd->w, (f32)ctx->m_viewport_size.y - (f32)cmd->y},
                {0, 0}, {(f32)cmd->w, (f32)cmd->h}, encode_color(cmd->color));
        }
        static void add_rect_multi_color(Context* ctx, VG::IShapeDrawList* draw_list, struct nk_command_rect_multi_color* cmd)
        {
            auto& points = draw_list->get_shape_buffer()->get_shape_points(true);
            u32 offset = (u32)points.size();
            VG::ShapeBuilder::add_rectangle_filled(points, 0, 0, cmd->w, cmd->h);
            u32 size = (u32)points.size() - offset;
            Float2 min_position {(f32)cmd->x, (f32)ctx->m_viewport_size.y - ((f32)cmd->y + (f32)cmd->h)};
            Float2 max_position {(f32)cmd->x + (f32)cmd->w, (f32)ctx->m_viewport_size.y - (f32)cmd->y};
            VG::Vertex v[4];
            v[0].position = min_position;
            v[1].position.x = min_position.x;
            v[1].position.y = max_position.y;
            v[2].position = max_position;
            v[3].position.x = max_position.x;
            v[3].position.y = min_position.y;
            Float2 min_shapecoord = {0, 0};
            Float2 max_shapecoord = {(f32)cmd->w, (f32)cmd->h};
            v[0].shapecoord = min_shapecoord;
            v[1].shapecoord.x = min_shapecoord.x;
            v[1].shapecoord.y = max_shapecoord.y;
            v[2].shapecoord = max_shapecoord;
            v[3].shapecoord.x = max_shapecoord.x;
            v[3].shapecoord.y = min_shapecoord.y;
            v[0].texcoord = Float2U{0, 0};
            v[1].texcoord = Float2U{0, 0};
            v[2].texcoord = Float2U{0, 0};
            v[3].texcoord = Float2U{0, 0};
            v[0].color = encode_color(cmd->bottom);
            v[1].color = encode_color(cmd->left);
            v[2].color = encode_color(cmd->top);
            v[3].color = encode_color(cmd->right);
            v[0].begin_command = v[1].begin_command = v[2].begin_command = v[3].begin_command = offset;
            v[0].num_commands = v[1].num_commands = v[2].num_commands = v[3].num_commands = size;
            u32 indices[] = {0, 1, 2, 0, 2, 3};
            draw_list->draw_shape_raw({v, 4}, {indices, 6});
        }
        static void add_triangle(Context* ctx, VG::IShapeDrawList* draw_list, struct nk_command_triangle* cmd)
        {
            f32 x1 = (f32)cmd->a.x;
            f32 y1 = (f32)ctx->m_viewport_size.y - (f32)cmd->a.y;
            f32 x2 = (f32)cmd->b.x;
            f32 y2 = (f32)ctx->m_viewport_size.y - (f32)cmd->b.y;
            f32 x3 = (f32)cmd->c.x;
            f32 y3 = (f32)ctx->m_viewport_size.y - (f32)cmd->c.y;
            OffsetRectF bounding_rect {
                min(min(x1, x2), x3),
                max(max(y1, y2), y3),
                max(max(x1, x2), x3),
                min(min(y1, y2), y3)
            };
            auto& points = draw_list->get_shape_buffer()->get_shape_points(true);
            u32 offset = (u32)points.size();
            VG::ShapeBuilder::add_triangle_bordered(points, x1, y1, x2, y2, x3, y3, cmd->line_thickness);
            u32 size = (u32)points.size() - offset;
            draw_list->draw_shape(offset, size, 
                {bounding_rect.left, bounding_rect.bottom}, {bounding_rect.right, bounding_rect.top},
                {0, 0}, {bounding_rect.right - bounding_rect.left, bounding_rect.top - bounding_rect.bottom},
                encode_color(cmd->color));
        }
        static void add_triangle_filled(Context* ctx, VG::IShapeDrawList* draw_list, struct nk_command_triangle_filled* cmd)
        {
            f32 x1 = (f32)cmd->a.x;
            f32 y1 = (f32)ctx->m_viewport_size.y - (f32)cmd->a.y;
            f32 x2 = (f32)cmd->b.x;
            f32 y2 = (f32)ctx->m_viewport_size.y - (f32)cmd->b.y;
            f32 x3 = (f32)cmd->c.x;
            f32 y3 = (f32)ctx->m_viewport_size.y - (f32)cmd->c.y;
            OffsetRectF bounding_rect {
                min(min(x1, x2), x3),
                max(max(y1, y2), y3),
                max(max(x1, x2), x3),
                min(min(y1, y2), y3)
            };
            auto& points = draw_list->get_shape_buffer()->get_shape_points(true);
            u32 offset = (u32)points.size();
            VG::ShapeBuilder::add_triangle_filled(points, x1, y1, x2, y2, x3, y3);
            u32 size = (u32)points.size() - offset;
            draw_list->draw_shape(offset, size, 
                {bounding_rect.left, bounding_rect.bottom}, {bounding_rect.right, bounding_rect.top},
                {0, 0}, {bounding_rect.right - bounding_rect.left, bounding_rect.top - bounding_rect.bottom},
                encode_color(cmd->color));
        }
        static void add_circle(Context* ctx, VG::IShapeDrawList* draw_list, struct nk_command_circle* cmd)
        {
            auto& points = draw_list->get_shape_buffer()->get_shape_points(true);
            u32 offset = (u32)points.size();
            f32 center_x = ((f32)cmd->w) / 2.0f;
            f32 center_y = ((f32)cmd->h) / 2.0f;
            VG::ShapeBuilder::add_axis_aligned_ellipse_bordered(points, center_x, center_y, center_x, center_y, cmd->line_thickness);
            u32 size = (u32)points.size() - offset;
            draw_list->draw_shape(offset, size, 
                {(f32)cmd->x, (f32)ctx->m_viewport_size.y - ((f32)cmd->y + (f32)cmd->h)}, 
                {(f32)cmd->x + (f32)cmd->w, (f32)ctx->m_viewport_size.y - (f32)cmd->y},
                {0, 0}, {(f32)cmd->w, (f32)cmd->h}, encode_color(cmd->color));
        }
        static void add_circle_filled(Context* ctx, VG::IShapeDrawList* draw_list, struct nk_command_circle_filled* cmd)
        {
            auto& points = draw_list->get_shape_buffer()->get_shape_points(true);
            u32 offset = (u32)points.size();
            f32 center_x = ((f32)cmd->w) / 2.0f;
            f32 center_y = ((f32)cmd->h) / 2.0f;
            VG::ShapeBuilder::add_axis_aligned_ellipse_filled(points, center_x, center_y, center_x, center_y);
            u32 size = (u32)points.size() - offset;
            draw_list->draw_shape(offset, size, 
                {(f32)cmd->x, (f32)ctx->m_viewport_size.y - ((f32)cmd->y + (f32)cmd->h)}, 
                {(f32)cmd->x + (f32)cmd->w, (f32)ctx->m_viewport_size.y - (f32)cmd->y},
                {0, 0}, {(f32)cmd->w, (f32)cmd->h}, encode_color(cmd->color));
        }
        static void add_arc(Context* ctx, VG::IShapeDrawList* draw_list, struct nk_command_arc* cmd)
        {
            auto& points = draw_list->get_shape_buffer()->get_shape_points(true);
            u32 offset = (u32)points.size();
            VG::ShapeBuilder::add_arc_bordered(points, 0, 0, (f32)cmd->r, rad_to_deg(cmd->a[0]), rad_to_deg(cmd->a[1]), (f32)cmd->line_thickness);
            u32 size = (u32)points.size() - offset;
            draw_list->draw_shape(offset, size, 
                {(f32)cmd->cx - (f32)cmd->r, (f32)ctx->m_viewport_size.y - ((f32)cmd->cy + (f32)cmd->r)}, 
                {(f32)cmd->cx + (f32)cmd->r, (f32)ctx->m_viewport_size.y - ((f32)cmd->cy - (f32)cmd->r)},
                {-(f32)cmd->r, -(f32)cmd->r}, {(f32)cmd->r, (f32)cmd->r}, encode_color(cmd->color));
        }
        static void add_arc_filled(Context* ctx, VG::IShapeDrawList* draw_list, struct nk_command_arc_filled* cmd)
        {
            auto& points = draw_list->get_shape_buffer()->get_shape_points(true);
            u32 offset = (u32)points.size();
            VG::ShapeBuilder::add_arc_filled(points, 0, 0, (f32)cmd->r, rad_to_deg(cmd->a[0]), rad_to_deg(cmd->a[1]));
            u32 size = (u32)points.size() - offset;
            draw_list->draw_shape(offset, size, 
                {(f32)cmd->cx - (f32)cmd->r, (f32)ctx->m_viewport_size.y - ((f32)cmd->cy + (f32)cmd->r)}, 
                {(f32)cmd->cx + (f32)cmd->r, (f32)ctx->m_viewport_size.y - ((f32)cmd->cy - (f32)cmd->r)},
                {-(f32)cmd->r, -(f32)cmd->r}, {(f32)cmd->r, (f32)cmd->r}, encode_color(cmd->color));
        }
        static void add_polygon(Context* ctx, VG::IShapeDrawList* draw_list, struct nk_command_polygon* cmd)
        {
            auto& points = draw_list->get_shape_buffer()->get_shape_points(true);
            Array<Float2U> draw_points(cmd->point_count);
            OffsetRectF bounding_rect(
                cmd->points[0].x, (ctx->m_viewport_size.y - cmd->points[0].y), 
                cmd->points[0].x, (ctx->m_viewport_size.y - cmd->points[0].y));
            for(u32 i = 1; i < cmd->point_count; ++i)
            {
                bounding_rect.left = min<f32>(bounding_rect.left, cmd->points[i].x);
                bounding_rect.top = max<f32>(bounding_rect.top, (ctx->m_viewport_size.y - cmd->points[i].y));
                bounding_rect.right = max<f32>(bounding_rect.right, cmd->points[i].x);
                bounding_rect.bottom = min<f32>(bounding_rect.bottom, (ctx->m_viewport_size.y - cmd->points[i].y));
            }
            Array<Float2U> vertices(cmd->point_count);
            for(u32 i = 0; i < cmd->point_count; ++i)
            {
                vertices[i] = Float2U(cmd->points[i].x - bounding_rect.left, 
                    (ctx->m_viewport_size.y - cmd->points[i].y) - bounding_rect.bottom);
            }
            u32 offset = (u32)points.size();
            VG::ShapeBuilder::add_polygon_bordered(points, vertices.cspan(), cmd->line_thickness);
            u32 size = (u32)points.size() - offset;
            draw_list->draw_shape(offset, size, 
                {bounding_rect.left, bounding_rect.bottom}, {bounding_rect.right, bounding_rect.top},
                {0, 0}, {bounding_rect.right - bounding_rect.left, bounding_rect.top - bounding_rect.bottom},
                encode_color(cmd->color));
        }
        static void add_polygon_filled(Context* ctx, VG::IShapeDrawList* draw_list, struct nk_command_polygon_filled* cmd)
        {
            auto& points = draw_list->get_shape_buffer()->get_shape_points(true);
            Array<Float2U> draw_points(cmd->point_count);
            OffsetRectF bounding_rect(
                cmd->points[0].x, (ctx->m_viewport_size.y - cmd->points[0].y), 
                cmd->points[0].x, (ctx->m_viewport_size.y - cmd->points[0].y));
            for(u32 i = 1; i < cmd->point_count; ++i)
            {
                bounding_rect.left = min<f32>(bounding_rect.left, cmd->points[i].x);
                bounding_rect.top = max<f32>(bounding_rect.top, (ctx->m_viewport_size.y - cmd->points[i].y));
                bounding_rect.right = max<f32>(bounding_rect.right, cmd->points[i].x);
                bounding_rect.bottom = min<f32>(bounding_rect.bottom, (ctx->m_viewport_size.y - cmd->points[i].y));
            }
            Array<Float2U> vertices(cmd->point_count);
            for(u32 i = 0; i < cmd->point_count; ++i)
            {
                vertices[i] = Float2U(cmd->points[i].x - bounding_rect.left, 
                    (ctx->m_viewport_size.y - cmd->points[i].y) - bounding_rect.bottom);
            }
            u32 offset = (u32)points.size();
            VG::ShapeBuilder::add_polygon_filled(points, vertices.cspan());
            u32 size = (u32)points.size() - offset;
            draw_list->draw_shape(offset, size, 
                {bounding_rect.left, bounding_rect.bottom}, {bounding_rect.right, bounding_rect.top},
                {0, 0}, {bounding_rect.right - bounding_rect.left, bounding_rect.top - bounding_rect.bottom},
                encode_color(cmd->color));
        }
        static void add_polyline(Context* ctx, VG::IShapeDrawList* draw_list, struct nk_command_polyline* cmd)
        {
            auto& points = draw_list->get_shape_buffer()->get_shape_points(true);
            Array<Float2U> draw_points(cmd->point_count);
            OffsetRectF bounding_rect(
                cmd->points[0].x, (ctx->m_viewport_size.y - cmd->points[0].y), 
                cmd->points[0].x, (ctx->m_viewport_size.y - cmd->points[0].y));
            for(u32 i = 1; i < cmd->point_count; ++i)
            {
                bounding_rect.left = min<f32>(bounding_rect.left, cmd->points[i].x);
                bounding_rect.top = max<f32>(bounding_rect.top, (ctx->m_viewport_size.y - cmd->points[i].y));
                bounding_rect.right = max<f32>(bounding_rect.right, cmd->points[i].x);
                bounding_rect.bottom = min<f32>(bounding_rect.bottom, (ctx->m_viewport_size.y - cmd->points[i].y));
            }
            Array<Float2U> vertices(cmd->point_count);
            for(u32 i = 0; i < cmd->point_count; ++i)
            {
                vertices[i] = Float2U(cmd->points[i].x - bounding_rect.left, 
                    (ctx->m_viewport_size.y - cmd->points[i].y) - bounding_rect.bottom);
            }
            u32 offset = (u32)points.size();
            VG::ShapeBuilder::add_polyline(points, vertices.cspan(), cmd->line_thickness);
            u32 size = (u32)points.size() - offset;
            draw_list->draw_shape(offset, size, 
                {bounding_rect.left, bounding_rect.bottom}, {bounding_rect.right, bounding_rect.top},
                {0, 0}, {bounding_rect.right - bounding_rect.left, bounding_rect.top - bounding_rect.bottom},
                encode_color(cmd->color));
        }
        static void add_text(Context* ctx, VG::IShapeDrawList* draw_list, struct nk_command_text* cmd)
        {
            VG::TextArrangeSection section;
            section.font_file = ctx->m_font_atlas->get_font(&section.font_index);
            section.num_chars = cmd->length;
            section.color = encode_color(cmd->foreground);
            section.font_size = cmd->height;
            VG::TextArrangeResult result = VG::arrange_text(
                cmd->string, cmd->length,
                {&section, 1},
                RectF((f32)cmd->x, (ctx->m_viewport_size.y - (f32)cmd->y - (f32)cmd->height), F32_MAX, F32_MAX),
                VG::TextAlignment::end,
                VG::TextAlignment::begin
            );
            if (cmd->background.a > 0.0f)
            {
                auto& points = draw_list->get_shape_buffer()->get_shape_points(true);
                // draw text background.
                u32 offset = (u32)points.size();
                VG::ShapeBuilder::add_rectangle_filled(points, 0, 0, (f32)cmd->w, (f32)cmd->height);
                u32 size = (u32)points.size() - offset;
                draw_list->draw_shape(offset, size, 
                        {(f32)cmd->x, (ctx->m_viewport_size.y - (f32)cmd->y - (f32)cmd->height)}, 
                        {(f32)cmd->x + (f32)cmd->w, (ctx->m_viewport_size.y - (f32)cmd->y)},
                        {0, 0}, {(f32)cmd->w, (f32)cmd->h}, 
                        encode_color(cmd->background));
            }
            VG::commit_text_arrange_result(result, {&section, 1}, ctx->m_font_atlas, draw_list);
        }
        static void add_image(Context* ctx, VG::IShapeDrawList* draw_list, struct nk_command_image* cmd)
        {
            // draw a textured rectangle.
            auto& points = draw_list->get_shape_buffer()->get_shape_points(true);
            u32 offset = (u32)points.size();
            VG::ShapeBuilder::add_rectangle_filled(points, 0, 0, cmd->w, cmd->h);
            u32 size = (u32)points.size() - offset;

            RHI::ITexture* texture = (RHI::ITexture*)cmd->img.handle.ptr;

            f32 min_texcoord_x = (f32)cmd->img.region[0] / (f32)cmd->img.w;
            f32 min_texcoord_y = (f32)cmd->img.region[1] / (f32)cmd->img.h;
            f32 max_texcoord_x = (f32)(cmd->img.w - cmd->img.region[2]) / (f32)cmd->img.w;
            f32 max_texcoord_y = (f32)(cmd->img.h - cmd->img.region[3]) / (f32)cmd->img.h;

            draw_list->set_texture(texture);
            draw_list->draw_shape(offset, size, 
                {(f32)cmd->x, (f32)ctx->m_viewport_size.y - ((f32)cmd->y + (f32)cmd->h)}, 
                {(f32)cmd->x + (f32)cmd->w, (f32)ctx->m_viewport_size.y - (f32)cmd->y},
                {0, 0}, {(f32)cmd->w, (f32)cmd->h}, encode_color(cmd->col), 
                {min_texcoord_x, min_texcoord_y}, {max_texcoord_x, max_texcoord_y});
            draw_list->set_texture(nullptr);
        }
        void Context::render(VG::IShapeDrawList* draw_list)
        {
            const struct nk_command *cmd = 0;
            nk_foreach(cmd, &m_ctx)
            {
                switch(cmd->type)
                {
                    case NK_COMMAND_NOP: 
                    case NK_COMMAND_CUSTOM: break;
                    case NK_COMMAND_SCISSOR:
                    add_scissor_rect(this, draw_list, (struct nk_command_scissor*)cmd); break;
                    case NK_COMMAND_LINE:
                    add_line(this, draw_list, (struct nk_command_line*)cmd); break;
                    case NK_COMMAND_CURVE:
                    add_curve(this, draw_list, (struct nk_command_curve*)cmd); break;
                    case NK_COMMAND_RECT:
                    add_rect(this, draw_list, (struct nk_command_rect*)cmd); break;
                    case NK_COMMAND_RECT_FILLED:
                    add_rect_filled(this, draw_list, (struct nk_command_rect_filled*)cmd); break;
                    case NK_COMMAND_RECT_MULTI_COLOR:
                    add_rect_multi_color(this, draw_list, (struct nk_command_rect_multi_color*)cmd); break;
                    case NK_COMMAND_CIRCLE:
                    add_circle(this, draw_list, (struct nk_command_circle*)cmd); break;
                    case NK_COMMAND_CIRCLE_FILLED:
                    add_circle_filled(this, draw_list, (struct nk_command_circle_filled*)cmd); break;
                    case NK_COMMAND_ARC:
                    add_arc(this, draw_list, (struct nk_command_arc*)cmd); break;
                    case NK_COMMAND_ARC_FILLED:
                    add_arc_filled(this, draw_list, (struct nk_command_arc_filled*)cmd); break;
                    case NK_COMMAND_TRIANGLE:
                    add_triangle(this, draw_list, (struct nk_command_triangle*)cmd); break;
                    case NK_COMMAND_TRIANGLE_FILLED:
                    add_triangle_filled(this, draw_list, (struct nk_command_triangle_filled*)cmd); break;
                    case NK_COMMAND_POLYGON:
                    add_polygon(this, draw_list, (struct nk_command_polygon*)cmd); break;
                    case NK_COMMAND_POLYGON_FILLED:
                    add_polygon_filled(this, draw_list, (struct nk_command_polygon_filled*)cmd); break;
                    case NK_COMMAND_POLYLINE:
                    add_polyline(this, draw_list, (struct nk_command_polyline*)cmd); break;
                    case NK_COMMAND_TEXT:
                    add_text(this, draw_list, (struct nk_command_text*)cmd); break;
                    case NK_COMMAND_IMAGE:
                    add_image(this, draw_list, (struct nk_command_image*)cmd); break;
                    break;
                }
            }
        }
        LUNA_GUI_API Ref<IContext> new_context(VG::IFontAtlas* font_atlas)
        {
            Ref<Context> ctx = new_object<Context>();
            ctx->m_font_atlas = font_atlas;
            ctx->init();
            return ctx;
        }
    }
}