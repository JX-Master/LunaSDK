#pragma once

#include "../Runtime/Runtime.h"
#include "../RHI/RHI.h"
#include "../Font/Font.h"

#include <stdint.h>

#if defined(_WIN32)
#define LUNA_VG_C_API __declspec(dllexport)
#else
#define LUNA_VG_C_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    LUNA_VG_TEXT_ALIGNMENT_BEGIN = 1,
    LUNA_VG_TEXT_ALIGNMENT_CENTER = 2,
    LUNA_VG_TEXT_ALIGNMENT_END = 3
};

typedef struct LunaVGShapeBufferHandle
{
    luna_handle_t object;
    void* ishape_buffer;
} LunaVGShapeBufferHandle;

typedef struct LunaVGFontAtlasHandle
{
    luna_handle_t object;
    void* ifont_atlas;
} LunaVGFontAtlasHandle;

typedef struct LunaVGShapeDrawListHandle
{
    luna_handle_t object;
    void* ishape_draw_list;
} LunaVGShapeDrawListHandle;

typedef struct LunaVGShapeRendererHandle
{
    luna_handle_t object;
    void* ishape_renderer;
} LunaVGShapeRendererHandle;

typedef struct LunaVGFloat2
{
    float x;
    float y;
} LunaVGFloat2;

typedef struct LunaVGFloat4
{
    float x;
    float y;
    float z;
    float w;
} LunaVGFloat4;

typedef struct LunaVGRectF
{
    float offset_x;
    float offset_y;
    float width;
    float height;
} LunaVGRectF;

typedef struct LunaVGMatrix4x4
{
    LunaVGFloat4 rows[4];
} LunaVGMatrix4x4;

typedef struct LunaVGVertex
{
    LunaVGFloat2 position;
    LunaVGFloat2 shapecoord;
    LunaVGFloat2 texcoord;
    uint32_t begin_command;
    uint32_t num_commands;
    LunaVGFloat4 color;
} LunaVGVertex;

typedef struct LunaVGShapeDrawCall
{
    LunaRhiBufferHandle shape_buffer;
    LunaRhiTextureHandle texture;
    LunaRhiSamplerDesc sampler;
    LunaVGRectF clip_rect;
    uint32_t base_index;
    uint32_t num_indices;
    LunaVGMatrix4x4 transform;
} LunaVGShapeDrawCall;

typedef struct LunaVGTextArrangeSection
{
    void* font_file;
    uint64_t num_chars;
    uint32_t font_index;
    LunaVGFloat4 color;
    float font_size;
    float char_span;
    float line_span;
} LunaVGTextArrangeSection;

typedef struct LunaVGTextGlyphArrangeResult
{
    LunaVGRectF bounding_rect;
    float origin_offset;
    float advance_length;
    uint32_t character;
    uint32_t index;
} LunaVGTextGlyphArrangeResult;

typedef struct LunaVGTextLineArrangeResult
{
    LunaVGRectF bounding_rect;
    float baseline_offset;
    float ascent;
    float decent;
    float line_gap;
} LunaVGTextLineArrangeResult;

typedef void* luna_vg_text_arrange_result_t;

LUNA_VG_C_API luna_errcode_t luna_vg_init_module(void);

LUNA_VG_C_API luna_errcode_t luna_vg_arrange_text(
    const char* text,
    uint64_t text_len,
    const LunaVGTextArrangeSection* sections,
    uint64_t num_sections,
    const LunaVGRectF* bounding_rect,
    uint8_t vertical_alignment,
    uint8_t horizontal_alignment,
    luna_vg_text_arrange_result_t* out_result);
LUNA_VG_C_API void luna_vg_text_arrange_result_free(luna_vg_text_arrange_result_t result);
LUNA_VG_C_API luna_errcode_t luna_vg_text_arrange_result_get_bounding_rect(
    luna_vg_text_arrange_result_t result,
    LunaVGRectF* out_bounding_rect);
LUNA_VG_C_API int32_t luna_vg_text_arrange_result_get_overflow(luna_vg_text_arrange_result_t result);
LUNA_VG_C_API uint64_t luna_vg_text_arrange_result_get_num_lines(luna_vg_text_arrange_result_t result);
LUNA_VG_C_API luna_errcode_t luna_vg_text_arrange_result_get_line(
    luna_vg_text_arrange_result_t result,
    uint64_t index,
    LunaVGTextLineArrangeResult* out_line);
LUNA_VG_C_API uint64_t luna_vg_text_arrange_result_get_num_glyphs(
    luna_vg_text_arrange_result_t result,
    uint64_t line_index);
LUNA_VG_C_API luna_errcode_t luna_vg_text_arrange_result_get_glyph(
    luna_vg_text_arrange_result_t result,
    uint64_t line_index,
    uint64_t glyph_index,
    LunaVGTextGlyphArrangeResult* out_glyph);
LUNA_VG_C_API luna_errcode_t luna_vg_generate_text_arrange_result_draw_vertices(
    luna_vg_text_arrange_result_t result,
    const LunaVGTextArrangeSection* sections,
    uint64_t num_sections,
    void* font_atlas,
    LunaVGVertex* out_vertices,
    uint64_t vertex_capacity,
    uint64_t* out_vertex_count,
    uint32_t* out_indices,
    uint64_t index_capacity,
    uint64_t* out_index_count);
LUNA_VG_C_API luna_errcode_t luna_vg_commit_text_arrange_result(
    luna_vg_text_arrange_result_t result,
    const LunaVGTextArrangeSection* sections,
    uint64_t num_sections,
    void* font_atlas,
    void* draw_list);

LUNA_VG_C_API luna_errcode_t luna_vg_new_shape_buffer(LunaVGShapeBufferHandle* out_shape_buffer);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_buffer_get_points(
    void* self,
    float* out_points,
    uint64_t capacity,
    uint64_t* out_count);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_buffer_set_points(
    void* self,
    const float* points,
    uint64_t count);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_buffer_build(
    void* self,
    void* device,
    LunaRhiBufferHandle* out_buffer);

LUNA_VG_C_API luna_errcode_t luna_vg_new_font_atlas(LunaVGFontAtlasHandle* out_font_atlas);
LUNA_VG_C_API void luna_vg_font_atlas_clear(void* self);
LUNA_VG_C_API luna_errcode_t luna_vg_font_atlas_get_font(void* self, LunaFontHandle* out_font, uint32_t* out_index);
LUNA_VG_C_API luna_errcode_t luna_vg_font_atlas_set_font(void* self, void* font_file, uint32_t index);
LUNA_VG_C_API luna_errcode_t luna_vg_font_atlas_get_shape_buffer(void* self, LunaVGShapeBufferHandle* out_shape_buffer);
LUNA_VG_C_API luna_errcode_t luna_vg_font_atlas_get_glyph(
    void* self,
    uint32_t codepoint,
    uint64_t* out_first_shape_point,
    uint64_t* out_num_shape_points,
    LunaVGRectF* out_bounding_rect);
LUNA_VG_C_API luna_errcode_t luna_vg_get_font_glyph_shape(
    void* font_file,
    uint32_t font_index,
    uint32_t codepoint,
    float* out_shape_points,
    uint64_t capacity,
    uint64_t* out_count,
    LunaVGRectF* out_bounding_rect);

LUNA_VG_C_API luna_errcode_t luna_vg_new_shape_draw_list(
    void* device,
    LunaVGShapeDrawListHandle* out_draw_list);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_draw_list_get_device(void* self, LunaRhiDeviceHandle* out_device);
LUNA_VG_C_API void luna_vg_shape_draw_list_reset(void* self);
LUNA_VG_C_API void luna_vg_shape_draw_list_set_shape_buffer(void* self, void* shape_buffer);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_draw_list_get_shape_buffer(void* self, LunaVGShapeBufferHandle* out_shape_buffer);
LUNA_VG_C_API void luna_vg_shape_draw_list_set_texture(void* self, void* texture);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_draw_list_get_texture(void* self, LunaRhiTextureHandle* out_texture);
LUNA_VG_C_API void luna_vg_shape_draw_list_set_sampler(void* self, const LunaRhiSamplerDesc* desc);
LUNA_VG_C_API void luna_vg_shape_draw_list_reset_sampler(void* self);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_draw_list_get_sampler(void* self, LunaRhiSamplerDesc* out_sampler);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_draw_list_set_transform(void* self, const LunaVGMatrix4x4* transform);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_draw_list_get_transform(void* self, LunaVGMatrix4x4* out_transform);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_draw_list_set_clip_rect(void* self, const LunaVGRectF* clip_rect);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_draw_list_get_clip_rect(void* self, LunaVGRectF* out_clip_rect);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_draw_list_draw_shape_raw(
    void* self,
    const LunaVGVertex* vertices,
    uint64_t num_vertices,
    const uint32_t* indices,
    uint64_t num_indices);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_draw_list_draw_shape(
    void* self,
    uint32_t begin_command,
    uint32_t num_commands,
    const LunaVGFloat2* min_position,
    const LunaVGFloat2* max_position,
    const LunaVGFloat2* min_shapecoord,
    const LunaVGFloat2* max_shapecoord,
    const LunaVGFloat4* color,
    const LunaVGFloat2* min_texcoord,
    const LunaVGFloat2* max_texcoord);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_draw_list_compile(void* self);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_draw_list_get_vertex_buffer(void* self, LunaRhiBufferHandle* out_buffer);
LUNA_VG_C_API uint32_t luna_vg_shape_draw_list_get_vertex_buffer_size(void* self);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_draw_list_get_index_buffer(void* self, LunaRhiBufferHandle* out_buffer);
LUNA_VG_C_API uint32_t luna_vg_shape_draw_list_get_index_buffer_size(void* self);
LUNA_VG_C_API uint64_t luna_vg_shape_draw_list_get_draw_call_count(void* self);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_draw_list_get_draw_call(
    void* self,
    uint64_t index,
    LunaVGShapeDrawCall* out_draw_call);

LUNA_VG_C_API luna_errcode_t luna_vg_new_fill_shape_renderer(LunaVGShapeRendererHandle* out_renderer);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_renderer_begin(void* self, void* render_target);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_renderer_draw(
    void* self,
    void* vertex_buffer,
    void* index_buffer,
    const LunaVGShapeDrawCall* draw_calls,
    uint64_t num_draw_calls);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_renderer_draw_with_transform(
    void* self,
    void* vertex_buffer,
    void* index_buffer,
    const LunaVGShapeDrawCall* draw_calls,
    uint64_t num_draw_calls,
    const LunaVGMatrix4x4* transform_matrix);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_renderer_end(void* self);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_renderer_submit(void* self, void* command_buffer);

LUNA_VG_C_API luna_errcode_t luna_vg_shape_builder_add_rectangle_filled(void* shape_buffer, float min_x, float min_y, float max_x, float max_y);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_builder_add_rectangle_bordered(void* shape_buffer, float min_x, float min_y, float max_x, float max_y, float border_width, float border_offset);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_builder_add_rounded_rectangle_filled(void* shape_buffer, float min_x, float min_y, float max_x, float max_y, float radius);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_builder_add_rounded_rectangle_bordered(void* shape_buffer, float min_x, float min_y, float max_x, float max_y, float radius, float border_width, float border_offset);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_builder_add_triangle_filled(void* shape_buffer, float x1, float y1, float x2, float y2, float x3, float y3);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_builder_add_triangle_bordered(void* shape_buffer, float x1, float y1, float x2, float y2, float x3, float y3, float border_width, float border_offset);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_builder_add_circle_filled(void* shape_buffer, float center_x, float center_y, float radius);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_builder_add_circle_bordered(void* shape_buffer, float center_x, float center_y, float radius, float border_width, float border_offset);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_builder_add_axis_aligned_ellipse_filled(void* shape_buffer, float center_x, float center_y, float radius_x, float radius_y);
LUNA_VG_C_API luna_errcode_t luna_vg_shape_builder_add_axis_aligned_ellipse_bordered(void* shape_buffer, float center_x, float center_y, float radius_x, float radius_y, float border_width, float border_offset);

LUNA_VG_C_API luna_errcode_t luna_vg_get_rect_shape_draw_vertices(
    LunaVGVertex out_vertices[4],
    uint32_t out_indices[6],
    uint32_t begin_command,
    uint32_t num_commands,
    const LunaVGFloat2* min_position,
    const LunaVGFloat2* max_position,
    const LunaVGFloat2* min_shapecoord,
    const LunaVGFloat2* max_shapecoord,
    const LunaVGFloat4* color,
    const LunaVGFloat2* min_texcoord,
    const LunaVGFloat2* max_texcoord);

#ifdef __cplusplus
}
#endif
