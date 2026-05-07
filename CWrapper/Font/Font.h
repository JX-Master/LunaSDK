#pragma once

#include "../Runtime/Runtime.h"

#include <stdint.h>

#if defined(_WIN32)
#define LUNA_FONT_C_API __declspec(dllexport)
#else
#define LUNA_FONT_C_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    LUNA_FONT_INVALID_GLYPH = -1,
    LUNA_FONT_COMMAND_MOVE_TO = 1,
    LUNA_FONT_COMMAND_LINE_TO = 2,
    LUNA_FONT_COMMAND_CURVE_TO = 3
};

typedef struct LunaFontHandle
{
    luna_handle_t object;
    void* ifont_file;
} LunaFontHandle;

typedef struct LunaFontRectI
{
    int32_t offset_x;
    int32_t offset_y;
    int32_t width;
    int32_t height;
} LunaFontRectI;

typedef struct LunaFontVMetrics
{
    int32_t ascent;
    int32_t descent;
    int32_t line_gap;
} LunaFontVMetrics;

typedef struct LunaFontGlyphHMetrics
{
    int32_t advance_width;
    int32_t left_side_bearing;
} LunaFontGlyphHMetrics;

LUNA_FONT_C_API luna_errcode_t luna_font_init_module(void);
LUNA_FONT_C_API luna_errcode_t luna_font_load_ttf_font_file(const void* data, uint64_t data_size, LunaFontHandle* out_font_file);
LUNA_FONT_C_API luna_errcode_t luna_font_get_default_font(LunaFontHandle* out_font_file);
LUNA_FONT_C_API luna_errcode_t luna_font_ifont_file_get_data(void* self, void** out_data, uint64_t* out_size);
LUNA_FONT_C_API uint32_t luna_font_ifont_file_get_num_fonts(void* self);
LUNA_FONT_C_API int32_t luna_font_ifont_file_find_glyph(void* self, uint32_t font_index, uint32_t codepoint);
LUNA_FONT_C_API float luna_font_ifont_file_scale_for_pixel_height(void* self, uint32_t font_index, float pixels);
LUNA_FONT_C_API void luna_font_ifont_file_get_vmetrics(void* self, uint32_t font_index, LunaFontVMetrics* out_metrics);
LUNA_FONT_C_API void luna_font_ifont_file_get_glyph_hmetrics(void* self, uint32_t font_index, int32_t glyph, LunaFontGlyphHMetrics* out_metrics);
LUNA_FONT_C_API int32_t luna_font_ifont_file_get_kern_advance(void* self, uint32_t font_index, int32_t glyph1, int32_t glyph2);
LUNA_FONT_C_API luna_errcode_t luna_font_ifont_file_get_glyph_shape(void* self, uint32_t font_index, int32_t glyph, int16_t* out_commands, uint64_t capacity, uint64_t* out_count);
LUNA_FONT_C_API luna_errcode_t luna_font_ifont_file_get_glyph_bounding_box(void* self, uint32_t font_index, int32_t glyph, LunaFontRectI* out_rect);
LUNA_FONT_C_API luna_errcode_t luna_font_ifont_file_get_glyph_bitmap_box(void* self, uint32_t font_index, int32_t glyph, float scale_x, float scale_y, float shift_x, float shift_y, LunaFontRectI* out_rect);
LUNA_FONT_C_API luna_errcode_t luna_font_ifont_file_render_glyph_bitmap(void* self, uint32_t font_index, int32_t glyph, void* output, int32_t out_w, int32_t out_h, int32_t out_row_pitch, float scale_x, float scale_y, float shift_x, float shift_y);

#ifdef __cplusplus
}
#endif
